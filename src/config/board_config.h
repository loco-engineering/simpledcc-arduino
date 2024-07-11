#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

typedef enum { ESP32Sx, LED_DRIVER_PCA9955B } CHIP_TYPE;
typedef enum { NONE, DIGITAL, PWM, ANGLE, AUDIO } SIGNAL_TYPES;

typedef struct
{
    char name[10]; // the name of an output as marked on a board, for example, on Loco.Engineering accessory board - "00", "01", "io08"
    uint8_t output_num; // the number of output of a microchip (for example, GPIO on ESP32 and LEDx on a LED driver), for Loco.Engineering boards numbers in names and output_num are the same
    CHIP_TYPE owner_id; //the id of a chip that has this output_num, see CHIP_TYPE to get list of supported chips
    SIGNAL_TYPES signal_types[5] = {NONE}; // one connection can have up to 5 SIGNAL_TYPES
} Connection;

/* How to add a new board
- Update the connection_amount with amount of connections on your board
- Fill information about connections on your board in get_board_connections
- If you add new CHIP_TYPE you should add a new handler for the new chip
- If you add new SINGNAL_TYPES you should update the web app to show signal types correctly in the web app (the state details screen) and you should add a new handler for the new signal type
*/
const uint8_t connection_amount = 28;
Connection board_connections [connection_amount];

void fill_board_connections(){

    //Fill connections to a LED driver
    for (uint8_t i = 0; i < 16; i++){
        sprintf(board_connections[i].name, "%02d", i);
        board_connections[i].output_num = i;
        board_connections[i].owner_id = LED_DRIVER_PCA9955B;
        board_connections[i].signal_types[0] = PWM;
    }

}

#endif
