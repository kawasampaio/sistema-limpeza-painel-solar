let graficoMensal = null;

function formatarDuracao(seg) {
  const m = Math.floor(seg / 60), s = seg % 60;
  return m ? `${m} min ${s} s` : `${s} s`;
}

// Escreve o número de ativações em cima de cada coluna
const rotulosNasColunas = {
  id: "rotulosNasColunas",
  afterDatasetsDraw(chart) {
    const { ctx } = chart;
    const meta = chart.getDatasetMeta(0);
    ctx.save();
    ctx.fillStyle = "#333";
    ctx.font = "bold 13px Arial";
    ctx.textAlign = "center";
    meta.data.forEach((barra, i) => {
      const valor = chart.data.datasets[0].data[i];
      if (valor > 0) ctx.fillText(valor, barra.x, barra.y - 6);
    });
    ctx.restore();
  }
};

function mostrarHistorico(ativacoes) {
  // ----- cards -----
  const total = ativacoes.reduce((soma, a) => soma + a.duracao_seg, 0);
  document.getElementById("total-ativacoes").textContent = ativacoes.length;
  document.getElementById("tempo-total").textContent = formatarDuracao(total);
  document.getElementById("tempo-medio").textContent =
    formatarDuracao(ativacoes.length ? Math.round(total / ativacoes.length) : 0);

  // ----- tabela -----
  document.querySelector("#tabela tbody").innerHTML = ativacoes.map(a => `
    <tr>
      <td>${new Date(a.inicio).toLocaleString("pt-BR")}</td>
      <td>${new Date(a.fim).toLocaleString("pt-BR")}</td>
      <td>${formatarDuracao(a.duracao_seg)}</td>
    </tr>`).join("");

  // ----- gráfico: uma coluna por dia do mês -----
  const hoje = new Date();
  const diasNoMes = new Date(hoje.getFullYear(), hoje.getMonth() + 1, 0).getDate();
  const contagem = Array(diasNoMes).fill(0);

  ativacoes.forEach(a => {
    const d = new Date(a.inicio);
    if (!isNaN(d)) contagem[d.getDate() - 1]++;
  });

  const dias = Array.from({ length: diasNoMes }, (_, i) => i + 1);
  const nomeMes = hoje.toLocaleDateString("pt-BR", { month: "long", year: "numeric" });

  if (graficoMensal) {
    graficoMensal.data.labels = dias;
    graficoMensal.data.datasets[0].data = contagem;
    graficoMensal.options.plugins.title.text = `Ativações da limpeza por dia: ${nomeMes}`;
    graficoMensal.update();
    return;
  }

  graficoMensal = new Chart(document.getElementById("grafico-mensal"), {
    type: "bar",
    data: {
      labels: dias,
      datasets: [{
        label: "Ativações",
        data: contagem,
        backgroundColor: "#ee5348",
        borderWidth: 0,
        barPercentage: 0.45,
        categoryPercentage: 1.0
      }]
    },
    options: {
      maintainAspectRatio: false,
      layout: { padding: { top: 20, right: 10 } },
      plugins: {
        legend: { display: false },
        title: {
          display: true,
          text: `Ativações da limpeza por dia: ${nomeMes}`,
          align: "start",
          color: "#333",
          font: { size: 18, weight: "normal" },
          padding: { bottom: 16 }
        },
        tooltip: {
          callbacks: {
            title: (itens) => `Dia ${itens[0].label}`,
            label: (item) => `${item.raw} ${item.raw === 1 ? "ativação" : "ativações"}`
          }
        }
      },
      scales: {
        x: {
          title: { display: true, text: "Dia do mês", color: "#555" },
          grid: { display: false },
          border: { color: "#bbb" }
        },
        y: {
          beginAtZero: true,
          title: { display: true, text: "(número de ativações)", color: "#555" },
          ticks: { stepSize: 1, precision: 0 },
          grid: { color: "#d9d9d9" },
          border: { color: "#bbb" }
        }
      }
    },
    plugins: [rotulosNasColunas]
  });
}