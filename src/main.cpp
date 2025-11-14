#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiClientSecure.h>
#include <DHT.h>

// =======================================================
// CONFIGURACIÓN WiFi
// =======================================================
const char* ssid = "Tu_SSID_Aqui";
const char* password = "Tu_Contraseña_Aqui";

// =======================================================
// CONFIGURACIÓN DHT11
// =======================================================
#define DHTPIN D2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// =======================================================
// CONFIGURACIÓN API GEMINI
// =======================================================
String GEMINI_API_KEY = "Tu_API_Key_Aqui";

// =======================================================
// SERVIDOR WEB
// =======================================================
ESP8266WebServer server(80);

// =======================================================
// PÁGINA HTML - TEMA OSCURO MEJORADO
// =======================================================
String paginaHTML() {
  return R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>🌿 Chat Ambiental con IA</title>
<style>
* {
  margin: 0;
  padding: 0;
  box-sizing: border-box;
}

body {
  font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
  background: linear-gradient(135deg, #1a1a2e 0%, #16213e 100%);
  color: #e0e0e0;
  min-height: 100vh;
  padding: 20px;
}

.container {
  max-width: 800px;
  margin: 0 auto;
}

h1 {
  text-align: center;
  color: #4ecca3;
  font-size: 2em;
  margin-bottom: 30px;
  text-shadow: 0 2px 10px rgba(78, 204, 163, 0.3);
}

.sensor-card {
  background: linear-gradient(145deg, #2a2a3e, #1f1f30);
  border-radius: 15px;
  padding: 25px;
  margin-bottom: 20px;
  box-shadow: 0 8px 32px rgba(0, 0, 0, 0.4);
  border: 1px solid rgba(78, 204, 163, 0.2);
}

.sensor-card h2 {
  color: #4ecca3;
  margin-bottom: 20px;
  font-size: 1.3em;
  text-align: center;
}

.sensor-data {
  display: flex;
  justify-content: space-around;
  gap: 20px;
  flex-wrap: wrap;
}

.sensor-item {
  background: rgba(78, 204, 163, 0.1);
  padding: 15px 25px;
  border-radius: 10px;
  text-align: center;
  flex: 1;
  min-width: 120px;
  border: 1px solid rgba(78, 204, 163, 0.3);
}

.sensor-item .icon {
  font-size: 1.8em;
  margin-bottom: 8px;
}

.sensor-item .value {
  font-size: 1.5em;
  font-weight: bold;
  color: #4ecca3;
  margin: 5px 0;
}

.sensor-item .label {
  font-size: 0.9em;
  color: #b0b0b0;
}

.chat-container {
  background: linear-gradient(145deg, #2a2a3e, #1f1f30);
  border-radius: 15px;
  padding: 20px;
  box-shadow: 0 8px 32px rgba(0, 0, 0, 0.4);
  border: 1px solid rgba(78, 204, 163, 0.2);
}

#chat {
  background: #16213e;
  height: 400px;
  overflow-y: auto;
  padding: 15px;
  border-radius: 10px;
  margin-bottom: 15px;
  border: 1px solid rgba(78, 204, 163, 0.2);
}

#chat::-webkit-scrollbar {
  width: 8px;
}

#chat::-webkit-scrollbar-track {
  background: #1a1a2e;
  border-radius: 10px;
}

#chat::-webkit-scrollbar-thumb {
  background: #4ecca3;
  border-radius: 10px;
}

.message {
  margin-bottom: 15px;
  padding: 12px 15px;
  border-radius: 10px;
  line-height: 1.6;
  animation: fadeIn 0.3s ease-in;
}

@keyframes fadeIn {
  from { opacity: 0; transform: translateY(10px); }
  to { opacity: 1; transform: translateY(0); }
}

.user-message {
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  color: white;
  margin-left: 20%;
  border-bottom-right-radius: 3px;
}

