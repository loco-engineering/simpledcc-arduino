import { send_wcc_message, send_wcc_project_file } from "./api.js";
import { reload_states } from "./app.js";

var current_decoder_id = null;

var project_settings = null;
project_settings = {};
project_settings.states = [];

export function get_project_settings(){
    return project_settings;
}

export function set_decoder_id(decoder_id){
    current_decoder_id = decoder_id;
}

export function clear_project(){
    localStorage.removeItem(current_decoder_id);
}

export function set_local_project(project){

    project_settings = JSON.parse(project);
    console.log("Loaded a project:" + JSON.stringify(project_settings));

    reload_states();
}

export function save_local_project(){
    localStorage.setItem(current_decoder_id, JSON.stringify(project_settings));

    //Resend WCC message with project updates
    //var test_wcc_msg = [ /*first byte - message type - WCC*/0x01, 0x00, 0x01, 0x00, 0x00, 0x02, 0x00, 0x00, 0x01, 0xdc, 0x54, 0x75, 0xd8, 0xf8, 0x30, 0x00, 0x00, 0x00, 0x07, 0x01, 0x01, 0x04, 0x06,/*states start*/ 0x00, 0x01, 0x01, 0x00, 0x00, 0x01, 0x08, 0x01 /*States End*/, 0x00, 0x00, 0x00, 0x00];
    send_wcc_message(project_settings);

    send_wcc_project_file(project_settings);
}

export function add_state(state) {
    state.id = generate_id();
    project_settings.states.push(state);
    save_local_project();
    return state
}

export function delete_state(state_id) {
    project_settings.states.forEach((state, index) => {
        if (state.id == state_id) {
            project_settings.states.splice(index, 1);
            save_local_project();
            return;
        }
    });
}

export function update_state(state) {
    save_local_project();
}

function generate_id() {
    var firstPart = (Math.random() * 46656) | 0;
    var secondPart = (Math.random() * 46656) | 0;
    firstPart = ("000" + firstPart.toString(36)).slice(-3);
    secondPart = ("000" + secondPart.toString(36)).slice(-3);
    return firstPart + secondPart;
}