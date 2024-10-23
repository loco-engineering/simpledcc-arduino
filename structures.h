#ifndef STRUCTURES_H
#define STRUCTURES_H

typedef struct DCCPacket{
   uint8_t raw_packet[8];
   uint8_t raw_packet_length;

   uint8_t packet_type; //0 - undefined type, 1 - speed_dcc_packet, 2 - function_dcc_packet

   uint8_t address[2];

   uint8_t user_data[10];
   uint8_t user_data_length;

   uint8_t packet_amount;
};

struct gpio_connection{
   uint8_t type; //0 - LED driver, 1 - GPIO of ESP32
   uint8_t signal_type;
   uint8_t io_pin;
   uint8_t connection_index;
   float output_value;
   bool is_on;
   bool is_enabled;
   unsigned long on_duration;
   unsigned long off_duration;
   double next_on;
   double next_off;
   uint8_t id[10];
};

#endif