.ai-message {
  background: linear-gradient(135deg, #4ecca3 0%, #3ba884 100%);
  color: #1a1a2e;
  margin-right: 20%;
  border-bottom-left-radius: 3px;
  white-space: pre-line;
}

.message-label {
  font-weight: bold;
  font-size: 0.85em;
  margin-bottom: 5px;
  opacity: 0.9;
}

.input-container {
  display: flex;
  gap: 10px;
}

#msg {
  flex: 1;
  padding: 12px 15px;
  border: 2px solid rgba(78, 204, 163, 0.3);
  border-radius: 10px;
  background: #16213e;
  color: #e0e0e0;
  font-size: 1em;
  transition: all 0.3s ease;
}

#msg:focus {
  outline: none;
  border-color: #4ecca3;
  box-shadow: 0 0 15px rgba(78, 204, 163, 0.3);
}

button {
  padding: 12px 30px;
  border: none;
  background: linear-gradient(135deg, #4ecca3 0%, #3ba884 100%);
  color: #1a1a2e;
  font-weight: bold;
  border-radius: 10px;
  cursor: pointer;
  transition: all 0.3s ease;
  font-size: 1em;
}

button:hover {
  transform: translateY(-2px);
  box-shadow: 0 5px 20px rgba(78, 204, 163, 0.4);
}

button:active {
  transform: translateY(0);
}

.loading {
  display: none;
  text-align: center;
  color: #4ecca3;
  margin: 10px 0;
  font-style: italic;
}

@media (max-width: 600px) {
  h1 { font-size: 1.5em; }
  .sensor-data { flex-direction: column; }
  .user-message { margin-left: 10%; }
  .ai-message { margin-right: 10%; }
}
</style>
</head>
<body>

<div class="container">
  <h1>🌿 Chat Ambiental con IA</h1>

  <div class="sensor-card">
    <h2>📊 Sensor DHT11</h2>
    <div class="sensor-data">
      <div class="sensor-item">
        <div class="icon">🌡️</div>
        <div class="value" id="temp">--</div>
        <div class="label">Temperatura °C</div>
      </div>
      <div class="sensor-item">
        <div class="icon">💧</div>
        <div class="value" id="hum">--</div>
        <div class="label">Humedad %</div>
      </div>
    </div>
  </div>

  <div class="chat-container">
    <div id="chat"></div>
    <div class="loading" id="loading">⏳ La IA está pensando...</div>
    <div class="input-container">
      <input id="msg" placeholder="Escribe tu pregunta sobre el clima..." 
             onkeypress="if(event.key==='Enter') send()">
      <button onclick="send()">Enviar</button>
    </div>
  </div>
</div>

<script>
async function updateSensor() {
  try {
    const r = await fetch('/data');
    const d = await r.json();
    document.getElementById("temp").innerText = d.temp;
    document.getElementById("hum").innerText = d.hum;
  } catch(e) {
    console.error('Error actualizando sensor:', e);
  }
}

async function send() {
  let q = document.getElementById("msg").value.trim();
  if (!q) return;

  const chat = document.getElementById("chat");
  const loading = document.getElementById("loading");
  
  // Mensaje del usuario
  const userMsg = document.createElement("div");
  userMsg.className = "message user-message";
  userMsg.innerHTML = '<div class="message-label">👤 Tú:</div>' + q;
  chat.appendChild(userMsg);
  
  document.getElementById("msg").value = "";
  chat.scrollTop = chat.scrollHeight;

  // Mostrar indicador de carga
  loading.style.display = "block";

  try {
    const r = await fetch("/ask?question=" + encodeURIComponent(q));
    const t = await r.text();

    // Mensaje de la IA
    const aiMsg = document.createElement("div");
    aiMsg.className = "message ai-message";
    aiMsg.innerHTML = '<div class="message-label">🤖 Gemini:</div>' + t;
    chat.appendChild(aiMsg);
  } catch(e) {
    const errorMsg = document.createElement("div");
    errorMsg.className = "message ai-message";
    errorMsg.innerHTML = '<div class="message-label">⚠️ Error:</div>No se pudo conectar con la IA';
    chat.appendChild(errorMsg);
  }

  loading.style.display = "none";
  chat.scrollTop = chat.scrollHeight;
}

// Actualizar sensor cada 3 segundos
setInterval(updateSensor, 3000);
updateSensor();

// Mensaje de bienvenida
window.onload = function() {
  const chat = document.getElementById("chat");
  const welcomeMsg = document.createElement("div");
  welcomeMsg.className = "message ai-message";
  welcomeMsg.innerHTML = '<div class="message-label">🤖 Gemini:</div>¡Hola! Soy tu asistente ambiental. Pregúntame sobre la temperatura, humedad o consejos para el clima actual.';
  chat.appendChild(welcomeMsg);
};
</script>

</body>
</html>
)rawliteral";
}


