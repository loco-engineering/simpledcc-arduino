import { add_state, update_state } from "./storage.js";
import {available_outputs} from "./api.js"
var close_callback = null;

class ActionModal extends HTMLElement {
    constructor() {
        super();

        this.innerHTML = /*html*/`
    <div id="action-modal-container" class="modal">
        <div class="modal-content">
            <div class="modal-content-container">  

            <span class="action_modal_title">State Settings</span>
            <hr>
            <div class="state_settings_container">

    <div class="state_text_input">
     <label class="state_input_text_label"for="state_name"> Name (optional)</label>
    <input type="text" placeholder="" name="state_name" id="state_name">
    </div>

    <div class="state_text_input">
    <label class="state_input_text_label" for="state_wcc_event"> WCC Message IDs (optional)</label>
    <input type="text" placeholder="WCC message IDs separated by comma" name="state_wcc_event" id="state_wcc_event">
    </div>

    <div class="state_text_input">
    <label class="state_input_text_label" for="state_dcc_packet"> DCC Packet (optional)</label>
    <input type="text" placeholder="" name="state_dcc_packet" id="state_dcc_packet">
    </div>


            <input class ="state_check_box" type="checkbox" id="state_active" name="state_active">
            <label class="state_input_text_label" id="state_is_active" for="state_active"> Active</label><br>
            
            </div>
            <div class="pure-g ">

            <div class="pure-u-1-2 ">
            <span class="action_modal_title">Outputs & values</span>
            </div>

            <div class="pure-u-1-2">
            <button style="margin-top:20px; margin-left:4px;" id="new_output_value_btn" class="new_btn new_action_btn">Add value</button>
            </div>

            </div>
            <hr>

            <div class="output_values_table">

            <div id="output_list">
            </div>
        </div>

                <div style="display: flex; width:300px; margin: auto; padding-top:10px;">
                    <button id="save_action_settings" class="s_wifi">Save</button>
                    <button id="close_action_settings" class="s_wifi">Cancel</button>
                </div>

            </div>
        </div>
    </div>
        `;

        document.getElementById("close_action_settings").onclick = function () {
            document.getElementById("action-modal-container").style.display = "none";
        }

        document.getElementById("state_is_active").onclick = function () {
            console.log("clicked on is active");
        }

        document.getElementById("save_action_settings").onclick = () => {

            save_outputs();
            
            current_state.name = document.getElementById("state_name").value;
            current_state.is_active = document.getElementById("state_is_active").value;

            var wcc_event_ids_str = document.getElementById("state_wcc_event").value;
            if (wcc_event_ids_str != undefined){
                current_state.wcc_event_ids = wcc_event_ids_str.split(",");
            }

            if (current_state.name == ''){
                if (current_state.values.length > 0){
                    var name = '';
                    current_state.values.forEach(value => {
                        if (value.output != undefined){
                            name += `${value.output} `;
                        }
                    });
                    current_state.name = name;
                }else{
                    current_state.name = `-`;
                }
            }
            if (current_state.id != undefined){
                update_state(current_state);
            }else{
                add_state(current_state);
            }

            close_callback();

            document.getElementById("action-modal-container").style.display = "none";

        };
    }
}

