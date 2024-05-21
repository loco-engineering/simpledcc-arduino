#define SS_PIN 2  // ESP32 pin GPIO5
#define RST_PIN 4 // ESP32 pin GPIO27

MFRC522DriverPinSimple ss_pin(SS_PIN); // Configurable, see typical pin layout above.

MFRC522DriverSPI driver{ss_pin}; // Create SPI driver.
// MFRC522DriverI2C driver{}; // Create I2C driver.
MFRC522 mfrc522{driver}; // Create MFRC522 instance.

void setup_nfc()
{
    Serial.begin(115200);
    while (!Serial)
        ;                  // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4).
    SPI.begin(7, 5, 6, 2); // init SPI bus

    mfrc522.PCD_Init();                                     // Init MFRC522 board.
    MFRC522Debug::PCD_DumpVersionToSerial(mfrc522, Serial); // Show details of PCD - MFRC522 Card Reader details.
    Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));
}

void loop_nfc()
{
    // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
    if (!mfrc522.PICC_IsNewCardPresent())
    {
        return;
    }

    // Select one of the cards.
    if (!mfrc522.PICC_ReadCardSerial())
    {
        return;
    }

    // Dump debug info about the card; PICC_HaltA() is automatically called.
    MFRC522Debug::PICC_DumpToSerial(mfrc522, Serial, &(mfrc522.uid));
}