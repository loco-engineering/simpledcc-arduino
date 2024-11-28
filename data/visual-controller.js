import { add_state, update_state } from "./storage.js";
import { available_outputs } from "./api.js"

class VisualController extends HTMLElement {

    constructor() {

        super();

        this.innerHTML = /*html*/`   
        <div id="train_controller_container"></div>     
        `;

    }

}

var app = null;

export async function start_controller() {

    document.querySelector('#train_controller_container').remove();

    let train_controller_width = 350;
    let train_controller_height = 550;

    let editor_el = document.querySelector('#visual-controller');
    if (window.innerWidth < 500) {
        editor_el.style.backgroundColor = '#ffffff';
    } else {
        editor_el.style.backgroundColor = '#ECECEC';
    }

    let editor_height = editor_el.offsetHeight;

    const train_controller_container = document.createElement("div");
    train_controller_container.classList.add('train_controller_container');
    train_controller_container.id = "train_controller_container";
    train_controller_container.style.top = editor_height / 2 - train_controller_height / 2 + 'px';
    train_controller_container.style.width = train_controller_width + 'px';
    train_controller_container.style.height = train_controller_height + 'px';
    editor_el.appendChild(train_controller_container);

    //Add loco picture
    let train_picture_btn_width = 120;
    let train_picture_top = 40;

    const train_picture_btn = document.createElement("div");
    train_picture_btn.classList.add('train_picture_btn');
    train_picture_btn.style.width = train_picture_btn_width + 'px';
    train_picture_btn.style.height = train_picture_btn_width + 'px';
    train_picture_btn.style.borderRadius = train_picture_btn_width / 2 + 'px';

    train_picture_btn.style.left = train_controller_width / 2 - train_picture_btn_width / 2 + 'px';
    train_picture_btn.style.top = train_picture_top + 'px';
    train_controller_container.appendChild(train_picture_btn);


    let traingle_offset = 40;

    //Add reverse btn
    let triangle_btn_width = 25;
    let reverse_btn_left = train_controller_width / 2 - train_picture_btn_width / 2 - traingle_offset - triangle_btn_width;

    const train_reverse_btn = document.createElement("div");
    train_reverse_btn.classList.add('triangle_left');
    train_reverse_btn.style.left = reverse_btn_left + 'px';
    train_reverse_btn.style.top = train_picture_top + train_picture_btn_width / 2 - triangle_btn_width + 'px';
    train_controller_container.appendChild(train_reverse_btn);

    //Add forward btn
    const train_forward_btn = document.createElement("div");
    train_forward_btn.classList.add('triangle_right');
    train_forward_btn.style.left = train_controller_width / 2 + train_picture_btn_width / 2 + traingle_offset + 'px';
    train_forward_btn.style.top = train_picture_top + train_picture_btn_width / 2 - triangle_btn_width + 'px';
    train_controller_container.appendChild(train_forward_btn);

    //Create speed slider
    let speed_slider = document.createElement('input');
    speed_slider.classList.add('speed_slider');
    speed_slider.id = "train_speed";
    speed_slider.type = 'range';
    speed_slider.min = 0;
    speed_slider.max = 1;
    speed_slider.value = 0.0;
    speed_slider.step = 0.01;
    speed_slider.style.width = train_controller_width - 2 * reverse_btn_left + 'px';
    speed_slider.style.left = reverse_btn_left + 'px';
    speed_slider.style.top = train_picture_top + train_picture_btn_width + 20 + 'px';
    train_controller_container.appendChild(speed_slider);

    let function_btn_width = 60;
    let function_btn_height = 40;

    //Create minus button
    let minus_btn_top = 40;
    let speed_btns_top = speed_slider.offsetTop + minus_btn_top;

    const speed_minus_btn = document.createElement("div");
    speed_minus_btn.innerText = "-";
    speed_minus_btn.classList.add('train_function_btn');
    speed_minus_btn.style.width = function_btn_width + 'px';
    speed_minus_btn.style.height = function_btn_height + 'px';
    speed_minus_btn.style.left = reverse_btn_left + 'px';
    speed_minus_btn.style.top = speed_btns_top + 'px';
    train_controller_container.appendChild(speed_minus_btn);

    //Create plus button
    const speed_plus_btn = document.createElement("div");
    speed_plus_btn.innerText = "+";
    speed_plus_btn.classList.add('train_function_btn');
    speed_plus_btn.style.width = function_btn_width + 'px';
    speed_plus_btn.style.height = function_btn_height + 'px';
    speed_plus_btn.style.right = reverse_btn_left + 'px';
    speed_plus_btn.style.top = speed_btns_top + 'px';
    train_controller_container.appendChild(speed_plus_btn);

    //Create stop button
    let stop_btn_width = function_btn_width * 2 - 20;
    const speed_stop_btn = document.createElement("div");
    speed_stop_btn.innerText = "STOP";
    speed_stop_btn.classList.add('train_function_btn');
    speed_stop_btn.style.backgroundColor = '#E63946';
    speed_stop_btn.style.color = '#ffffff';
    speed_stop_btn.style.height = function_btn_height + 'px';
    speed_stop_btn.style.width = stop_btn_width + 'px';
    speed_stop_btn.style.left = train_controller_width / 2 - stop_btn_width / 2 + 'px';
    speed_stop_btn.style.top = speed_btns_top + 'px';
    train_controller_container.appendChild(speed_stop_btn);


    //Generate function buttons
    let function_btns_top = speed_btns_top + 40 + function_btn_height;
    let function_btns_left = reverse_btn_left;
    function_btn_width = 50;
    let func_number = 1;
    let cell_width = (train_controller_width - 2 * function_btns_left) / 4;

    for (let row = 0; row < 4; row += 1) {
        for (let col = 0; col < 4; col += 1) {

            const func_btn = document.createElement("div");
            func_btn.innerText = `F${func_number}`;
            func_btn.classList.add('train_function_btn');
            func_btn.style.height = function_btn_height + 'px';
            func_btn.style.width = function_btn_width + 'px';
            func_btn.style.left = function_btns_left + cell_width/2 - function_btn_width/2 + col * cell_width + 'px';
            func_btn.style.top = function_btns_top + row * 60 + 'px';
            train_controller_container.appendChild(func_btn);

            func_number += 1;
        }
    }

}

window.addEventListener('resize', resize);
function resize() {
    start_controller();
}


customElements.define("visual-controller", VisualController);