export function reload_action_outputs() {
    //Add a cell to the action outputs list
    document.querySelector('#output_list').innerHTML = "";

    for (let value_index = 0; value_index < current_state.values.length; ++value_index) {

        var output_select = `<select class="state_output" id="state_output_${value_index}">`;
        output_select += `<option value="0">Not selected</option>`;

        for (let index = 0; index < available_outputs.length; ++index) {
            const output = available_outputs[index];
            output_select += `<option value="${index}">${output.name}</option>`;
        }
        output_select += `</select>`;

        var type_select = `<select class="type_output" id="type_output_${value_index}">`;
        type_select += `<option value="0">Not selected</option>`;

        for (let index = 0; index < 1; ++index) {
            type_select += `<option value="gpio">GPIO</option>`;
        }
        type_select += `</select>`;

        /*var tr_node = document.createElement('tr');
        tr_node.classList.add("service_cell");
        tr_node.innerHTML = `<td height="40">${output_select}</td><td height="40">${type_select}</td><input type="text" id="state_value_${value_index}" placeholder="0 - 255"></td>`;
        document.querySelector('#action_outputs_tbody').appendChild(tr_node);*/

        var grid_node = document.createElement('div');
        grid_node.classList.add("pure-g");
        if (value_index % 2 == 0){
            grid_node.classList.add("state_outputs_list_gray");
        }else{
            grid_node.classList.add("state_outputs_list_gray");
        }

        grid_node.innerHTML = `
    
        <div class="pure-u-1-2 "><div class="state_input_text_label state_settings_adv_params">Name on a decoder</div></div>
        <div class="pure-u-1-2 "><div class="output_cell">${output_select}</div></div>

        <div class="pure-u-1-2 "><div class="state_input_text_label state_settings_adv_params">Type</div></div>
        <div class="pure-u-1-2 "><div class="output_cell">${type_select}</div></div>

        <div class="pure-u-1-2 "><div class="state_input_text_label state_settings_adv_params">Value</div></div>
        <div class="pure-u-1-2 "><div class="output_cell">
            <input class="state_settings_input" type="text" id="state_value_${value_index}" value="0">
        </div>
        </div>

        <div class="pure-u-1-2 "><div class="state_input_text_label state_settings_adv_params">Action</div></div>
        <div class="pure-u-1-2 "><div class="output_cell">
        <select class="type_output" id="state_output_action_${value_index}">
        <option value="0">None</option>
        <option value="1">Blink</option>
        </select>
        </div></div>

        <div class="pure-g">

        <div class="pure-u-1-2 "><div class="state_input_text_label state_settings_adv_params">Start delay</div></div>
        <div class="pure-u-1-2 "><div class="output_cell"><input class="state_settings_input" type="text" id="state_value_start_delay${value_index}" placeholder="in ms"></div></div>

        <div class="pure-u-1-2 "><div class="state_input_text_label state_settings_adv_params">"On" duration</div></div>
        <div class="pure-u-1-2 "><div class="output_cell"><input class="state_settings_input" type="text" id="state_value_start_delay${value_index}" placeholder="in ms"></div></div>

        <div class="pure-u-1-2 "><div class="state_input_text_label state_settings_adv_params">"Off" duration</div></div>
        <div class="pure-u-1-2 "><div class="output_cell"><input class="state_settings_input" type="text" id="state_value_start_delay${value_index}" placeholder="in ms"></div></div>
        
        <div class="pure-u-1-2 "></div>
        <div class="pure-u-1-2 "><div class="output_cell state_value_cell_btns">
        <button class="delete_btn delete_output" data-id="${value_index}"><img class="delete_btn_icn" src="/delete.png"></button>

        </div></div>

        </div>
        `;

        document.querySelector('#output_list').appendChild(grid_node);


        if (current_state.values[value_index].connection_name != undefined){
            document.getElementById(`state_output_${value_index}`).value = current_state.values[value_index].connection_id;
            document.getElementById(`state_value_${value_index}`).value = current_state.values[value_index].value;

        }

    }

    const cells = document.querySelectorAll('.delete_output');
    cells.forEach(cell => {
        cell.addEventListener('click', function handleClick(event) {
            event.preventDefault();

            current_state.values.splice(this.dataset.id, 1);
            reload_action_outputs();

        });
    });

    /*var tr_node = document.createElement('tr');
    tr_node.classList.add("service_cell");
    tr_node.innerHTML = `<td class="add_output_value_cell"><button id="new_output_value_btn" class="new_btn new_action_btn">Add value</button></td><td  class="add_output_value_cell"></td>`;
    document.querySelector('#action_outputs_tbody').appendChild(tr_node);*/

    document.getElementById("new_output_value_btn").onclick = () => {

        save_outputs();

        current_state.values.push({});
        reload_action_outputs();
    };

}


var current_state = null;

export function save_outputs(){
    current_state.values = [];

    var state_output_els = document.getElementsByClassName('state_output');
    for (var i = 0; i < state_output_els.length; ++i) {
        const state_connection_id = parseInt(document.getElementById(`state_output_${i}`).value);
        const state_connection_name = available_outputs[state_connection_id];
        const state_value_val = document.getElementById(`state_value_${i}`).value;

        current_state.values.push({connection_name:  state_connection_name, connection_id: state_connection_id, signal_type: 0, value: parseInt(state_value_val)});

    }
}

export function show_new_action(reload_states) {

    current_state = {};
    current_state.values = [{}];

    reload_action_outputs();
    document.getElementById("action-modal-container").style.display = "block";

    close_callback = reload_states;
}

export function show_edit_action(reload_states, state) {

    current_state = state;

    reload_action_outputs();

    document.getElementById("state_name").value = current_state.name;
    document.getElementById("state_is_active").value = current_state.is_active;

    document.getElementById("action-modal-container").style.display = "block";

    close_callback = reload_states;
}

customElements.define("action-modal", ActionModal);