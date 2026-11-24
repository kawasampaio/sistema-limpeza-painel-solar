const graficoSinal = new Chart(document.getElementById("grafico-sinal"), {
  type: "line",
  data: {
    labels: [],
    datasets: [
      { label: "Sinal (%)", data: [], borderColor: "#f5a623", tension: 0.3 },
      { label: "Limite de limpeza", data: [], borderColor: "#d32f2f", borderDash: [6, 6], pointRadius: 0 }
    ]
  },
  options: { animation: false, scales: { y: { min: 80, max: 105 } } }
});

const TEXTOS = {
  normal: ["Normal", "normal"],
  sujeira: ["Sujeira detectada", "ativo"],
  limpando: ["LIMPANDO (bombas ligadas)", "ativo"],
  aguardando: ["Aguardando estabilizar", "normal"]
};

function mostrarEstado(estado) {
  const [texto, classe] = TEXTOS[estado] || ["--", ""];
  const el = document.getElementById("status");
  el.textContent = texto;
  el.className = classe;
}

function receberDado(d) {
  mostrarEstado(d.estado);

  if (d.porcentagem !== undefined) {
    const { labels, datasets } = graficoSinal.data;
    labels.push(new Date().toLocaleTimeString("pt-BR"));
    datasets[0].data.push(d.porcentagem);
    datasets[1].data.push(CONFIG.LIMITE);
    if (labels.length > CONFIG.MAX_PONTOS) {
      labels.shift();
      datasets[0].data.shift();
      datasets[1].data.shift();
    }
    graficoSinal.update();
  }

  // terminou uma limpeza: atualiza cards, tabela e gráfico do mês
  if (d.ativacao_salva) buscarAtivacoes().then(mostrarHistorico);
}

function iniciarTempoReal() {
  const ws = new WebSocket(CONFIG.WS_URL);
  ws.onopen = () => {
    document.getElementById("status").textContent = "Conectado, aguardando dados...";
  };
  ws.onmessage = (e) => receberDado(JSON.parse(e.data));
  ws.onerror = () => {
    document.getElementById("status").textContent = "Sem conexão com o servidor";
  };
  ws.onclose = () => setTimeout(iniciarTempoReal, 3000);
}