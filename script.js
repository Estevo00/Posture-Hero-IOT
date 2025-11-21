const API_URL = "https://691f83d631e684d7bfc9ef91.mockapi.io/posture";

async function fetchPostureData() {
  try {
    const res = await fetch(API_URL);
    const data = await res.json();

    if (!data || data.length === 0) return;

    const last = data[data.length - 1];

    // elementos
    const statusEl = document.getElementById("status");
    const distanceEl = document.getElementById("distance");
    const scoreEl = document.getElementById("score");
    const alertsEl = document.getElementById("alerts");
    const missionBar = document.getElementById("missionBar");
    const missionText = document.getElementById("missionText");
    const timestampEl = document.getElementById("timestamp");

    // status
    if (last.status === "good") {
      statusEl.textContent = "Boa Postura";
      statusEl.className = "status-good";
    } else {
      statusEl.textContent = "Má Postura";
      statusEl.className = "status-bad";
    }

    distanceEl.textContent = last.distance;
    scoreEl.textContent = last.score;
    alertsEl.textContent = last.alerts;

    timestampEl.textContent =
      "Última atualização: " + (last.createdAt || "agora");

    // missão do dia
    const goal = 60;
    const clamped = Math.min(last.score, goal);
    const percent = (clamped / goal) * 100;

    missionBar.style.width = percent + "%";
    missionText.textContent = `Progresso: ${clamped} / ${goal} pontos`;

  } catch (e) {
    console.log("Erro ao buscar dados:", e);
  }
}

fetchPostureData();
setInterval(fetchPostureData, 3000);

