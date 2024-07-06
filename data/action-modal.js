import { set_decoder_id, load_local_project, add_state, get_project_settings } from "./storage.js";

class ActionModal extends HTMLElement {
    constructor() {
        super();

        this.innerHTML = /*html*/`
    <div id="action-modal-container" class="modal">
        <div class="modal-content">
            <div class="modal-content-container">  

            <span class="action_modal_title">Settings</span>
            <hr>
            <div class="state_settings_container">

    <div class="state_text_input">
     <label class="state_input_text_label"for="state_name"> Name (optional)</label>
    <input type="text" placeholder="" name="state_name">
    </div>

    <div class="state_text_input">
    <label class="state_input_text_label" for="state_name"> DCC Packet (optional)</label>
    <input type="text" placeholder="" name="state_name">
    </div>

    <div class="state_text_input">
    <label class="state_input_text_label" for="state_name"> WCC Event (optional)</label>
    <input type="text" placeholder="" name="state_name">
    </div>


            <input class ="state_check_box" type="checkbox" id="state_active" name="state_active" value="Bike">
            <label class="state_input_text_label" for="state_active"> Active</label><br>
            
            </div>

            <span class="action_modal_title">Outputs & values</span>
            <hr>

            <div class="tableFixHead output_values_table">

            <div id="output_list">
            </div>
        </div>
        <button id="new_output_value_btn" class="new_btn new_action_btn">Add value</button></td><td  class="add_output_value_cell">

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

        document.getElementById("save_action_settings").onclick = () => {

            save_outputs();

            //Iterate over all outputs and values
            var new_state = {};
            new_state.values = action_output_values;
            add_state(new_state);
            document.getElementById("action-modal-container").style.display = "none";

        };
    }
}

export function reload_action_outputs() {
    //Add a cell to the action outputs list
    document.querySelector('#output_list').innerHTML = "";

    for (let value_index = 0; value_index < action_output_values.length; ++value_index) {

        var output_select = `<select class="state_output" id="state_output_${value_index}">`;

        for (let index = 0; index < available_outputs.length; ++index) {
            const output = available_outputs[index];
            output_select += `<option value="${output.name}">${output.name}</option>`;
        }
        output_select += `</select>`;

        var type_select = `<select class="type_output" id="type_output_${value_index}">`;

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
        <div class="pure-u-1-3 "><div class="output_cell">${output_select}</div></div>
        <div class="pure-u-1-3 "><div class="output_cell">${type_select}</div></div>
        <div class="pure-u-1-3 "><div class="output_cell"><input type="text" id="state_value_${value_index}" placeholder="0 - 255"></div></div>
        
        <div class="pure-u-1-3 "><div class="output_cell">Hey</div></div>
        <div class="pure-u-1-3 "><div class="output_cell">Brrr</div></div>
        <div class="pure-u-1-3 "><div class="output_cell">UUUU</div></div>

        `;

        document.querySelector('#output_list').appendChild(grid_node);


        if (action_output_values[value_index].output != undefined){
            document.getElementById(`state_output_${value_index}`).value = action_output_values[value_index].output;
            document.getElementById(`state_value_${value_index}`).value = action_output_values[value_index].value;

        }

    }

    /*var tr_node = document.createElement('tr');
    tr_node.classList.add("service_cell");
    tr_node.innerHTML = `<td class="add_output_value_cell"><button id="new_output_value_btn" class="new_btn new_action_btn">Add value</button></td><td  class="add_output_value_cell"></td>`;
    document.querySelector('#action_outputs_tbody').appendChild(tr_node);*/

    document.getElementById("new_output_value_btn").onclick = () => {

        save_outputs();

        action_output_values.push({});
        reload_action_outputs();
    };

}

export function save_outputs(){
    action_output_values = [];

    var state_output_els = document.getElementsByClassName('state_output');
    for (var i = 0; i < state_output_els.length; ++i) {
        const state_output_val = document.getElementById(`state_output_${i}`).value;
        const state_value_val = document.getElementById(`state_value_${i}`).value;

        action_output_values.push({output: state_output_val, type: 1, value: state_value_val});

    }
}

var available_outputs = [];
var action_output_values = [{}];

export function show_new_action() {

    available_outputs = [];

    available_outputs.push({name: "IO1", type:"IO"});
    available_outputs.push({name: "IO2", type:"IO"});
    available_outputs.push({name: "LED1", type:"LED"});
    available_outputs.push({name: "LED2", type:"LED"});
    available_outputs.push({name: "LED3", type:"LED"});
    available_outputs.push({name: "Speaker", type:"AUDIO"});

    reload_action_outputs();
    document.getElementById("action-modal-container").style.display = "block";
}

customElements.define("action-modal", ActionModal);