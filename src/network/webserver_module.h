
#ifndef WEBSERVER_MODULE_H
#define WEBSERVER_MODULE_H

#include "../features/wcc_module.h"

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Create a WebSocket object
AsyncWebSocket ws("/ws");

static void server_handle_upload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);

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

void send_dcc_packets(DCCPacket *cached_packets, uint8_t cached_packets_count)
{
  const size_t dcc_packet_size = sizeof(DCCPacket);
  uint8_t msg_to_send[dcc_packet_size * MAX_CACHED_DCC_PACKETS];
  // Set message type
  msg_to_send[0] = 4;                    // 4 - a message with DCC packets
  msg_to_send[1] = cached_packets_count; // amount of DCC packates in the message
  uint16_t msg_index = 2;

  // Iterate DCC packets in cached_packets and add to a websockets message
  for (int i = 0; i < cached_packets_count; ++i)
  {
    DCCPacket cached_packet = cached_packets[i];
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

void send_board_config_message()
{

  const size_t dcc_packet_size = sizeof(Connection);
  uint8_t msg_to_send[dcc_packet_size * connection_amount];

  // Set message type
  msg_to_send[0] = 5;                 // 5 - a message with a board config
  msg_to_send[1] = connection_amount; // amount of DCC packates in the message

  uint16_t msg_index = 2;

  // Iterate DCC packets in cached_packets and add to a websockets message
  for (uint8_t i = 0; i < connection_amount; ++i)
  {
    Connection connection = board_connections[i];

    // Fill the connection name
    for (uint8_t name_ind = 0; name_ind < CONNECTION_NAME_LENGTH; ++name_ind)
    {
      msg_to_send[msg_index++] = connection.name[name_ind];
    }

    // Output number on the chip
    msg_to_send[msg_index++] = connection.output_num;

    // Chip type
    msg_to_send[msg_index++] = connection.owner_id;

    // Fill the signal types this connection can handle
    for (uint8_t sig_type_ind = 0; sig_type_ind < CONNECTION_SIGNAL_TYPES_AMOUNT; ++sig_type_ind)
    {

      msg_to_send[msg_index++] = connection.signal_types[sig_type_ind];
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
  Serial.println(info->len);
  Serial.println("Content");
  for (int i = 0; i < len; i++)
  {
    Serial.print(data[i]);
    Serial.print("|");
  }
  Serial.println();
  // Check the type - first byte
  if (data[0] == 1)
  {
    // This message is WCC board settings
    // Move a pointer with data to the second byte
    data++;
    handle_wcc_message(data, len);
  }

  // Check the type - first byte
  if (data[0] == 2)
  {
    // This message is WCC event
    // Move a pointer with data to the second byte
    data++;
    handle_wcc_event(data, len);
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

    // Send the board configuration message
    send_board_config_message();

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

// Handles media file uploads to the SPIFFS directory
static void server_handle_SPIFFS_upload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
  // make sure authenticated before allowing upload
  String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
  Serial.println(logmessage);

  if (!index)
  {
    logmessage = "Upload Start: " + String(filename);
    // open the file on first call and store the file handle in the request object
    request->_tempFile = SPIFFS.open("/" + filename, "w");
    Serial.println(logmessage);
  }

  if (len)
  {
    // stream the incoming chunk to the opened file
    request->_tempFile.write(data, len);
    logmessage = "Writing file: " + String(filename) + " index=" + String(index) + " len=" + String(len);
    Serial.println(logmessage);
  }

  if (final)
  {
    logmessage = "Upload Complete: " + String(filename) + ",size: " + String(index + len);
    // close the file handle as the upload is now done
    request->_tempFile.close();
    Serial.println(logmessage);
    request->redirect("/");
  }
}

// Handles OTA firmware update
static void server_handle_OTA_update(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{

  String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
  Serial.println(logmessage);

  if (!index)
  {
    logmessage = "OTA Update Start: " + String(filename);
    Serial.println(logmessage);
    if (!Update.begin(UPDATE_SIZE_UNKNOWN))
    { // start with max available size
      Update.printError(Serial);
    }
  }

  if (len)
  {
    // flashing firmware to ESP
    if (Update.write(data, len) != len)
    {
      Update.printError(Serial);
    }
    logmessage = "Writing file: " + String(filename) + " index=" + String(index) + " len=" + String(len);
    Serial.println(logmessage);
  }

  if (final)
  {
    if (Update.end(true))
    { // true to set the size to the current progress
      logmessage = "OTA Complete: " + String(filename) + ",size: " + String(index + len);
      Serial.println(logmessage);
      request->send(200);
      delay(500);
      WiFi.disconnect();
      delay(500);
      ESP.restart();

    }
    else
    {
      Update.printError(Serial);
    }
  }
}

static void server_handle_upload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
  if (filename.endsWith(".bin"))
  {
    server_handle_OTA_update(request, filename, index, data, len, final);
  }
  else
  {
    server_handle_SPIFFS_upload(request, filename, index, data, len, final);
  }
}

void setup_webserver()
{

  initWiFi();
  initWebSocket();

  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");

  server.onNotFound([](AsyncWebServerRequest *request) {
    if (request->method() == HTTP_OPTIONS) {
      request->send(200);
    } else {
      request->send(404);
    }
  });

  // Web Server Root URL
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/app.html", "text/html"); });

  // run handleUpload function when any file is uploaded
   server.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request) {
        request->send(200); }, server_handle_upload);

  server.serveStatic("/", SPIFFS, "/");

  // Start server
  server.begin();
}

void loop_webserver()
{
  // ws.cleanupClients();
}

#endif