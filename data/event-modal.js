import { add_state, update_state } from "./storage.js";
import { available_outputs } from "./api.js"

class EventModal extends HTMLElement {
    constructor() {
        super();

        this.innerHTML = /*html*/`
    <div id="event-modal-container" class="modal">
        <div class="modal-content">
            <div class="modal-content-container">  

            <span class="action_modal_title">Send Event</span>
            <hr>
            <div class="state_settings_container">

            </div>

            </div>
        </div>
    </div>
        `;

        document.getElementById("close_event_modal").onclick = function () {
            document.getElementById("event-modal-container").style.display = "none";
        }

        document.getElementById("send_event").onclick = () => {

        };
    }
}

export function show_event_modal() {

    document.getElementById("event-modal-container").style.display = "block";

}

customElements.define("event-modal", EventModal);