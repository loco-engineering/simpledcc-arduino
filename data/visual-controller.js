import { add_state, update_state } from "./storage.js";
import { available_outputs } from "./api.js"

class VisualController extends HTMLElement {

    constructor() {

        super();

        this.innerHTML = /*html*/`        
        `;

    }

}

var app = null;

export async function start_controller() {

    if (app != null) {
        app.destroy({ removeView: true });
    }

    app = new PIXI.Application();

    console.log(document.querySelector('#visual-controller').style.width);

    let pixi_background_color = '#ECECEC';

    if (window.innerWidth < 500) {
        pixi_background_color = '#ffffff';
    }
    await app.init({ width: document.querySelector('#visual-controller').style.width, height: document.querySelector('#visual-controller').style.height, background: pixi_background_color, resizeTo: document.querySelector('#visual-controller') })
    document.querySelector('#visual-controller').appendChild(app.canvas);

    // Create background
    const controller_width = 350;
    const controller_height = 700;

    const container = new PIXI.Container({
        x: app.screen.width / 2 - controller_width / 2,
        y: app.screen.height / 2 - controller_height / 2
    });

    app.stage.addChild(container);


    const controller_background = new PIXI.Graphics();
    controller_background.roundRect(0, 0, controller_width, controller_height, 40);
    controller_background.fill(0xffffff);
    container.addChild(controller_background);

    //Create picture circle
    const circle_photo_radius = 60
    const circle_photo_y = 100 + circle_photo_radius / 2
    const circle_photo_x = controller_width / 2

    const circle_photo = new PIXI.Graphics();
    circle_photo.circle(circle_photo_x, circle_photo_y, circle_photo_radius);
    circle_photo.fill(0xECECEC);
    // Opt-in to interactivity
    circle_photo.eventMode = 'static';

    // Shows hand cursor
    circle_photo.cursor = 'pointer'
    circle_photo.on('pointerdown', pictureClicked);

    function pictureClicked() {
        console.log("clicked");
    }

    container.addChild(circle_photo);


    //Create reverse direction triangle
    const triangle_width = 25;
    const triangle_circle_offset = 80;
    const left_triangle_x = circle_photo_x - triangle_circle_offset - circle_photo_radius / 2 - triangle_width;

    const left_triangle = new PIXI.Graphics();
    left_triangle.moveTo(left_triangle_x, circle_photo_y);
    left_triangle.lineTo(left_triangle_x + triangle_width, circle_photo_y - triangle_width);
    left_triangle.lineTo(left_triangle_x + triangle_width, circle_photo_y + triangle_width);
    left_triangle.lineTo(left_triangle_x, circle_photo_y);
    left_triangle.fill(0xECECEC);
    left_triangle.eventMode = 'static';
    left_triangle.cursor = 'pointer'
    left_triangle.on('pointerdown', changeDirectionReverse);

    function changeDirectionReverse() {
        console.log("clicked reverse");
    }

    container.addChild(left_triangle);

    //Create forward direction triangle
    const right_triangle_x = circle_photo_x + circle_photo_radius / 2 + triangle_circle_offset + triangle_width;
    const right_triangle = new PIXI.Graphics();
    right_triangle.moveTo(right_triangle_x, circle_photo_y);
    right_triangle.lineTo(right_triangle_x - triangle_width, circle_photo_y - triangle_width);
    right_triangle.lineTo(right_triangle_x - triangle_width, circle_photo_y + triangle_width);
    right_triangle.lineTo(right_triangle_x, circle_photo_y);

    right_triangle.eventMode = 'static';
    right_triangle.cursor = 'pointer'
    right_triangle.on('pointerdown', changeDirectionForward);

    function changeDirectionForward() {
        console.log("clicked forward");
    }

    right_triangle.fill(0x3D405B);
    container.addChild(right_triangle);


    //Create speed slider
    const sliderWidth = controller_width - 80;
    const slider = new PIXI.Graphics().rect(0, 0, sliderWidth, 4).fill({ color: 0x3D405B });

    slider.x = (controller_width - sliderWidth) / 2;
    slider.y = circle_photo_y + circle_photo_radius + 50;

    // Draw the handle
    const handle_width = 16;
    const handle = new PIXI.Graphics().circle(0, 0, handle_width).fill({ color: 0xD9D9D9 });

    handle.y = slider.height / 2;
    handle.x = handle_width;
    handle.eventMode = 'static';
    handle.cursor = 'pointer';

    handle.on('pointerdown', onDragStart).on('pointerup', onDragEnd).on('pointerupoutside', onDragEnd);

    container.addChild(slider);
    slider.addChild(handle);

    //Add + - and stop buttons
    const speed_control_btns_y = slider.y + 40;
    const control_btn_width = 60;
    const control_btn_height = 40;

    create_train_control_btn(left_triangle_x, speed_control_btns_y, control_btn_width, control_btn_height, "-", 0xECECEC, '#000000', 20, container, "stop_btn", speed_min_step_clicked)
    function speed_min_step_clicked() {
        console.log("clicked minus");
    }

    create_train_control_btn(right_triangle_x - control_btn_width, speed_control_btns_y, control_btn_width, control_btn_height, "+", 0xECECEC, '#000000', 20, container, "stop_btn", speed_plus_step_clicked)
    function speed_plus_step_clicked() {
        console.log("clicked plus");
    }

    const train_stop_btn_width = 80;
    create_train_control_btn(controller_width/2 - train_stop_btn_width/2, speed_control_btns_y, train_stop_btn_width, control_btn_height, "STOP", 0xE63946, '#ffffff', 15, container, "stop_btn", train_stop_clicked)
    function train_stop_clicked() {
        console.log("clicked stop");
    }

    //Generate function buttons
    let function_btns_y = slider.y + 120;
    let function_btns_x = left_triangle_x;
    let function_btn_width = 50;
    let func_number = 1;
    let cell_width = (controller_width - 2*function_btns_x)/4;

    for (let row = 0; row < 5; row += 1){
        for (let col = 0; col < 4; col += 1){
            create_train_control_btn(function_btns_x + col*cell_width + cell_width/2 - function_btn_width/2 , function_btns_y + row*60, function_btn_width, control_btn_height, `F${func_number}`, 0xECECEC, '#000000', 14, container, `func${func_number}`, speed_plus_step_clicked_down, speed_plus_step_clicked_up)
            function speed_plus_step_clicked_down() {
                console.log("clicked func " + this.tag);
                //this.alpha = 0.8;
                this.fill(0xf4a261);

            }
            function speed_plus_step_clicked_up() {
                console.log("clicked func up");
                //this.alpha = 1.0;
            }
            func_number += 1;
        }
    }

    // Add a ticker callback to move the sprite back and forth
    let elapsed = 0.0;
    app.ticker.add((ticker) => {
        elapsed += ticker.deltaTime;
    });

        // Listen to pointermove on stage once handle is pressed.
        function onDragStart()
        {
            app.stage.eventMode = 'static';
            app.stage.addEventListener('pointermove', onDrag);
        }
    
        // Stop dragging feedback once the handle is released.
        function onDragEnd(e)
        {
            app.stage.eventMode = 'auto';
            app.stage.removeEventListener('pointermove', onDrag);
        }


    
        // Update the handle's position & bunny's scale when the handle is moved.
        function onDrag(e)
        {
            const halfHandleWidth = handle.width / 2;
            // Set handle y-position to match pointer, clamped to (4, screen.height - 4).
    
            handle.x = Math.max(halfHandleWidth, Math.min(slider.toLocal(e.global).x, sliderWidth - halfHandleWidth));
            // Normalize handle position between -1 and 1.
            const t = (handle.x / (sliderWidth));
    
            console.log("New scale: " + (t));
        }

}

function create_train_control_btn(x, y, width, height, text, color, text_color, font_size, container, tag, actionDown, actionUp){

    const btn = new PIXI.Graphics();
    btn.roundRect(x,y,width,height, 8);
    btn.fill(color);
    btn.eventMode = 'static';
    btn.cursor = 'pointer';
    btn.tag = tag;
    btn.on('pointerdown', actionDown);

    if (actionUp != undefined){
        btn.on('pointerup', actionUp);
    }

    container.addChild(btn);


    const btn_text_style = new PIXI.TextStyle({
        fontFamily: 'Helvetica',
        fill: text_color,
        fontSize: font_size,
        fontWeight: 'bold',
    });

    const basicText = new PIXI.Text({ text: text, style: btn_text_style });

    basicText.x = x + width/2;
    basicText.y = y + height/2;
    basicText.anchor.set(0.5, 0.5);

    btn.addChild(basicText);
}

window.addEventListener('resize', resize);
function resize() {
    start_controller();
}


customElements.define("visual-controller", VisualController);