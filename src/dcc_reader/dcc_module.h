#include "NmraDcc.h"
#include "../network/webserver_module.h"
#include "../handlers/event_handler.h"
NmraDcc Dcc;

DCCPacket cached_packets[MAX_CACHED_DCC_PACKETS];
uint8_t cached_packets_count = 0;

void notifyDccSpeed(uint16_t Addr, DCC_ADDR_TYPE AddrType, uint8_t Speed, DCC_DIRECTION Dir, DCC_SPEED_STEPS SpeedSteps)
{
  Serial.print("notifyDccSpeed: Addr: ");
  Serial.print(Addr, DEC);
  Serial.print((AddrType == DCC_ADDR_SHORT) ? "-S" : "-L");
  Serial.print(" Speed: ");
  Serial.print(Speed, DEC);
  Serial.print(" Steps: ");
  Serial.print(SpeedSteps, DEC);
  Serial.print(" Dir: ");
  Serial.println((Dir == DCC_DIR_FWD) ? "Forward" : "Reverse");

  if (Addr == 4){
  if (Dir == DCC_DIR_FWD){

                                ledcWrite(6, Speed*32);

                        ledcWrite(7, 0);


  }else{
                                ledcWrite(7, Speed*32);

                        ledcWrite(6, 0);
  }
  }




    uint8_t cur_packet_index = cached_packets_count - 1;

  // Set packet type
  cached_packets[cur_packet_index].packet_type = 1;
  // Save address
  cached_packets[cur_packet_index].address[0] = Addr & 0xff;
  cached_packets[cur_packet_index].address[1] = (Addr >> 8);
  // Save user data
  cached_packets[cur_packet_index].user_data[0] = Speed;
  cached_packets[cur_packet_index].user_data[1] = Dir;
  cached_packets[cur_packet_index].user_data[2] = SpeedSteps;
  cached_packets[cur_packet_index].user_data_length = 3;

  cached_packets[cur_packet_index].packet_amount = 1;

  //Check if the previous packet the same
  if (cached_packets_count > 1){
    uint8_t prev_packet_index = cur_packet_index - 1;

    if (cached_packets[prev_packet_index].packet_type == cached_packets[cur_packet_index].packet_type &&
      cached_packets[prev_packet_index].address[0] == cached_packets[cur_packet_index].address[0] &&
      cached_packets[prev_packet_index].address[1] == cached_packets[cur_packet_index].address[1] &&
      cached_packets[prev_packet_index].user_data_length == cached_packets[cur_packet_index].user_data_length &&
      cached_packets[prev_packet_index].user_data[0] == cached_packets[cur_packet_index].user_data[0] &&
      cached_packets[prev_packet_index].user_data[1] == cached_packets[cur_packet_index].user_data[1] &&
      cached_packets[prev_packet_index].user_data[2] == cached_packets[cur_packet_index].user_data[2]){
        //Increase the amount of packets of the previous packet
          cached_packets[prev_packet_index].packet_amount++;
          cached_packets_count--; // we should reduce the counter because we don't add a new packet

      }

  }
  
};

