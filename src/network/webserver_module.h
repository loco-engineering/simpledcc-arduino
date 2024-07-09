
#ifndef WEBSERVER_MODULE_H
#define WEBSERVER_MODULE_H

#include "../features/wcc_module.h"

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Create a WebSocket object
AsyncWebSocket ws("/ws");

// Initialize WiFi
void initWiFi()
{
  WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);

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
  const size_t dcc_packet_size = sizeof(dcc_packet);
  uint8_t msg_to_send[dcc_packet_size * MAX_CACHED_DCC_PACKETS];
  // Set message type
  msg_to_send[0] = 4;                    // 4 - a message with DCC packets
  msg_to_send[1] = cached_packets_count; // amount of DCC packates in the message
  uint16_t msg_index = 2;

  // Iterate DCC packets in cached_packets and add to a websockets message
  for (int i = 0; i < cached_packets_count; ++i)
  {
    struct dcc_packet cached_packet = cached_packets[i];
    msg_to_send[msg_index++] = cached_packet.packet_type;
    msg_to_send[msg_index++] = cached_packet.address[0];
    msg_to_send[msg_index++] = cached_packet.address[1];
    msg_to_send[msg_index++] = cached_packet.raw_packet_length;

    for (int k = 0; k < cached_packet.raw_packet_length; ++k)
    {
      msg_to_send[msg_index++] = cached_packet.raw_packet[k];
    }
    msg_to_send[msg_index++] = cached_packet.user_data_length;
    for (int k = 0; k < cached_packet.user_data_length; ++k)
    {
      msg_to_send[msg_index++] = cached_packet.user_data[k];
    }
  }
  ws.binaryAll(msg_to_send, msg_index);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
     
     
    Serial.println("=======================");
    Serial.println("New message over WebSocket received");
    Serial.println("Size");
    Serial.println(info->len );
    Serial.println("Content");
    for(int i=0; i < len; i++) {
      Serial.print(data[i]);
      Serial.print("|");
    }
    Serial.println();
    //Check the type - first byte
    if (data[0] == 1){
      //This message is WCC message, trying to parse
      //Move a pointer with data to the second byte
      data++;
      handle_wcc_message(data, len);
    }
    Serial.println("=======================");

  if (info->final && info->index == 0 && info->len == len)
  {
    String message = (char *)data;
    Serial.print(message);
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
  char log_buffer[100]; // Used for printing messages with formatting

  switch (type)
  {
  case WS_EVT_CONNECT:
    sprintf(log_buffer, "WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
    Serial.print(log_buffer);
    break;
  case WS_EVT_DISCONNECT:
    sprintf(log_buffer, "WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
    Serial.print(log_buffer);
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
  // ws.cleanupClients();
}

#endif