#include "SPIFFS.h"
#include "config.h"

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>

#include "src/dcc_reader/NmraDcc.h"
#include "src/network/webserver.h"

NmraDcc Dcc;

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
};

void notifyDccFunc(uint16_t Addr, DCC_ADDR_TYPE AddrType, FN_GROUP FuncGrp, uint8_t FuncState)
{
  Serial.print("notifyDccFunc: Addr: ");
  Serial.print(Addr, DEC);
  Serial.print((AddrType == DCC_ADDR_SHORT) ? 'S' : 'L');
  Serial.print("  Function Group: ");
  Serial.print(FuncGrp, DEC);

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

// This function is called whenever a normal DCC Turnout Packet is received and we're in Output Addressing Mode
void notifyDccAccTurnoutOutput( uint16_t Addr, uint8_t Direction, uint8_t OutputPower )
{
  Serial.print("notifyDccAccTurnoutOutput: ") ;
  Serial.print(Addr,DEC) ;
  Serial.print(',');
  Serial.print(Direction,DEC) ;
  Serial.print(',');
  Serial.println(OutputPower, HEX) ;
}

// This function is called whenever a DCC Signal Aspect Packet is received
void notifyDccSigOutputState( uint16_t Addr, uint8_t State)
{
  Serial.print("notifyDccSigOutputState: ") ;
  Serial.print(Addr,DEC) ;
  Serial.print(',');
  Serial.println(State, HEX) ;
}

void notifyDccMsg(DCC_MSG *Msg)
{
  Serial.print("notifyDccMsg: ");

  String ws_msg = "";
  for (uint8_t i = 0; i < Msg->Size; i++)
  {
    Serial.print(Msg->Data[i], HEX);
    Serial.write(' ');

    ws_msg += String(Msg->Data[i]) + ' ';

  }
  Serial.println();

  notifyClients(Msg->Data, Msg->Size);
}

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
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB
  }
  Serial.print("Initializing DCC reader...");

  Dcc.pin(DCC_PIN, 0);

  Dcc.init(MAN_ID_DIY, 0, 0, 0);
  Serial.print("DCC reader is ready...");

  initSPIFFS();
  setup_webserver();
}

void loop()
{
  loop_webserver();
  Dcc.process();
}