void notifyDccFunc(uint16_t Addr, DCC_ADDR_TYPE AddrType, FN_GROUP FuncGrp, uint8_t FuncState)
{
  Serial.print("notifyDccFunc: Addr: ");
  Serial.print(Addr, DEC);
  Serial.print((AddrType == DCC_ADDR_SHORT) ? 'S' : 'L');
  Serial.print("  Function Group: ");
  Serial.print(FuncGrp, DEC);

    uint8_t cur_packet_index = cached_packets_count - 1;

  // Set packet type
  cached_packets[cur_packet_index].packet_type = 2;
  // Save address
  cached_packets[cur_packet_index].address[0] = Addr & 0xff;
  cached_packets[cur_packet_index].address[1] = (Addr >> 8);
  // Save user data
  cached_packets[cur_packet_index].user_data[0] = FuncGrp;
  cached_packets[cur_packet_index].user_data[1] = FuncState;
  cached_packets[cur_packet_index].user_data_length = 2;
  cached_packets[cur_packet_index].packet_amount = 1;

//Check if the previous packet the same
  if (cached_packets_count > 1){
    uint8_t prev_packet_index = cur_packet_index - 1;

    if (cached_packets[prev_packet_index].packet_type == cached_packets[cur_packet_index].packet_type &&
      cached_packets[prev_packet_index].address[0] == cached_packets[cur_packet_index].address[0] &&
      cached_packets[prev_packet_index].address[1] == cached_packets[cur_packet_index].address[1] &&
      cached_packets[prev_packet_index].user_data_length == cached_packets[cur_packet_index].user_data_length &&
      cached_packets[prev_packet_index].user_data[0] == cached_packets[cur_packet_index].user_data[0] &&
      cached_packets[prev_packet_index].user_data[1] == cached_packets[cur_packet_index].user_data[1]){
        //Increase the amount of packets of the previous packet
          cached_packets[prev_packet_index].packet_amount++;
          cached_packets_count--; // we should reduce the counter because we don't add a new packet
      }

  }

  switch (FuncGrp)
  {
#ifdef NMRA_DCC_ENABLE_14_SPEED_STEP_MODE
  case FN_0:
    Serial.print(" FN0: ");
    Serial.println((FuncState & FN_BIT_00) ? "1  " : "0  ");
    break;
#endif

  case FN_0_4:
    if (Dcc.getCV(CV_29_CONFIG) & CV29_F0_LOCATION) // Only process Function 0 in this packet if we're not in Speed Step 14 Mode
    {
      Serial.print(" FN 0: ");
      Serial.print((FuncState & FN_BIT_00) ? "1  " : "0  ");
    }

    Serial.print(" FN 1-4: ");
    Serial.print((FuncState & FN_BIT_01) ? "1  " : "0  ");
    Serial.print((FuncState & FN_BIT_02) ? "1  " : "0  ");
    Serial.print((FuncState & FN_BIT_03) ? "1  " : "0  ");
    Serial.println((FuncState & FN_BIT_04) ? "1  " : "0  ");
    break;

  case FN_5_8:
    Serial.print(" FN 5-8: ");
    Serial.print((FuncState & FN_BIT_05) ? "1  " : "0  ");
    Serial.print((FuncState & FN_BIT_06) ? "1  " : "0  ");
    Serial.print((FuncState & FN_BIT_07) ? "1  " : "0  ");
    Serial.println((FuncState & FN_BIT_08) ? "1  " : "0  ");
    break;

  case FN_9_12:
    Serial.print(" FN 9-12: ");
    Serial.print((FuncState & FN_BIT_09) ? "1  " : "0  ");
    Serial.print((FuncState & FN_BIT_10) ? "1  " : "0  ");
    Serial.print((FuncState & FN_BIT_11) ? "1  " : "0  ");
    Serial.println((FuncState & FN_BIT_12) ? "1  " : "0  ");
    break;

  case FN_13_20:
    Serial.print(" FN 13-20: ");
    Serial.print((FuncState & FN_BIT_13) ? "1  " : "0  ");
    Serial.print((FuncState & FN_BIT_14) ? "1  " : "0  ");
    Serial.print((FuncState & FN_BIT_15) ? "1  " : "0  ");
    Serial.print((FuncState & FN_BIT_16) ? "1  " : "0  ");
    Serial.print((FuncState & FN_BIT_17) ? "1  " : "0  ");
    Serial.print((FuncState & FN_BIT_18) ? "1  " : "0  ");
    Serial.print((FuncState & FN_BIT_19) ? "1  " : "0  ");
    Serial.println((FuncState & FN_BIT_20) ? "1  " : "0  ");
    break;

  case FN_21_28:
    Serial.print(" FN 21-28: ");
    Serial.print((FuncState & FN_BIT_21) ? "1  " : "0  ");
    Serial.print((FuncState & FN_BIT_22) ? "1  " : "0  ");
    Serial.print((FuncState & FN_BIT_23) ? "1  " : "0  ");
    Serial.print((FuncState & FN_BIT_24) ? "1  " : "0  ");
    Serial.print((FuncState & FN_BIT_25) ? "1  " : "0  ");
    Serial.print((FuncState & FN_BIT_26) ? "1  " : "0  ");
    Serial.print((FuncState & FN_BIT_27) ? "1  " : "0  ");
    Serial.println((FuncState & FN_BIT_28) ? "1  " : "0  ");
    break;
  }
}

