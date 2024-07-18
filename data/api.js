//var gateway = `ws://${window.location.hostname}/ws`;
var gateway = `ws://192.168.4.1/ws`;

var websocket;
export var available_outputs = [];

export function generate_wcc_message(module_settings) {

    var wcc_msg = [];
    let wcc_msg_ind = 0;
    wcc_msg[wcc_msg_ind++] = 1; //message type, 1 - WCC message

    //Add WCC protocol version
    const protocol_version = bytesArray(110, 2);
    wcc_msg[wcc_msg_ind++] = protocol_version[0];
    wcc_msg[wcc_msg_ind++] = protocol_version[1];


    //Add project version
    const project_version = bytesArray(329, 3);
    wcc_msg[wcc_msg_ind++] = project_version[0];
    wcc_msg[wcc_msg_ind++] = project_version[1];
    wcc_msg[wcc_msg_ind++] = project_version[2];

    //Add project version
    const decoder_amount = bytesArray(1, 3);
    wcc_msg[wcc_msg_ind++] = decoder_amount[0];
    wcc_msg[wcc_msg_ind++] = decoder_amount[1];
    wcc_msg[wcc_msg_ind++] = decoder_amount[2];

    //NOTE: Now we send information about only 1 decoder because this web app allows to setup just one decoder

    //Add Mac address of a decoder
    wcc_msg[wcc_msg_ind++] = 0xdc;
    wcc_msg[wcc_msg_ind++] = 0x54;
    wcc_msg[wcc_msg_ind++] = 0x75;
    wcc_msg[wcc_msg_ind++] = 0xd8;
    wcc_msg[wcc_msg_ind++] = 0xf8;
    wcc_msg[wcc_msg_ind++] = 0x30;


    //Add the message length for the decoder with mac address we just parsed above
    //Note: we don't use it until we have settings for only one decoder inside this WCC message
    //That's why we send just random number
    const decoder_msg_length = bytesArray(858, 4);
    wcc_msg[wcc_msg_ind++] = decoder_msg_length[0];
    wcc_msg[wcc_msg_ind++] = decoder_msg_length[1];
    wcc_msg[wcc_msg_ind++] = decoder_msg_length[2];
    wcc_msg[wcc_msg_ind++] = decoder_msg_length[3];

    //Get state amount for this decoder
    const state_amount = bytesArray(1, 2);
    wcc_msg[wcc_msg_ind++] = state_amount[0];
    wcc_msg[wcc_msg_ind++] = state_amount[1];

    //Fill states
    //Each state has 
    //- id, unique inside a decoder
    //- output_id (we set relations between output_id and physical outputs in board_config.h file)
    //- value to send to output_id
    //- status - is active this state or not

    module_settings.states.forEach(state => {
        //Set state's id
        for (var id_ind = 0; id_ind < state.id.length; id_ind += 1) {
            wcc_msg[wcc_msg_ind++] = state.id.charCodeAt(id_ind);
        }

        //Set value amount
        wcc_msg[wcc_msg_ind++] = state.values.length;

        state.values.forEach(value => {

            //Set state's connection_id, 1 byte
            wcc_msg[wcc_msg_ind++] = value.connection_id;

            //Set state's value length in bytes, 2 bytes
            const value_length = bytesArray(1, 2);
            wcc_msg[wcc_msg_ind++] = value_length[0];
            wcc_msg[wcc_msg_ind++] = value_length[1];

            //Set state's value
            wcc_msg[wcc_msg_ind++] = value.value;

            /*for (var val_ind = 0; val_ind < value_length; val_ind += 1) {
                wcc_msg[wcc_msg_ind++] = value_data[val_ind];
            }*/

        });

        wcc_msg[wcc_msg_ind++] = state.is_active;


        //Set wcc message ids
        //each message id takes 6 bytes because should be unique inside the layout 
        const wcc_event_ids_amount = state.wcc_event_ids.length
        wcc_msg[wcc_msg_ind++] = wcc_event_ids_amount;

        state.wcc_event_ids.forEach(wcc_message_id => {
            for (var id_ind = 0; id_ind < 6; id_ind += 1) {
                wcc_msg[wcc_msg_ind++] = wcc_message_id.charCodeAt(id_ind);
            }

            //Set if the state should be activated or not
            wcc_msg[wcc_msg_ind++] = 1;
        });


    });

    //Add handlers
    //Now we don't send handlers
    //Set handler ammount, 2 bytes
    wcc_msg[wcc_msg_ind++] = 0;
    wcc_msg[wcc_msg_ind++] = 0;

    //Add events
    //Now we don't send events
    //Set event ammount, 2 bytes
    wcc_msg[wcc_msg_ind++] = 0;
    wcc_msg[wcc_msg_ind++] = 0;

    return wcc_msg;

}

