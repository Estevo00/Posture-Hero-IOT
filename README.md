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

## ▶ Simulação Wokwi
Link para simulação completa no Wokwi:  
https://wokwi.com/projects/448101008671082497

---

## 📽 Vídeo Demonstrativo
Demonstração do projeto funcionando:  
https://youtu.be/mopySJtn7VE

---

## 👥 Autores
- Felipe Estevo RM567780
- Cauã Silva RM568143
