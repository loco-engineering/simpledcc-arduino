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

    var pixi_background_color = '#ECECEC';

    if (window.innerWidth < 500){
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


    //Create backward direction triangle
    const triangle_width = 25;
    const triangle_circle_offset = 60;
    const left_triangle_x = circle_photo_x - triangle_circle_offset - circle_photo_radius/2 - triangle_width;

    const left_triangle = new PIXI.Graphics();
    left_triangle.moveTo(left_triangle_x, circle_photo_y);
    left_triangle.lineTo(left_triangle_x + triangle_width, circle_photo_y - triangle_width);
    left_triangle.lineTo(left_triangle_x + triangle_width, circle_photo_y + triangle_width);
    left_triangle.lineTo(left_triangle_x, circle_photo_y);
    left_triangle.fill(0xECECEC);
    container.addChild(left_triangle);

    const right_triangle_x = circle_photo_x + circle_photo_radius/2 + triangle_circle_offset + triangle_width;
    const right_triangle = new PIXI.Graphics();
    right_triangle.moveTo(right_triangle_x, circle_photo_y);
    right_triangle.lineTo(right_triangle_x - triangle_width, circle_photo_y - triangle_width);
    right_triangle.lineTo(right_triangle_x - triangle_width, circle_photo_y + triangle_width);
    right_triangle.lineTo(right_triangle_x, circle_photo_y);
    right_triangle.fill(0x3D405B);
    container.addChild(right_triangle);

    // Add a ticker callback to move the sprite back and forth
    let elapsed = 0.0;
    app.ticker.add((ticker) => {
        elapsed += ticker.deltaTime;
        //sprite.x = 100.0 + Math.cos(elapsed/50.0) * 100.0;
    });

}

window.addEventListener('resize', resize);
function resize() {
    start_controller();
}


customElements.define("visual-controller", VisualController);