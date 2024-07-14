#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

typedef enum
{
    ESP32Sx,
    LED_DRIVER_PCA9955B
} CHIP_TYPE;
typedef enum
{
    NONE,
    DIGITAL,
    PWM,
    ANGLE,
    AUDIO
} SIGNAL_TYPES;

const uint8_t CONNECTION_NAME_LENGTH = 4; // If you change this value you should update it in the web app - search for CONNECTION_NAME_LENGTH in js files
const uint8_t CONNECTION_SIGNAL_TYPES_AMOUNT = 5; // If you change this value you should update it in the web app - search for CONNECTION_SIGNAL_TYPES_AMOUNT in js files

typedef struct
{
    char name[CONNECTION_NAME_LENGTH];                         // the name of an output as marked on a board, for example, on Loco.Engineering accessory board - "00", "01", "io08"
    uint8_t output_num;                    // the number of output of a microchip (for example, GPIO on ESP32 and LEDx on a LED driver), for Loco.Engineering boards numbers in names and output_num are the same
    CHIP_TYPE owner_id;                    // the id of a chip that has this output_num, see CHIP_TYPE to get list of supported chips
    SIGNAL_TYPES signal_types[CONNECTION_SIGNAL_TYPES_AMOUNT] = {NONE}; // one connection can have up to 5 SIGNAL_TYPES
} Connection;

/* How to add a new board
- Update the connection_amount with amount of connections on your board
- Fill information about connections on your board in fill_board_connections
- If you add a new CHIP_TYPE you should add a new handler for the new chip
- If you add a new SIGNAL_TYPES you should update the web app to show signal types correctly in the web app (the "State details" screen) and you should add a new handler for the new signal type
*/
const uint8_t connection_amount = 28;
Connection board_connections[connection_amount];

void fill_board_connections()
{

    const uint8_t LED_outputs_amount = 16;
    // Fill connections to a LED driver
    for (uint8_t i = 0; i < LED_outputs_amount; i++)
    {
        sprintf(board_connections[i].name, "%02d", i);
        board_connections[i].output_num = i;
        board_connections[i].owner_id = LED_DRIVER_PCA9955B;
        board_connections[i].signal_types[0] = PWM;
    }

    // Fill GPIO outputs
    sprintf(board_connections[LED_outputs_amount].name, "io04");
    board_connections[LED_outputs_amount].output_num = 4;
    board_connections[LED_outputs_amount].owner_id = ESP32Sx;
    board_connections[LED_outputs_amount].signal_types[0] = DIGITAL;
    board_connections[LED_outputs_amount].signal_types[1] = PWM;

    sprintf(board_connections[LED_outputs_amount + 1].name, "io05");
    board_connections[LED_outputs_amount + 1].output_num = 5;
    board_connections[LED_outputs_amount + 1].owner_id = ESP32Sx;
    board_connections[LED_outputs_amount + 1].signal_types[0] = DIGITAL;
    board_connections[LED_outputs_amount + 1].signal_types[1] = PWM;

    sprintf(board_connections[LED_outputs_amount + 2].name, "io06");
    board_connections[LED_outputs_amount + 2].output_num = 6;
    board_connections[LED_outputs_amount + 2].owner_id = ESP32Sx;
    board_connections[LED_outputs_amount + 2].signal_types[0] = DIGITAL;
    board_connections[LED_outputs_amount + 2].signal_types[1] = PWM;

}

#endif