// This function is called whenever a normal DCC Turnout Packet is received and we're in Board Addressing Mode
void notifyDccAccTurnoutBoard(uint16_t BoardAddr, uint8_t OutputPair, uint8_t Direction, uint8_t OutputPower)
{
  Serial.print("notifyDccAccTurnoutBoard: ");
  Serial.print(BoardAddr, DEC);
  Serial.print(',');
  Serial.print(OutputPair, DEC);
  Serial.print(',');
  Serial.print(Direction, DEC);
  Serial.print(',');
  Serial.println(OutputPower, HEX);

    uint8_t cur_packet_index = cached_packets_count - 1;

  // Set packet type
  cached_packets[cur_packet_index].packet_type = 4; // Turnout Packet
  // Save address
  cached_packets[cur_packet_index].address[0] = BoardAddr & 0xff;
  cached_packets[cur_packet_index].address[1] = (BoardAddr >> 8);
  // Save user data
  cached_packets[cur_packet_index].user_data[0] = Direction;
  cached_packets[cur_packet_index].user_data[1] = OutputPower;
  cached_packets[cur_packet_index].user_data[2] = OutputPair;
  cached_packets[cur_packet_index].user_data_length = 3;

    cached_packets[cur_packet_index].packet_amount = 1;

//Check if the previous packet the same
  if (cached_packets_count > 1){
    uint8_t prev_packet_index = cur_packet_index - 1;

    if (cached_packets[prev_packet_index].packet_type == cached_packets[cur_packet_index].packet_type &&
      cached_packets[prev_packet_index].address[0] == cached_packets[cur_packet_index].address[0] &&
      cached_packets[prev_packet_index].address[1] == cached_packets[cur_packet_index].address[1] &&
      cached_packets[prev_packet_index].user_data_length == cached_packets[cur_packet_index].user_data_length &&
      cached_packets[prev_packet_index].user_data[0] == cached_packets[cur_packet_index].user_data[0] &&
      cached_packets[prev_packet_index].user_data[1] == cached_packets[cur_packet_index].user_data[1] &&
      cached_packets[prev_packet_index].user_data[2] == cached_packets[cur_packet_index].user_data[2]){
        //Increase the amount of packets of the previous packet
          cached_packets[prev_packet_index].packet_amount++;
          cached_packets_count--; // we should reduce the counter because we don't add a new packet

      }

  }

}

// This function is called whenever a normal DCC Turnout Packet is received and we're in Output Addressing Mode
void notifyDccAccTurnoutOutput(uint16_t Addr, uint8_t Direction, uint8_t OutputPower)
{
  Serial.print("notifyDccAccTurnoutOutput: ");
  Serial.print(Addr, DEC);

  Serial.print(',');
  Serial.print(Direction, DEC);
  Serial.print(',');
  Serial.println(OutputPower, HEX);

    uint8_t cur_packet_index = cached_packets_count - 1;

  // Set packet type
  cached_packets[cur_packet_index].packet_type = 4; // Turnout Packet
  // Save address

  cached_packets[cur_packet_index].address[0] = Addr & 0xff;
  cached_packets[cur_packet_index].address[1] = (Addr >> 8);

  // Save user data
  cached_packets[cur_packet_index].user_data[0] = Direction;
  cached_packets[cur_packet_index].user_data[1] = OutputPower;
  cached_packets[cur_packet_index].user_data_length = 2;
    cached_packets[cur_packet_index].packet_amount = 1;

  process_dcc_turnout(&cached_packets[cur_packet_index]);
  
  //Check if the previous packet the same
  if (cached_packets_count > 1){
    uint8_t prev_packet_index = cur_packet_index - 1;

    if (cached_packets[prev_packet_index].packet_type == cached_packets[cur_packet_index].packet_type &&
      cached_packets[prev_packet_index].address[0] == cached_packets[cur_packet_index].address[0] &&
      cached_packets[prev_packet_index].address[1] == cached_packets[cur_packet_index].address[1] &&
      cached_packets[prev_packet_index].user_data_length == cached_packets[cur_packet_index].user_data_length &&
      cached_packets[prev_packet_index].user_data[0] == cached_packets[cur_packet_index].user_data[0] &&
      cached_packets[prev_packet_index].user_data[1] == cached_packets[cur_packet_index].user_data[1]){
        //Increase the amount of packets of the previous packet
          cached_packets[prev_packet_index].packet_amount++;
          cached_packets_count--; // we should reduce the counter because we don't add a new packet

      }

  }

}

