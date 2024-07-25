
#define LED_TYPE LED_STRIP_WS2812
#define LED_TYPE_IS_RGBW 0 // if the LED is an RGBW type, change the 0 to 1
#define LED_GPIO 38        // change this number to be the GPIO pin connected to the LED
#define LED_BRIGHT 30      // sets how bright the LED is. O is off; 255 is burn your eyeballs out (not recommended)

// pick the colour you want from the list here and change it in setup()
static const crgb_t L_RED = 0xff0000;
static const crgb_t L_GREEN = 0x00ff00;
static const crgb_t L_BLUE = 0x0000ff;
static const crgb_t L_WHITE = 0xe0e0e0;

LiteLED myLED(LED_TYPE, LED_TYPE_IS_RGBW); // create the LiteLED object; we're calling it "myLED"

/**
 * Initialize.
 */
void setup_status_led()
{

    myLED.begin(LED_GPIO, 1);      // initialze the myLED object. Here we have 1 LED attached to the LED_GPIO pin
    myLED.brightness(LED_BRIGHT);  // set the LED photon intensity level
    myLED.setPixel(0, L_BLUE, 1); // set the LED colour and show it
    myLED.brightness(0, 1);        // turn the LED on

}

void on_status_led(crgb_t color_in_hex = 0x00ff00)
{
    myLED.setPixel(0, color_in_hex, 1); // set the LED colour and show it
    myLED.brightness(LED_BRIGHT, 1);
}

void off_status_led()
{
    myLED.brightness(0, 1);
}