// =======================================================
// EXTRAER TEXTO DE GEMINI (estructura moderna)
// =======================================================
String extractGeminiText(String response) {
  String key = "\"text\": \"";
  int pos = response.indexOf(key);
  if (pos == -1) {
    return "No pude interpretar la respuesta de Gemini.";
  }

  pos += key.length();
  int end = response.indexOf("\"", pos);
  if (end == -1) {
    return "Respuesta incompleta.";
  }

  String out = response.substring(pos, end);
  out.replace("\\n", "\n");
  out.replace("\\\"", "\"");
  out.replace("\\\\", "\\");

  return out;
}

// =======================================================
// API → /data
// =======================================================
void handleData() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    server.send(500, "application/json", "{\"error\":\"No se puede leer el DHT11\"}");
    return;
  }

  String json = "{\"temp\":" + String(t,1) + ",\"hum\":" + String(h,1) + "}";
  server.send(200, "application/json", json);
}

// =======================================================
// API → /ask  (llama a Gemini)
// =======================================================
void handleAsk() {
  if (!server.hasArg("question")) {
    server.send(400, "text/plain", "Falta ?question=");
    return;
  }

  String q = server.arg("question");
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  String prompt =
    "Eres un asistente ambiental experto. "
    "La temperatura actual es " + String(t) + "°C y la humedad es " + String(h) +
    "%. Responde de forma clara y concisa a la siguiente pregunta: " + q;

  WiFiClientSecure client;
  client.setInsecure(); 

  if (!client.connect("generativelanguage.googleapis.com", 443)) {
    server.send(500, "text/plain", "Error conectando a Gemini");
    return;
  }

  String url = "/v1beta/models/gemini-2.0-flash:generateContent?key=" + GEMINI_API_KEY;
  String body = "{\"contents\":[{\"parts\":[{\"text\":\"" + prompt + "\"}]}]}";

  String req =
    "POST " + url + " HTTP/1.1\r\n"
    "Host: generativelanguage.googleapis.com\r\n"
    "Content-Type: application/json\r\n"
    "Content-Length: " + String(body.length()) + "\r\n\r\n" +
    body;

  client.print(req);

  String response = "";

  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") break;
  }

  while (client.available()) {
    response += client.readString();
  }

  String out = extractGeminiText(response);

  server.send(200, "text/plain", out);
}

// =======================================================
// SETUP
// =======================================================
void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("\nIniciando DHT11...");
  dht.begin();
  delay(1500);

  Serial.println("Conectando WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }

  Serial.println("\n\n✅ WiFi Conectado!");
  Serial.print("🌐 Abre en tu navegador: http://");
  Serial.println(WiFi.localIP());

  server.on("/", [](){ server.send(200, "text/html", paginaHTML()); });
  server.on("/data", handleData);
  server.on("/ask", handleAsk);

  server.begin();
  Serial.println("Servidor iniciado 🚀");
}

// =======================================================
// LOOP
// =======================================================
void loop() {
  server.handleClient();
}