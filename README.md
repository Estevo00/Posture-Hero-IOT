# Posture Hero - Sistema IoT de Monitoramento de Postura

Projeto desenvolvido como parte do Trabalho Interdisciplinar da FIAP.

## 🎯 Objetivo do Sistema
Monitorar a postura do usuário por meio de um sensor ultrassônico HC-SR04 acoplado a um ESP32.  
Os dados são enviados para uma API MockAPI onde ficam armazenados, e um dashboard web exibe as informações em tempo real.

## 🛠 Tecnologias Utilizadas
- ESP32 (Wokwi)
- Sensor Ultrassônico HC-SR04
- MockAPI (HTTP POST/GET)
- HTML/CSS/JS (Dashboard)
- GitHub

---

## 📡 Funcionamento do Circuito (ESP32)
- Mede continuamente a distância entre o usuário e o sensor.
- Classifica a postura como:
  - **Boa postura** → LED verde
  - **Má postura** → LED vermelho + buzzer
- A cada ciclo, envia dados para o endpoint HTTP:
  - status
  - distance
  - score
  - alerts

## 📍 Endpoint HTTP usado https://691f83d631e684d7bfc9ef91.mockapi.io/posture
---

## Arquivos do Repositório

## `esp32-http-server.ino`
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

// --------- Wi-Fi (Wokwi) ---------
const char* ssid        = "Wokwi-GUEST";  // padrão do Wokwi
const char* password    = "";             // sem senha

// --------- URL da API (MockAPI) ---------
const char* serverUrl   = "https://691f83d631e684d7bfc9ef91.mockapi.io/posture";

// --------- Pinos do circuito ---------
const int TRIG_PIN    = 5;   // HC-SR04 TRIG
const int ECHO_PIN    = 18;  // HC-SR04 ECHO
const int LED_GOOD    = 27;   // LED verde (boa postura)
const int LED_BAD     = 26;   // LED vermelho (má postura)
const int BUZZER_PIN  = 15;  // buzzer

// --------- Limiares de distância (em cm) ---------
const float DIST_GOOD_MIN = 25.0;
const float DIST_GOOD_MAX = 75.0;

// --------- Gamificação ---------
long lastCycleTime        = 0;
const long CYCLE_INTERVAL = 5000; // 5 s 

int postureScore   = 0;   // pontuação total
int alertCount     = 0;   // quantos alertas de má postura
int goodCycles     = 0;   // quantos ciclos em boa postura

String lastPostureStatus = "unknown";

// --------- Configuração de pinos ---------
void setupPins() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_GOOD, OUTPUT);
  pinMode(LED_BAD, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  digitalWrite(LED_GOOD, LOW);
  digitalWrite(LED_BAD, LOW);
  digitalWrite(BUZZER_PIN, LOW);
}

// --------- Leitura do sensor ultrassônico ---------
float readDistanceCM() {
  // garante TRIG em LOW
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  // pulso de 10 us em HIGH
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // mede duração do pulso no ECHO
  long duration = pulseIn(ECHO_PIN, HIGH);

  // distância em cm (velocidade do som ~0.034cm/us, divide por 2 ida/volta)
  float distance = duration * 0.034 / 2.0;
  return distance;
}

// --------- Conexão Wi-Fi ---------
void connectWiFi() {
  Serial.print("Conectando ao WiFi: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi conectado!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

// --------- Envio HTTP para o MockAPI (HTTPS) ---------
void sendHttpToMockAPI(String postureStatus, float distance) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi desconectado, não foi possível enviar HTTP.");
    return;
  }

  WiFiClientSecure client;
  client.setInsecure();  

  HTTPClient http;
  http.begin(client, serverUrl);  // usa o client seguro + URL https
  http.addHeader("Content-Type", "application/json");

  // Corpo JSON que será salvo no MockAPI
  String jsonPayload = "{";
  jsonPayload += "\"status\":\"" + postureStatus + "\",";
  jsonPayload += "\"distance\":" + String(distance, 2) + ",";
  jsonPayload += "\"score\":" + String(postureScore) + ",";
  jsonPayload += "\"alerts\":" + String(alertCount) + ",";
  // >>> AQUI entra o createdAt que você pediu <<<
  jsonPayload += "\"createdAt\":\"" + String(millis()) + "\"";
  jsonPayload += "}";

  Serial.print("Enviando HTTP POST: ");
  Serial.println(jsonPayload);

  int httpResponseCode = http.POST(jsonPayload);

  Serial.print("HTTP Response code: ");
  Serial.println(httpResponseCode);

  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.print("Resposta do servidor: ");
    Serial.println(response);
  } else {
    Serial.print("Falha na requisição. Erro: ");
    Serial.println(httpResponseCode);
  }

  http.end();
}

