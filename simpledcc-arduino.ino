#include "LittleFS.h"

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

//#include <SPI.h>
//#include <MFRC522.h>

#include <LiteLED.h>

#include "src/dcc_reader/dcc_module.h"
#include "src/features/led_module.h"
#include "src/features/wcc_module.h"
//#include "src/features/nfc_module.h"
#include "src/features/spiffs_module.h"
#include "src/features/status_led_module.h"
#include "src/features/preferences_module.h"
#include "src/features/bdc_motor_module.h"
#include "src/network/webserver_module.h"
#include "src/features/audio_module.h"

void initLittleFS(){
  if(!LittleFS.begin()){
    Serial.println("An Error has occurred while mounting LittleFS");
    return;
  }

    Serial.println("LittleFS mounted successfully");

  Serial.println("LittleFS used space size in bytes");
  Serial.println(LittleFS.usedBytes());

  Serial.println("LittleFS total size in bytes");
  Serial.println(LittleFS.totalBytes());
}

void setup()
{

  Serial.begin(115200);
  Serial.print("Initializing SimpleDCC/WCC decoder ... Full documentation is available at https://loco.engineering/docs");
  
  initLittleFS();

  //Load board preferences
  init_preferences_module();

  //Set board config
  fill_board_connections(); //after we call this function we can get connections by using var board_connections

  setup_dcc_module();
  setup_webserver();
  setup_led();
  //setup_nfc();
  setup_status_led();
  setup_audio();
  setup_bdc_module();

  on_status_led(0x00ff00);

  // LED connection examples for a level crossing with 2 LEDs blinking alternately

  //LED tests, LED driver
  /*
  add_led_connection(0, 0, 0.06, 2000, 9000, 0); //white
  add_led_connection(0, 11, 0.09, 2000, 9000, 2000); //green
  add_led_connection(0, 10, 0.5, 2000, 9000, 4000); //yellow bottom
  add_led_connection(0, 8, 0.7, 2000, 9000, 4000); //yellow top
  add_led_connection(0, 9, 0.08, 2000, 9000, 6000); //red
  */
  //LED tests, GPIO outputs
  //add_led_connection(1,33, 0.5, 1000, 1000, 1000);
  //add_led_connection(1,34, 0.5, 1000, 1000, 1000);


  // Test LittleFS
  //deleteFile(LittleFS, "/level_crossing_1.wav");
  //deleteFile(LittleFS, "/train.wav");

  listDir(LittleFS, "/", 0);

  // Test audio(
  //play_audio_from_header_file();

  reload_and_send_media_files_list();
  //play_audio_from_spiffs("train_mon_16bit_32khz.wav", 0);
  //play_audio_from_spiffs("lev_cros1_mon_16bit_32k.wav", 0);

  //Pullup on GPIO 00 is required for Loco.Engineering flashing tools
  pinMode(0, INPUT_PULLUP);

  //Uncomment and change WiFI Tx Power if you want to increase the range or reduce the board heating
  //Possible values can be found at https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFi/src/WiFiGeneric.h, struct wifi_power_t
  WiFi.setTxPower(WIFI_POWER_5dBm);

}

void loop()
{

  
  loop_dcc_module();
  loop_webserver();
  loop_led();
  //loop_nfc();
  loop_audio();
  loop_gpio_module();
  loop_bdc_module();
  //Uncomment if you want to check memory leaks and usage
  //Data can be visualized in Serial Plotter"
  //Serial.printf("\nStack:%d,Heap:%lu\n", uxTaskGetStackHighWaterMark(NULL), (unsigned long)ESP.getFreeHeap());

}