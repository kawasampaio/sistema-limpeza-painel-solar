# Sensor infravermelho de sujidade para painel solar

Projeto para Arduino Uno usando VS Code + PlatformIO.

## Critério de acionamento adotado

A versão inicial usava uma queda de 25% do sinal. Esse valor era arbitrário.

Nesta versão, o ponto inicial foi alterado para **5% de queda do sinal óptico**:

- referência com painel limpo = 100%
- acionamento = 95% ou menos
- confirmação = 8 leituras seguidas abaixo do limite
- limpeza = bombas ligadas por 5 s, espera de 10 s e nova medição

O objetivo é detectar a sujeira antes que a perda de potência se torne grande.

### Fundamentação

**Alkharusi, T.; Alzahrani, M. M.; Pandey, C.; Yildizhan, H.; Markides, C. N. (2024).  
Experimental investigation of nonuniform PV soiling. Solar Energy, 272, 112493.  
DOI: 10.1016/j.solener.2024.112493**

O estudo experimental relata aproximadamente **6–7% de deterioração da potência para cada 5% de queda na transmitância**, nas condições estudadas.

Outro trabalho útil:

**Majeed, R.; Waqas, A.; Sami, H.; Ali, M.; Shahzad, N. (2020).  
Experimental investigation of soiling losses and a novel cost-effective cleaning system for PV modules. Solar Energy, 201, 298–306.  
DOI: 10.1016/j.solener.2020.03.014**

Nesse experimento, uma densidade de poeira de 4,6 g/m² esteve associada a queda de potência de aproximadamente **16,16% em módulo monocristalino** e **11,54% em policristalino** após um mês.

## Atenção científica importante

O artigo de Alkharusi et al. mede **transmitância óptica** do vidro/módulo.

O sensor conectado ao A0 deste projeto mede uma **resposta infravermelha/reflexiva**.

Por isso, não é correto afirmar automaticamente que:

`5% de queda no infravermelho = exatamente 5% de perda de potência`.

O limite de 95% deve ser tratado como um **ponto inicial de projeto baseado em literatura**.

Para transformar o sensor em um instrumento calibrado, o ideal é fazer um ensaio:

1. medir o A0 com o painel limpo;
2. medir tensão/corrente/potência do painel nas mesmas condições;
3. adicionar níveis controlados de poeira;
4. repetir as duas medições;
5. montar uma tabela relacionando queda do sensor e queda real de potência;
6. escolher o valor do A0 correspondente à perda máxima aceitável.

## Ligações

Sensor:

- VCC -> 5V
- GND -> GND
- AO -> A0

Saída:

- D7 = relé K1 (bomba 1)
- D8 = relé K2 (bomba 2)
- módulo relé de 2 canais, acionado em nível LOW

**Não conecte uma bomba diretamente aos pinos D7/D8.**
Use relé ou MOSFET adequado e fonte apropriada para a bomba.

## Parâmetros que podem ser alterados

No `src/main.cpp`:

```cpp
const float TRIGGER_PERCENT = 95.0;
const uint8_t CONFIRMATION_READS = 8;
const unsigned long TEMPO_BOMBAS = 5000;
```

Exemplos:

- `95.0` = aciona com queda de 5%
- `90.0` = aciona com queda de 10%
- `85.0` = aciona com queda de 15%
- `75.0` = aciona com queda de 25%

## Sentido do sensor

```cpp
const bool SENSOR_INVERTED = false;
```

Se mais sujeira fizer a leitura de A0 diminuir, mantenha `false`.

Se mais sujeira fizer a leitura de A0 aumentar, use:

```cpp
const bool SENSOR_INVERTED = true;
```

## PlatformIO

`platformio.ini`:

```ini
[env:uno]
platform = atmelavr
board = uno
framework = arduino
monitor_speed = 115200
```

Comandos no VS Code:

- Build: Ctrl+Alt+B
- Upload: Ctrl+Alt+U
- Serial Monitor: Ctrl+Alt+S
- python -m uvicorn servidor:app --port 8010

## Rodar em outra máquina (Windows)

Pré-requisito: Python 3.10 ou mais novo. Se a máquina não tiver, instale e depois feche e abra o PowerShell de novo:

```powershell
winget install -e --id Python.Python.3.13
```

1. Instalar todas as bibliotecas (no PowerShell, dentro da pasta do projeto):

```powershell
python -m pip install -r requirements.txt
```

2. Ligar o Arduino no USB e ver em qual porta COM ele está:

```powershell
python -m serial.tools.list_ports
```

Se não for `COM3`, troque a linha `PORTA = "COM3"` em `back-end/servidor.py`.

3. Fechar o Monitor Serial do VS Code / Arduino IDE (só um programa pode usar a porta por vez).

4. Deixar o sensor apontado para o painel LIMPO (o Arduino calibra ao conectar) e iniciar o servidor:

```powershell
cd back-end
python -m uvicorn servidor:app --port 8010
```

5. Abrir no navegador: http://localhost:8010/

Opcional, só se precisar gravar o código no Arduino a partir dessa máquina:

```powershell
python -m pip install platformio
python -m platformio run -t upload
```
