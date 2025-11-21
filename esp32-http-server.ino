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
