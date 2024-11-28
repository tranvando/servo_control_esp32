#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <ESP32Servo.h> // Servo control library

// Wi-Fi credentials
const char* ssid = "IT";
const char* password = "23456789@";

// Define servo pin
const int servoPin = 4; 
Servo myServo; 

// Create WebSocket server object
AsyncWebServer server(80);
AsyncWebSocket ws("/ws"); // Define WebSocket at "/ws"

// Wi-Fi connection
void setup() {
  Serial.begin(115200);
  
  // Wi-Fi connection
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.println(WiFi.localIP()); // Print ESP32's IP address

  // Start Servo
  myServo.attach(servoPin);  
  myServo.write(90); // Set initial servo angle to 90 degrees

  // Define WebSocket event handler
  ws.onEvent(onWsEvent);

  // Start WebSocket
  server.addHandler(&ws);

  // Serve HTML page with progress bar and WebSocket
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = "<!DOCTYPE html><html><head><title>servo control</title></head><body>"
                  "<h1>Servo Control</h1>"
                  "<label for='progress'>steering wheel angle: </label>"
                  "<input type='range' id='progress' min='0' max='180' value='90' step='1' oninput='updateServo(this.value)'>"
                  "<p>corner: <span id='angle'>90</span></p>"
                  "<script>"
                  "var socket = new WebSocket('ws://' + window.location.hostname + '/ws');"
                  "socket.onopen = function() { console.log('WebSocket kết nối'); };"
                  "socket.onmessage = function(event) { document.getElementById('angle').innerHTML = event.data; };"
                  "function updateServo(value) {"
                  "  document.getElementById('angle').innerHTML = value;"
                  "  socket.send(value);"
                  "}"
                  "</script></body></html>";
    request->send(200, "text/html", html);
  });

  // Start server
  server.begin();
}

// WebSocket event handler
void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, 
               void *arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    Serial.println("WebSocket connected");
  }
  else if (type == WS_EVT_DISCONNECT) {
    Serial.println("WebSocket disconnected");
  }
  else if (type == WS_EVT_DATA) {
    String msg = String((char*)data);
    int angle = msg.toInt();
    if (angle >= 0 && angle <= 180) {
      myServo.write(angle); // Control servo based on angle received from WebSocket
      Serial.print("Servo angle: ");
      Serial.println(angle);
      client->text(String(angle)); // Send the updated angle back to the browser
    }
  }
}

void loop() {
  // Cleanup WebSocket clients on new events
  ws.cleanupClients();
}