export function generate_wcc_event_msg(wcc_event) {

    var wcc_msg = [];
    let wcc_msg_ind = 0;
    wcc_msg[wcc_msg_ind++] = 2; //message type, 2 - WCC event

    for (var id_ind = 0; id_ind < 6; id_ind += 1) {
        wcc_msg[wcc_msg_ind++] = wcc_event.id.charCodeAt(id_ind);
    }

    //Set if the state should be activated or not
    wcc_msg[wcc_msg_ind++] = wcc_event.is_state_active;

    return wcc_msg;

}

const bytesArray = (n, required_length) => {
    const a = []
    a.unshift(n & 255)
    while (n >= 256) {
        n = n >>> 8
        a.unshift(n & 255)
    }
    var bytes_arr = new Uint8Array(a);
    if (required_length != undefined && bytes_arr.length < required_length) {
        bytes_arr = concatTypedArrays(Uint8Array.from(new Array(required_length - bytes_arr.length).fill(0)), bytes_arr);
    }
    return bytes_arr;
}

function concatTypedArrays(a, b) { // a, b TypedArray of same type
    var c = new (a.constructor)(a.length + b.length);
    c.set(a, 0);
    c.set(b, a.length);
    return c;
}

export function send_wcc_message(module_settings) {
    var message_to_send = generate_wcc_message(module_settings);
    send_websocket_message(message_to_send);
}

export function send_wcc_event(wcc_event) {
    var message_to_send = generate_wcc_event_msg(wcc_event);
    send_websocket_message(message_to_send);
}

export function send_websocket_message(message_to_send) {
    if (websocket.readyState == WebSocket.OPEN) {
        let msg = new Uint8Array(message_to_send);
        console.log(msg);
        websocket.send(msg);
    }
}

export function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}
function onOpen(event) {
    console.log('Connection opened');
    var wcc_test_event = {};
    wcc_test_event.id = 'eq125k';
    wcc_test_event.is_state_active = 1;
    send_wcc_event(wcc_test_event);
}

