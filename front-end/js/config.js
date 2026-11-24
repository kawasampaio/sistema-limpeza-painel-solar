const BASE = location.protocol === "file:" ? "localhost:8010" : location.host;

const CONFIG = {
  USAR_SIMULACAO: false,
  API_URL: `http://${BASE}/api`,
  WS_URL: `ws://${BASE}/ws/sensor/`,
  LIMITE: 95,
  MAX_PONTOS: 60
};