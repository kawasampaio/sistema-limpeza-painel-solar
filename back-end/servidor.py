import asyncio
import sqlite3
import threading
import time
from datetime import datetime
from pathlib import Path

import serial
from fastapi import FastAPI, WebSocket, WebSocketDisconnect
from fastapi.middleware.cors import CORSMiddleware
from fastapi.staticfiles import StaticFiles

PORTA = "COM3"      # porta USB do Arduino (NAO é a 8000)
BAUD = 115200
DB = "ativacoes.db"
PASTA_FRONT = Path(__file__).resolve().parent.parent / "front-end"

app = FastAPI()
app.add_middleware(CORSMiddleware, allow_origins=["*"], allow_methods=["*"], allow_headers=["*"])

clientes: set[WebSocket] = set()
diagnostico = {"linhas_recebidas": 0, "ultima_linha": "", "clientes_conectados": 0}


def iniciar_banco():
    with sqlite3.connect(DB) as c:
        c.execute("""CREATE TABLE IF NOT EXISTS ativacoes (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            inicio TEXT, fim TEXT, duracao_seg INTEGER, queda_max REAL)""")


def salvar_ativacao(inicio, fim, queda_max):
    duracao = round((fim - inicio).total_seconds())
    with sqlite3.connect(DB) as c:
        c.execute("INSERT INTO ativacoes (inicio, fim, duracao_seg, queda_max) VALUES (?,?,?,?)",
                  (inicio.isoformat(), fim.isoformat(), duracao, queda_max))


async def transmitir(dados: dict):
    for ws in list(clientes):
        try:
            await ws.send_json(dados)
        except Exception:
            clientes.discard(ws)


def enviar(loop, dados):
    asyncio.run_coroutine_threadsafe(transmitir(dados), loop)


def ler_serial(loop):
    inicio_limpeza = None
    queda_max = 0.0
    ultima_queda = 0.0

    while True:
        try:
            with serial.Serial(PORTA, BAUD, timeout=1) as s:
                print(f"Conectado ao Arduino em {PORTA}")
                while True:
                    linha = s.readline().decode(errors="ignore").strip()
                    if not linha:
                        continue

                    print("ARDUINO:", linha)
                    diagnostico["linhas_recebidas"] += 1
                    diagnostico["ultima_linha"] = linha

                    if "INICIANDO LIMPEZA" in linha:
                        inicio_limpeza = datetime.now()
                        queda_max = ultima_queda
                        enviar(loop, {"estado": "limpando"})
                        continue

                    if "LIMPEZA FINALIZADA" in linha and inicio_limpeza:
                        salvar_ativacao(inicio_limpeza, datetime.now(), queda_max)
                        inicio_limpeza = None
                        enviar(loop, {"estado": "aguardando", "ativacao_salva": True})
                        continue

                    partes = [p.strip() for p in linha.split("|")]
                    if len(partes) == 4 and partes[1].endswith("%"):
                        try:
                            bruto = float(partes[0])
                            porcentagem = float(partes[1].rstrip("%"))
                            queda = float(partes[2].rstrip("%"))
                        except ValueError:
                            continue
                        ultima_queda = queda
                        queda_max = max(queda_max, queda)
                        estado = "sujeira" if partes[3].startswith("SUJEIRA") else "normal"
                        enviar(loop, {"bruto": bruto, "porcentagem": porcentagem,
                                      "queda": queda, "estado": estado})
        except serial.SerialException as e:
            print(f"Erro na serial: {e}. Tentando de novo em 3s...")
            time.sleep(3)


@app.on_event("startup")
async def startup():
    iniciar_banco()
    loop = asyncio.get_running_loop()
    threading.Thread(target=ler_serial, args=(loop,), daemon=True).start()


@app.websocket("/ws/sensor/")
async def ws_sensor(ws: WebSocket):
    await ws.accept()
    clientes.add(ws)
    print("Navegador conectado no WebSocket")
    try:
        while True:
            await ws.receive_text()
    except WebSocketDisconnect:
        clientes.discard(ws)


@app.get("/api/ativacoes/")
def listar_ativacoes(mes: str | None = None):
    mes = mes or datetime.now().strftime("%Y-%m")
    with sqlite3.connect(DB) as c:
        linhas = c.execute(
            "SELECT inicio, fim, duracao_seg, queda_max FROM ativacoes WHERE inicio LIKE ? ORDER BY inicio DESC",
            (f"{mes}%",)).fetchall()
    return [{"inicio": i, "fim": f, "duracao_seg": d, "queda_max": q} for i, f, d, q in linhas]


@app.get("/api/status/")
def status():
    diagnostico["clientes_conectados"] = len(clientes)
    return diagnostico


# SEMPRE por último: entrega o front-end em http://localhost:8010/
app.mount("/", StaticFiles(directory=PASTA_FRONT, html=True), name="front")