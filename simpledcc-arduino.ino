#include "SPIFFS.h"
#include "FS.h"
#include "config.h"
#include "structures.h"

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <PCA9955B.h>

#include <Update.h>

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
#include "src/features/wcc_module.h"
#include "src/network/webserver_module.h"

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

  //Set board config
  fill_board_connections(); //after we call this function we can get connections by using var board_connections
  for (int i = 0; i < 16; i++) {
      Serial.println( board_connections[i].name);
  }

  setup_dcc_module();
  setup_webserver();
  setup_led();
  //setup_nfc();
  setup_status_led();
  on_status_led(0x0000ff);

  // LED connection examples for a level crossing with 2 LEDs blinking alternately
  //add_led_connection(0, 0, 0.5, 1000, 1000, 0);
  //add_led_connection(1, 2, 0.5, 1000, 1000, 1000);

  // Test SPIFFS
  listDir(SPIFFS, "/", 0);
  // Test audio
  //play_audio_from_header_file();
  
}

void loop()
{
  loop_dcc_module();
  loop_webserver();
  loop_led();
  //loop_nfc();
  loop_audio();
  loop_gpio_module();
  //Uncomment if you want to check memory leaks and usage
  //Data can be visualized in Serial Plotter"
  //Serial.printf("\nStack:%d,Heap:%lu\n", uxTaskGetStackHighWaterMark(NULL), (unsigned long)ESP.getFreeHeap());

}