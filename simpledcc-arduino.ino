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

#include <SPI.h>
#include <MFRC522.h>

#include <LiteLED.h>

#include "src/dcc_reader/dcc_module.h"
#include "src/features/led_module.h"
#include "src/features/wcc_module.h"
#include "src/features/nfc_module.h"
#include "src/features/spiffs_module.h"
#include "src/features/status_led_module.h"
#include "src/features/preferences_module.h"
#include "src/features/bdc_motor_module.h"
#include "src/network/webserver_module.h"
#include "src/features/audio_module.h"

#include "driver/temp_sensor.h"

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
  
  //Don't forget to add to add_led_connection some random ID as the last parameter
  /*uint8_t id_1[10] = {1,1,1,1,1,1,1,1,1,1};
  uint8_t id_2[10] = {1,1,1,1,1,1,1,1,2,1};
  uint8_t id_3[10] = {1,1,1,2,1,1,1,1,2,1};
  uint8_t id_4[10] = {1,1,1,1,4,2,1,2,2,1};
  uint8_t id_5[10] = {1,1,1,1,4,3,1,2,2,1};
  uint8_t id_6[10] = {1,1,1,1,4,5,1,2,2,1};
  uint8_t id_7[10] = {1,1,1,1,4,6,1,2,2,1};
  uint8_t id_8[10] = {1,1,1,1,4,7,1,2,2,1};
  uint8_t id_9[10] = {1,1,1,1,5,7,1,2,2,1};
  uint8_t id_10[10] = {1,1,1,1,5,8,1,2,2,1};

  add_led_connection(0, 15, 1.0, 10000, 2000, 0, id_1); // 0
  add_led_connection(0, 14, 1.0, 9000, 3000, 1000, id_2); //1
    add_led_connection(0, 13, 1.0, 8000, 4000, 2000, id_3); //1

    add_led_connection(0, 9, 1.0, 7000, 5000, 3000, id_4); //2
    add_led_connection(0, 10, 1.0, 6000, 6000, 4000, id_5); //2
    add_led_connection(0, 11, 1.0, 4000, 7000, 6000, id_6); //white*/

  //add_led_connection(0, 4, 0.2, 2000, 10000, 4000, id_2); // green
  //add_led_connection(0, 0, 0.4, 2000, 10000, 6000, id_3); //top yellow
  //add_led_connection(0, 15, 0.3, 2000, 10000, 6000, id_4); //bottom yellow

  //add_led_connection(0, 5, 0.3, 2000, 10000, 6300, id_5); //red
  //add_led_connection(0, 11, 0.1, 2000, 10000, 6300, id_6); //whit

 /* add_led_connection(0, 4, 0.3, 3000, 1500, 1500, id_5); //ped red

  add_led_connection(0, 11, 0.3, 3000, 1500, 1500, id_6); //ped red
  add_led_connection(0, 12, 0.5, 1500, 3000, 1500, id_7); //car yellow
  add_led_connection(0, 13, 0.3, 1500, 3000, 0, id_8); //car red
  add_led_connection(0, 14, 0.3, 1500, 3000, 3000, id_9); //car green
  add_led_connection(0, 15, 0.3, 1500, 3000, 0, id_10); //ped green

  add_led_connection(0, 4, 0.1, 1500, 3000, 3000, id_2); //red

  add_led_connection(0, 15, 0.6, 1500, 3000, 1500, id_3); //yellow
    add_led_connection(0, 5, 1.0, 0, 0, 0, id_4); //white

  add_led_connection(0, 11, 1.0, 0, 0, 0, id_5); //blue
    add_led_connection(0, 10, 1.0, 0, 0, 0, id_6); //blue

  add_led_connection(0, 14, 1.0, 0, 0, 0, id_7); //white
    add_led_connection(0, 15, 1.0, 0, 0, 0, id_8); //white
  
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
  //play_audio_from_spiffs("train_mon_16bit_32khz.wav", 1);
  //play_audio_from_spiffs("lev_cros1_mon_16bit_32k.wav", 0);

  //Pullup on GPIO 00 is required for Loco.Engineering flashing tools
  pinMode(0, INPUT_PULLUP);

  //Uncomment and change WiFI Tx Power if you want to increase the range or reduce the board heating
  //Possible values can be found at https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFi/src/WiFiGeneric.h, struct wifi_power_t
  WiFi.setTxPower(WIFI_POWER_2dBm);

  initTempSensor();

}

void initTempSensor(){
    temp_sensor_config_t temp_sensor = TSENS_CONFIG_DEFAULT();
    temp_sensor.dac_offset = TSENS_DAC_L2;  // TSENS_DAC_L2 is default; L4(-40°C ~ 20°C), L2(-10°C ~ 80°C), L1(20°C ~ 100°C), L0(50°C ~ 125°C)
    temp_sensor_set_config(temp_sensor);
    temp_sensor_start();
}

double next_time_to_check_temp = 0;

void loop()
{

  if (millis() > next_time_to_check_temp){
    next_time_to_check_temp = millis() + 1000;
      float result = 0;
    temp_sensor_read_celsius(&result);
    /*Serial.println(result);
    Serial.println(" °C");*/
  }

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