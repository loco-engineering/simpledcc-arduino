

struct dcc_packet{
   uint8_t raw_packet[8];
   uint8_t raw_packet_length;

   uint8_t packet_type; //0 - undefined type, 1 - speed_dcc_packet, 2 - function_dcc_packet

   uint8_t address[2];

   uint8_t user_data[10];
   uint8_t user_data_length;
};