// This function is called whenever a DCC Signal Aspect Packet is received
void notifyDccSigOutputState(uint16_t Addr, uint8_t State)
{
  Serial.print("notifyDccSigOutputState: ");
  Serial.print(Addr, DEC);
  Serial.print(',');
  Serial.println(State, HEX);

    uint8_t cur_packet_index = cached_packets_count - 1;

  // Set packet type
  cached_packets[cur_packet_index].packet_type = 3; // Signal Aspect Packet
  // Save address
  cached_packets[cur_packet_index].address[0] = Addr & 0xff;
  cached_packets[cur_packet_index].address[1] = (Addr >> 8);
  // Save user data
  cached_packets[cur_packet_index].user_data[0] = State;
  cached_packets[cur_packet_index].user_data_length = 1;

    cached_packets[cur_packet_index].packet_amount = 1;

  process_dcc_signal(&cached_packets[cur_packet_index]);

  //Check if the previous packet the same
  if (cached_packets_count > 1){
    uint8_t prev_packet_index = cur_packet_index - 1;

    if (cached_packets[prev_packet_index].packet_type == cached_packets[cur_packet_index].packet_type  &&
      cached_packets[prev_packet_index].address[0] == cached_packets[cur_packet_index].address[0] &&
      cached_packets[prev_packet_index].address[1] == cached_packets[cur_packet_index].address[1] &&
      cached_packets[prev_packet_index].user_data_length == cached_packets[cur_packet_index].user_data_length &&
      cached_packets[prev_packet_index].user_data[0] == cached_packets[cur_packet_index].user_data[0] ){
        //Increase the amount of packets of the previous packet
          cached_packets[prev_packet_index].packet_amount++;
          cached_packets_count--; // we should reduce the counter because we don't add a new packet

      }

  }
}

void notifyDccMsg(DCC_MSG *Msg)
{
  //Skip idle DCC packets
  if (Msg->Size == 3 && Msg->Data[0] == 0xFF && Msg->Data[1] == 0 && Msg->Data[2] == 0xFF){
    return;
  }

  Serial.print("notifyDccMsg: ");

  String ws_msg = "";

  for (uint8_t i = 0; i < Msg->Size; i++)
  {
    Serial.print(Msg->Data[i], HEX);
    Serial.write(' ');

    ws_msg += String(Msg->Data[i]) + ' ';
    cached_packets[cached_packets_count].raw_packet[i] = Msg->Data[i];
  }
  Serial.println();

  cached_packets[cached_packets_count].raw_packet_length = Msg->Size;
  // Reset the DCC packet type to undefined
  cached_packets[cached_packets_count].packet_type = 0;
  cached_packets[cached_packets_count].user_data_length = 1;

  ++cached_packets_count;
}

void setup_dcc_module()
{

  Dcc.pin(preferences_dcc_pin(), 0);

  Dcc.init(MAN_ID_DIY, 0, CV29_ACCESSORY_DECODER | CV29_OUTPUT_ADDRESS_MODE, 0);
  //Dcc.init(MAN_ID_DIY, 0, 0, 0);
  Serial.print("DCC reader is ready...");

const int freq = 5000;
const int resolution = 12;

  /*  ledcSetup(6, freq, resolution);
  ledcAttachPin(13, 6);

          ledcSetup(7, freq, resolution);
  ledcAttachPin(14, 7);*/

}

double time_from_last_sent_dcc_packet = 0;

void loop_dcc_module()
{
  Dcc.process();

  // Send cached packets to the web app when
  //  - cached_packets_count == MAX_CACHED_DCC_PACKETS
  //   or
  //   - every second

      if (cached_packets_count == MAX_CACHED_DCC_PACKETS || millis() - time_from_last_sent_dcc_packet > 2000){
        //Time to send packets to the web app over websockets
        if (cached_packets_count > 0)
        {
          send_dcc_packets(cached_packets, cached_packets_count);
          // Reset cached_packets_count
          cached_packets_count = 0;
        }
        time_from_last_sent_dcc_packet = millis();
    }

}