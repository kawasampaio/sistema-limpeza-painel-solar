async function buscarAtivacoes() {
  if (CONFIG.USAR_SIMULACAO) {
    return Array.from({ length: 12 }, (_, i) => {
      const inicio = new Date(2026, 9, i * 2 + 1, 10, 0, 0);
      const duracao = 60 + Math.floor(Math.random() * 240);
      return { inicio, fim: new Date(inicio.getTime() + duracao * 1000), duracao_seg: duracao };
    });
  }
  const r = await fetch(`${CONFIG.API_URL}/ativacoes/`);
  const dados = await r.json();
  return dados.map(a => ({ ...a, inicio: new Date(a.inicio), fim: new Date(a.fim) }));
}