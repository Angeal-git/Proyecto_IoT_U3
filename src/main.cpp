#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiClientSecure.h>
#include <DHT.h>

// ===== CONFIGURACIÓN DE RED =====
const char* ssid = "TU_SSID";
const char* password = "TU_PASSWORD";

// ===== CONFIGURACIÓN GEMINI =====
String GEMINI_API_KEY = "TU_API_KEY_DE_GEMINI";

// ===== SENSOR DHT11 =====
#define DHTPIN D4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// ===== SERVIDOR WEB =====
ESP8266WebServer server(80);

// ===== FUNCIÓN: Página HTML =====
String paginaHTML() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
  <meta charset="UTF-8">
  <title>🌿 Chat Ambiental con IA</title>
  <style>
    body {
      font-family: "Poppins", sans-serif;
      background: linear-gradient(135deg, #f2f2f2, #d9e4f5);
      color: #333;
      text-align: center;
      padding: 50px;
      transition: all 0.8s ease;
    }
    h1 { font-size: 2em; margin-bottom: 10px; }
    .content {
      max-width: 600px;
      margin: auto;
      background: white;
      padding: 20px;
      border-radius: 20px;
      box-shadow: 0 5px 15px rgba(0,0,0,0.1);
    }
    .data {
      font-size: 1.2em;
      color: #007bff;
      margin-bottom: 20px;
    }
    input {
      width: 70%;
      padding: 10px;
      border-radius: 10px;
      border: 1px solid #ccc;
    }
    button {
      padding: 10px 15px;
      margin-left: 10px;
      border: none;
      border-radius: 10px;
      background: #28a745;
      color: white;
      cursor: pointer;
      transition: 0.3s;
    }
    button:hover { background: #1f7e36; }
    #chatBox {
      text-align: left;
      margin-top: 25px;
      background: #f4f4f4;
      padding: 15px;
      border-radius: 10px;
      font-family: monospace;
      height: 300px;
      overflow-y: auto;
      white-space: pre-wrap;
    }
  </style>
</head>
<body>
  <h1>🌿 Chat Ambiental con IA</h1>
  <div class="content">
    <div class="data">
      🌡️ Temperatura: <span id="temp">--</span> °C<br>
      💧 Humedad: <span id="hum">--</span> %
    </div>
    <input type="text" id="input" placeholder="Hazle una pregunta a la IA...">
    <button onclick="enviar()">Enviar</button>
    <div id="chatBox"></div>
  </div>

  <script>
    async function actualizarDatos() {
      const r = await fetch('/data');
      const d = await r.json();
      document.getElementById("temp").textContent = d.temp;
      document.getElementById("hum").textContent = d.hum;
    }
    async function enviar() {
      const input = document.getElementById("input");
      const chat = document.getElementById("chatBox");
      const pregunta = input.value.trim();
      if (!pregunta) return;
      chat.innerHTML += `\n🧍‍♂️ Tú: ${pregunta}`;
      input.value = "";
      const r = await fetch("/ask?question=" + encodeURIComponent(pregunta));
      const txt = await r.text();
      chat.innerHTML += `\n🤖 IA: ${txt}`;
      chat.scrollTop = chat.scrollHeight;
    }
    setInterval(actualizarDatos, 5000);
    actualizarDatos();
  </script>
</body>
</html>
)rawliteral";
  return html;
}

// ===== RUTAS =====
void handleRoot() {
  server.send(200, "text/html", paginaHTML());
}

void handleData() {
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  if (isnan(temp) || isnan(hum)) {
    server.send(500, "application/json", "{\"error\":\"Error en DHT11\"}");
    return;
  }
  String json = "{\"temp\":" + String(temp, 1) + ",\"hum\":" + String(hum, 1) + "}";
  server.send(200, "application/json", json);
}

void handleAsk() {
  if (!server.hasArg("question")) {
    server.send(400, "text/plain", "Falta el parámetro 'question'");
    return;
  }
  String pregunta = server.arg("question");
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  String prompt = "Actúa como un asistente ambiental. "
                  "Con base en estos datos: Temperatura " + String(temp) + " °C, "
                  "Humedad " + String(hum) + "%. "
                  "Responde de forma amigable a la pregunta: " + pregunta;

  WiFiClientSecure client;
  client.setInsecure();
  if (!client.connect("generativelanguage.googleapis.com", 443)) {
    server.send(500, "text/plain", "No se pudo conectar a Gemini");
    return;
  }

  String url = "/v1beta/models/gemini-1.5-flash:generateContent?key=" + GEMINI_API_KEY;
  String body = "{\"contents\":[{\"parts\":[{\"text\":\"" + prompt + "\"}]}]}";

  String request = "POST " + url + " HTTP/1.1\r\n" +
                   "Host: generativelanguage.googleapis.com\r\n" +
                   "Content-Type: application/json\r\n" +
                   "Content-Length: " + String(body.length()) + "\r\n\r\n" +
                   body;

  client.print(request);

  String response;
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") break;
  }
  while (client.available()) {
    response += client.readString();
  }

  int i = response.indexOf("\"text\":");
  String output = "No se obtuvo respuesta";
  if (i != -1) {
    output = response.substring(i + 8);
    int end = output.indexOf("\"");
    if (end != -1) output = output.substring(0, end);
    output.replace("\\n", " ");
  }

  server.send(200, "text/plain", output);
}

// ===== SETUP =====
void setup() {
  Serial.begin(115200);
  dht.begin();
  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi conectado");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.on("/ask", handleAsk);
  server.begin();
  Serial.println("Servidor iniciado 🚀");
}

// ===== LOOP =====
void loop() {
  server.handleClient();
}