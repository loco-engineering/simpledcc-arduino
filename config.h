//This file includes configuration values for your WCC/SimpleDCC decoder

// GPIO that receives DCC Signal 
#define MAX_CACHED_DCC_PACKETS 10

//LEDs
#define MAX_LED_CONNECTIONS 30

//GPIO
#define MAX_GPIO_CONNECTIONS 30

//Logs
#define DEBUG 1

#if DEBUG==1
#define serial_print(x); Serial.print(x);
#define serial_print_format(x, y); Serial.print(x, y);
#define serial_println(x); Serial.println(x);
#define serial_println_format(x, y); Serial.println(x, y);
#else
#define serial_print(x); 
#define serial_print_format(x, y); 
#define serial_println(x); 
#define serial_println_format(x, y);
#endif

