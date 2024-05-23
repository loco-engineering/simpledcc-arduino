#include "SPIFFS.h"
#include "FS.h"
#include "config.h"
#include "structures.h"

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <PCA9955B.h>

#include "driver/i2s.h"

#include <SPI.h>
#include <MFRC522.h>

#include <LiteLED.h>

#include "src/dcc_reader/dcc_module.h"
#include "src/features/led_module.h"
#include "src/features/nfc_module.h"
#include "src/features/audio_module.h"
#include "src/features/spiffs_module.h"
#include "src/features/status_led_module.h"

// Initialize SPIFFS
void initSPIFFS()
{
  if (!SPIFFS.begin(true))
  {
    Serial.println("An error has occurred while mounting SPIFFS");
  }
  Serial.println("SPIFFS mounted successfully");

  Serial.println("SPIFFS used space size in bytes");
  Serial.println(SPIFFS.usedBytes());

  Serial.println("SPIFFS total size in bytes");
  Serial.println(SPIFFS.totalBytes());
}

void setup()
{
  Serial.begin(115200);
  Serial.print("Initializing SimpleDCC/WCC decoder ... Full documentation is available at https://loco.engineering/docs");

  initSPIFFS();

  setup_dcc_module();
  setup_webserver();
  setup_led();
  setup_nfc();
  setup_status_led();

  // LED connection examples for a level crossing with 2 LEDs blinking alternately
  // add_led_connection(0, 0, 0.5, 1000, 1000, 0);
  // add_led_connection(1, 2, 0.5, 1000, 1000, 1000);

  // Test SPIFFS
  listDir(SPIFFS, "/", 0);

  // Test audio
  // play_audio_from_header_file();
}

void loop()
{
  loop_dcc_module();
  loop_webserver();
  loop_led();
  loop_nfc();
  loop_audio();
}