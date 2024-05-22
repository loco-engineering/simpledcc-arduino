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
#include <MFRC522v2.h>
#include <MFRC522DriverSPI.h>
//#include <MFRC522DriverI2C.h>
#include <MFRC522DriverPinSimple.h>
#include <MFRC522Debug.h>

#include "src/dcc_reader/dcc_module.h"
#include "src/features/led_module.h"
#include "src/features/nfc_module.h"
#include "src/features/audio_module.h"
#include "src/features/spiffs_module.h"

// Initialize SPIFFS
void initSPIFFS()
{
  if (!SPIFFS.begin(true))
  {
    Serial.println("An error has occurred while mounting SPIFFS");
  }
  Serial.println("SPIFFS mounted successfully");
}

void setup()
{
  Serial.begin(115200);
  while (!Serial)
  {
    ; // wait for serial port to connect. Needed for native USB
  }
  Serial.print("Initializing DCC reader...");

  setup_dcc_module();

  initSPIFFS();
  setup_webserver();
  setup_led();
  //setup_nfc();
  // LED connection examples for a level crossing with 2 LEDs blinking alternately
  //add_led_connection(1, 1, 0.5, 1000, 1000, 0);
  //add_led_connection(1, 2, 0.5, 1000, 1000, 1000);

  listDir(SPIFFS, "/", 0);

  //Test audio
  play_audio();
}

void loop()
{
  loop_dcc_module();
  loop_webserver();
  loop_led();
  //loop_nfc();
  loop_audio();
}