function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}
async function onMessage(event) {

    const buffer = new Uint8Array(await event.data.arrayBuffer());
    console.log("New message. Length: " + buffer.length);

    console.log(Array.apply([], buffer).join(","));

    //Parse the message from a decoder
    //Get a type
    var msg_index = 0;
    let msg_type = buffer[msg_index++];

    if (msg_type == 4) {
        //This a message with DCC packets
        //Get the amount of DCC packets
        let dcc_packets_amount = buffer[msg_index++];
        for (var i = 0; i < dcc_packets_amount; i++) {
            let packet_type = buffer[msg_index++];
            var packet_type_str = "Unknown";
            var description_str = "";

            let adr_1 = buffer[msg_index++];
            let adr_2 = buffer[msg_index++];

            let address = (((adr_2 & 0xFF) << 8) | (adr_1 & 0xFF));
            let raw_packet_length = buffer[msg_index++];

            var raw_packet = [];
            var raw_packet_str = "";

            for (var k = 0; k < raw_packet_length; k++) {
                raw_packet[k] = buffer[msg_index++];
                raw_packet_str += toBinString(Uint8Array.from([raw_packet[k]])) + " ";
            }

            let user_data_length = buffer[msg_index++];

            var user_data = [];
            for (var k = 0; k < user_data_length; k++) {
                user_data[k] = buffer[msg_index++];

            }

            switch (packet_type) {
                case 0:
                    description_str = "No description available";
                    address = "-";
                    break;
                case 1:
                    packet_type_str = "Speed";
                    description_str = `Speed: ${user_data[0]} | Steps: ${user_data[2]} | Dir: ${user_data[1] ? 'Forward' : 'Reverse'}`
                    break;
                case 2:
                    packet_type_str = "Function";
                    //Get the function group
                    var function_group_str = "0 - 4";
                    switch (user_data[0]) {
                        case 2:
                            function_group_str = "5 - 8";
                            break;
                        case 3:
                            function_group_str = "9 - 12";
                            break;
                        case 4:
                            function_group_str = "13 - 20";
                            break;
                        case 5:
                            function_group_str = "21 - 28";
                            break;
                        default:
                            break;
                    }
                    description_str = `Function Group: ${function_group_str} | Steps: ${user_data[2]} | Dir: ${user_data[1] ? 'Forward' : 'Reverse'}`
                    break;
                case 3:
                        packet_type_str = "Signal Aspect";
                        description_str = `Aspect: ${user_data[0]}`;
                        break;
                case 4:
                        packet_type_str = "Turnout";
                        description_str = `Direction: ${user_data[0]} | Power: ${user_data[1]}`;
                        if (user_data_length == 3){
                            description_str += ` | Output Pair: ${user_data[2]}`;
                        }
                        break;
                default:
                    break;
            }

            //Add a cell to the DCC packets list
            var tr_node = document.createElement('tr');
            tr_node.classList.add("service_cell");
            tr_node.innerHTML = `<td>${address}</td><td>${packet_type_str}</td><td>${description_str}</td><td>${raw_packet_str}</td><td><button id="table_btn" class="table_btn">Add action</button></td>`;

            document.querySelector('#dcc_packets_tbody').appendChild(tr_node);

        }

        document.querySelector('#no_dcc_packets_hint').style.display = "none";

    }


    if (msg_type == 5) {
        //This a message with a board config
        //Get the amount board connections
        let connections_amount = buffer[msg_index++];


        available_outputs = [];

        const CONNECTION_NAME_LENGTH = 4; // If you change this value you should update it in the web app - search for CONNECTION_NAME_LENGTH in js files
        const CONNECTION_SIGNAL_TYPES_AMOUNT = 5; // If you change this value you should update it in the web app - search for CONNECTION_SIGNAL_TYPES_AMOUNT in js files
        const connection_types = ["Digital", "PWM"];

        for (var i = 0; i < connections_amount; i++) {

            var connection = {};
            connection.name = '';
            //Get connection name
            for (var name_ind = 0; name_ind < CONNECTION_NAME_LENGTH; name_ind++) {
                connection.name += String.fromCharCode(buffer[msg_index++]);
            }

            connection.output_num = buffer[msg_index++];
            connection.owner_id = buffer[msg_index++];
            connection.signal_types = [];

            for (var type_ind = 0; type_ind < CONNECTION_SIGNAL_TYPES_AMOUNT; type_ind++) {
                const type = buffer[msg_index++];
                if (type != 0) {
                    connection.signal_types.push(connection_types[type - 1]);
                }
            }

            if (connection.name != '\u0000\u0000\u0000\u0000') {
                available_outputs.push(connection);
            }

        }

        console.log("Board connections: " + JSON.stringify(available_outputs));
    }



    //Autoscoll
    //let dcc_packets_table_el = document.querySelector('#dcc_packets_table_container');
    //dcc_packets_table_el.scrollTop = dcc_packets_table_el.scrollHeight;

}

const toBinString = (bytes) =>
    bytes.reduce((str, byte) => str + byte.toString(2).padStart(8, '0'), '');

