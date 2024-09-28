import {get_project_settings} from "./storage.js"
import {show_edit_action } from "./state-details-modal.js";

export function reload_states() {
    let project = get_project_settings();

    document.querySelector('#states_list_tbody').innerHTML = '';

    for (var i = 0; i < project.states.length; i++) {
        var state = project.states[i];

        //Add a cell to the DCC packets list
        var tr_node = document.createElement('tr');
        tr_node.classList.add("service_cell");
        tr_node.classList.add("state_cell");

        tr_node.dataset.id = state.id;

        tr_node.innerHTML = `<td ><div class="state_icn"></div></td><td>${state.name}</td><td></td><td></td><td class="delete_state_btn" data-id="${state.id}" style="text-align:center;">
            <button class="delete_btn"><img class="delete_btn_icn" src="./delete.png"></button>
            </td>`;

        document.querySelector('#states_list_tbody').appendChild(tr_node);
    }

    if (project.states.length > 0) {
        document.querySelector('#no_states_hint').style.display = "none";
    } else {
        document.querySelector('#no_states_hint').style.display = "block";
    }

    document.querySelectorAll('.state_cell').forEach(cell => {
        cell.addEventListener('click', function handleClick(event) {
            event.preventDefault();

            var selected_state = null;
            //Find a selected product
            project.states.forEach((state) => {
                if (state.id == this.dataset.id) {
                    show_edit_action(reload_states, state);
                }
            });

        });
    });

    document.querySelectorAll('.delete_state_btn').forEach(cell => {
        cell.addEventListener('click', function handleClick(event) {
            event.preventDefault();
            event.stopPropagation();

            var selected_state = null;
            //Find a selected product
            let project = get_project_settings();

            project.states.forEach((state, index) => {
                if (state.id == this.dataset.id) {

                    let text = `State "${state.name}" will be deleted permanently. Are you sure?`;
                    if (confirm(text) == true) {
                        delete_state(this.dataset.id);
                        reload_states();
                    } else {

                    }


                }
            });

        });
    });

}