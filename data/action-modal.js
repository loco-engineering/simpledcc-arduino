
class ActionModal extends HTMLElement {
    constructor() {
        super();
        
        this.innerHTML = /*html*/`
    <div id="action-modal-container" class="modal">
        <div class="modal-content">
            <div class="modal-content-container">  

            <span class="action_modal_title">Active</span>
            <label class="switch">
                <input type="checkbox" checked>
                <span class="slider round"></span>
            </label>
            
            <span class="action_modal_title">Enabled</span>
            <label class="switch">
                <input type="checkbox" checked>
                <span class="slider round"></span>
            </label>
            
            <span class="action_modal_title">Outputs & values</span>

            <div class="tableFixHead output_values_table">

            <table id="dcc_packets" class="common_table">
                <thead>
                    <tr>
                        <th width="100">OUTPUT</th>
                        <th width="100">VALUE</th>
                    </tr>
                </thead>
                <tbody id="action_outputs_tbody">
                </tbody>

            </table>
        </div>

                <div style="display: flex; width:300px; margin: auto;">
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
        };
    }
}

export function reload_action_outputs(){
    //Add a cell to the action outputs list
    document.querySelector('#action_outputs_tbody').innerHTML = "";

for (let value_index = 0; value_index < action_output_values.length; ++value_index){

    var output_select = `<select id="action_output">`;

    for (let index = 0; index < available_outputs.length; ++index) {
        const output = available_outputs[index];
        output_select += `<option value="${index}">${output.name}</option>`;
    }
    output_select += `</select>`;

    var tr_node = document.createElement('tr');
    tr_node.classList.add("service_cell");
    tr_node.innerHTML = `<td>${output_select}</td><td><input type="text" placeholder="Output value"></td>`;
    document.querySelector('#action_outputs_tbody').appendChild(tr_node);

}

var tr_node = document.createElement('tr');
tr_node.classList.add("service_cell");
tr_node.innerHTML = `<td class="add_output_value_cell"><button id="new_output_value_btn" class="new_btn new_action_btn">Add value</button></td><td  class="add_output_value_cell"></td>`;
document.querySelector('#action_outputs_tbody').appendChild(tr_node);

}

var available_outputs = [];
var action_output_values = [{}];

export function show_new_action(outputs) {
    available_outputs = outputs;
    reload_action_outputs();
    document.getElementById("action-modal-container").style.display = "block";
}

customElements.define("action-modal", ActionModal);