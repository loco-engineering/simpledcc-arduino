
#ifndef WEBSERVER_MODULE_H
#define WEBSERVER_MODULE_H

#include "../features/wcc_module.h"
#include "../features/spiffs_module.h"

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Create a WebSocket object
AsyncWebSocket ws("/ws");

static void server_handle_upload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);

// Initialize WiFi
void initWiFi()
{
  WiFi.setTxPower(WIFI_POWER_MINUS_1dBm);
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

    msg_to_send[msg_index++] = cached_packet.packet_amount;

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

void reload_and_send_media_files_list()
{
  fs::FS fs = LittleFS;
  const char *dirname = "/";
  uint8_t levels = 0;

  // Fill a message
  uint8_t *msg;

  size_t msg_index = 0;

  Serial.printf("Fill WCC message with media files in : %s\r\n", dirname);
  // We allocate 100 bytes but will reallocate memory if 100 bytes are not enough
  uint16_t allocated_bytes = 100;
  uint8_t files_amount = 0;

  msg = (uint8_t *)ps_calloc(allocated_bytes, sizeof(uint8_t));

  // Set message type
  msg[0] = 6; // 6 - a message with media files
  msg[1] = 0; // amount of files in the message, we'll fill that value with real value later
  msg_index = 2;

  File root = fs.open(dirname);
  if (!root)
  {
    Serial.println("- failed to open directory");
    return;
  }
  if (!root.isDirectory())
  {
    Serial.println(" - not a directory");
    return;
  }

  // Clean and allocate new memory for media files list
  // ToDo: Arduino cannot allocate dynamic memory for a struct with File inside so now we just used predefined array of MediaFile
  /*if (board_settings.media_files != NULL)
  {
    // Iterate and clean states
    for (unsigned int file_number = 0; file_number < board_settings.media_files_amount; ++file_number)
    {
      MediaFile media_file = board_settings.media_files[file_number];
      if (media_file.file_name != NULL)
      {
        free(media_file.file_name);
      }
    }

    free(board_settings.media_files);
    board_settings.media_files = NULL;
    board_settings.media_files_amount = 0;
  }

  board_settings.media_files = (MediaFile *)ps_malloc(10* sizeof(MediaFile));*/

  File file = root.openNextFile();
  while (file)
  {
    if (file.isDirectory())
    {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels)
      {
        listDir(fs, file.path(), levels - 1);
      }
    }
    else
    {

      if (strstr(file.name(), ".wav") != NULL || strstr(file.name(), ".aac") != NULL || strstr(file.name(), ".mp3") != NULL)
      {

        Serial.print("  FILE: ");
        Serial.print(file.name());

        board_settings.media_files[files_amount].file_name = (char *)ps_malloc(strlen(file.name()) * sizeof(char));

        strcpy(board_settings.media_files[files_amount].file_name, file.name());
        board_settings.media_files[files_amount].status = 0;

        // Open file and keep File ref
        char path[50] = "/";
        strcat(path, file.name());
        board_settings.media_files[files_amount].file = LittleFS.open(path);

        uint8_t *file_name = (uint8_t *)file.name();

        for (uint8_t c = *file_name; c != '\0'; c = *++file_name)
        {
          msg[msg_index++] = c;
          if (msg_index == allocated_bytes)
          {
            allocated_bytes += 100;
            // Reallocate memory
            msg = (uint8_t *)ps_realloc(msg, allocated_bytes);
          }
        }
        msg[msg_index++] = '\0';
        ++files_amount;
      }
      file = root.openNextFile();
    }
  }

  msg[1] = files_amount; // amount of files in the message
  board_settings.media_files_amount = files_amount;

  Serial.println("msg to send with files");
  for (int i = 0; i < msg_index; i++)
  {
    Serial.print(msg[i]);
    Serial.print("|");
  }
  Serial.println();

  ws.binaryAll(msg, msg_index);

  free(msg);
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

  // Check the type - first byte
  if (data[0] == 3)
  {
    // This message is WCC message to manage media files
    // Move a pointer with data to the second byte
    data++;
    handle_wcc_media_file_message(data, len);
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
    //} width="180"
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

    // Send a list with media files
    reload_and_send_media_files_list();

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

size_t cache_ind = 0;
uint8_t cache_data[10000];

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

    int headers = request->headers();
    for (uint8_t i = 0; i < headers; i++)
    {
      AsyncWebHeader *h = request->getHeader(i);
      if (strcmp(h->name().c_str(), "del-prev") == 0)
      {
        Serial.println("Deleting prev media file");
        deleteFile(LittleFS, String("/" + filename).c_str());
      }
    }

    request->_tempFile = LittleFS.open("/" + filename, FILE_APPEND);

    Serial.println(logmessage);
  }

  if (len)
  {
    // stream the incoming chunk to the opened file
    request->_tempFile.write(data, len);

    /*if (cache_ind > 8000){

      request->_tempFile.write(cache_data, cache_ind);
      cache_ind = 0;

    }

 for (size_t i = 0; i < len; i++) {
            cache_data[cache_ind + i] = data[i];
        }

     cache_ind += len;*/

    logmessage = "Writing file: " + String(filename) + " index=" + String(index) + " len=" + String(len);
    Serial.println(logmessage);
  }

  if (final)
  {
    logmessage = "Upload Complete: " + String(filename) + ",size: " + String(index + len);
    // close the file handle as the upload is now done

    /*     request->_tempFile.write(cache_data, cache_ind);
     cache_ind = 0;*/
    request->_tempFile.close();
    listDir(LittleFS, "/", 0);

    Serial.println(logmessage);
    request->redirect("/");

    // Reload files and send to the client
    reload_and_send_media_files_list();
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
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "*");

  server.onNotFound([](AsyncWebServerRequest *request)
                    {
    if (request->method() == HTTP_OPTIONS) {
      request->send(200);
    } else {
      request->send(404);
    } });

  // Web Server Root URL
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(LittleFS, "/app.html", "text/html"); });

  // run handleUpload function when any file is uploaded
  server.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request)
            { request->send(200); }, server_handle_upload);

  server.serveStatic("/", LittleFS, "/");

  // Start server
  server.begin();
}

void loop_webserver()
{
  // ws.cleanupClients();
}

#endif