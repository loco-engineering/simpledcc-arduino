// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Create a WebSocket object
AsyncWebSocket ws("/ws");

// Initialize WiFi
void initWiFi()
{
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  while (!MDNS.begin("loco"))
  {
    Serial.println("Starting mDNS...");
    delay(1000);
  }
}

void send_dcc_packets(struct dcc_packet *cached_packets, uint8_t cached_packets_count)
{
  const size_t dcc_packet_size = 5;//sizeof(dcc_packet);
  uint8_t msg_to_send[dcc_packet_size * MAX_CACHED_DCC_PACKETS];
 // ws.binaryAll(msg, len);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len)
  {
     data[len] = 0;
     String message = (char*)data;
     Serial.print (message);
    //  Check if the message is "getReadings"
    // if (strcmp((char*)data, "getReadings") == 0) {
    // if it is, send current sensor readings
    // String sensorReadings = getSensorReadings();
    // Serial.print(sensorReadings);
    // notifyClients(sensorReadings);
    //}
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
  char log_buffer[100]; //Used for printing messages with formatting

  switch (type)
  {
  case WS_EVT_CONNECT:
    sprintf (log_buffer, "WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
    Serial.print (log_buffer);
    break;
  case WS_EVT_DISCONNECT:
    sprintf (log_buffer, "WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
    Serial.print (log_buffer);
    break;
  case WS_EVT_DATA:
    handleWebSocketMessage(arg, data, len);
    break;
  case WS_EVT_PONG:
  case WS_EVT_ERROR:
    break;
  }
}

void initWebSocket()
{
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

void setup_webserver()
{

  initWiFi();
  initWebSocket();

  // Web Server Root URL
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/app.html", "text/html"); });

  server.serveStatic("/", SPIFFS, "/");

  // Start server
  server.begin();
}

void loop_webserver()
{
  //ws.cleanupClients();
}