// --------- SETUP ---------
void setup() {
  Serial.begin(115200);
  delay(1000);

  setupPins();
  connectWiFi();

  lastCycleTime = millis();
}

// --------- LOOP PRINCIPAL ---------
void loop() {
  // 1) Lê distância
  float distance = readDistanceCM();
  Serial.print("Distância: ");
  Serial.print(distance);
  Serial.println(" cm");

  // 2) Decide se postura está boa ou ruim
  String postureStatus;

  if (distance >= DIST_GOOD_MIN && distance <= DIST_GOOD_MAX) {
    // boa postura
    postureStatus = "good";
    digitalWrite(LED_GOOD, HIGH);
    digitalWrite(LED_BAD, LOW);
    digitalWrite(BUZZER_PIN, LOW);
  } else {
    // má postura
    postureStatus = "bad";
    digitalWrite(LED_GOOD, LOW);
    digitalWrite(LED_BAD, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);
  }

  // 3) Conta alerta quando muda de boa -> ruim
  if (postureStatus == "bad" && lastPostureStatus == "good") {
    alertCount++;
    Serial.println("ALERTA: usuário curvou as costas!");
  }

  // 4) A cada ciclo de tempo, atualiza pontuação e envia dados
  long now = millis();
  if (now - lastCycleTime >= CYCLE_INTERVAL) {
    lastCycleTime = now;

    if (postureStatus == "good") {
      postureScore += 10;  // +10 pontos por ciclo bom
      goodCycles++;
    }

    Serial.println("----- RESUMO CICLO -----");
    Serial.print("Score: ");   Serial.println(postureScore);
    Serial.print("Alertas: "); Serial.println(alertCount);
    Serial.print("Ciclos bons: "); Serial.println(goodCycles);
    Serial.println("------------------------");

    // Envia para MockAPI
    sendHttpToMockAPI(postureStatus, distance);
  }

  lastPostureStatus = postureStatus;

  delay(200); // pequeno delay para estabilizar
}

---------------------------------------
### `index.html`

<!DOCTYPE html>
<html lang="pt-BR">
<head>
  <meta charset="UTF-8" />
  <title>Posture Hero - Dashboard</title>
  <link rel="stylesheet" href="style.css" />
</head>

<body>
  <h1>Posture Hero - Dashboard</h1>
  <p>Monitoramento de postura e gamificação no futuro do trabalho.</p>

  <div class="card">
    <p class="label">Status da Postura:</p>
    <p id="status">Carregando...</p>

    <p class="label">Distância da coluna (cm):</p>
    <p id="distance">--</p>

    <p class="label">Pontuação (Score):</p>
    <p id="score">0</p>

    <p class="label">Alertas de má postura:</p>
    <p id="alerts">0</p>

    <p class="label">Missão do dia:</p>
    <p>Acumular 60 pontos de postura correta.</p>
    <div class="mission-bar-container">
      <div id="missionBar" class="mission-bar"></div>
    </div>
    <p class="small" id="missionText">Progresso: 0 / 60 pontos</p>

    <p class="small" id="timestamp">Última atualização: --</p>
  </div>

  <script src="script.js"></script>
</body>
</html>
---------------------------------------
### `style.css`
body {
  font-family: Arial, sans-serif;
  background: #0f172a;
  color: #e5e7eb;
  display: flex;
  flex-direction: column;
  align-items: center;
  padding: 20px;
}

.card {
  background: #111827;
  padding: 20px;
  border-radius: 12px;
  max-width: 420px;
  width: 100%;
  box-shadow: 0 10px 20px rgba(0,0,0,0.4);
  margin-top: 20px;
}

h1 {
  margin-bottom: 10px;
  text-align: center;
}

.status-good {
  color: #22c55e;
  font-weight: bold;
}

.status-bad {
  color: #ef4444;
  font-weight: bold;
}

.label {
  font-weight: bold;
  margin-top: 8px;
}

.mission-bar-container {
  margin-top: 15px;
  background: #1f2937;
  border-radius: 999px;
  overflow: hidden;
  height: 16px;
}

.mission-bar {
  height: 100%;
  width: 0%;
  background: linear-gradient(90deg, #22c55e, #a3e635);
  transition: width 0.3s;
}

.small {
  font-size: 12px;
  color: #9ca3af;
  margin-top: 6px;
}

------------------------------------------------
### `script.js`

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

---

## ▶ Simulação Wokwi
Link para simulação completa no Wokwi:  
https://wokwi.com/projects/448101008671082497

---

## 📽 Vídeo Demonstrativo
Demonstração do projeto funcionando:  
https://youtu.be/mopySJtn7VE

---

## 👥 Autores
- Felipe Estevo
- Cauã Silva
