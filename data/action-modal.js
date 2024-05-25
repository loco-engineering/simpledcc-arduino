
class ActionModal extends HTMLElement {
    constructor() {
        super();
        
        this.innerHTML = /*html*/`
    
    <div id="action-modal-container" class="modal">
        <div class="modal-content">
            <div class="modal-content-container">  
            </div>
        </div>
    </div>
        `;


        document.getElementById("action-modal-container_close_btn").onclick = function () {
            document.getElementById("action-modal-container").style.display = "none";
        }

        document.getElementById("add_element_btn").onclick = () => {          
        };
    }
}

export function show_new_action() {
    document.getElementById("action-modal-container").style.display = "block";
}

customElements.define("action-modal", ActionModal);