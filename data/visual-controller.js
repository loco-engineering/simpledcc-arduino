import { add_state, update_state } from "./storage.js";
import { available_outputs } from "./api.js"

class VisualController extends HTMLElement {

    //Example of canvas zooming from https://codepen.io/chengarda/pen/wRxoyB
    constructor() {
        super();

        this.innerHTML = /*html*/`
        <input id="background_image" type="file" accept="image/*">

        <canvas id="canvas_ctrl" class="canvas_controller">


        </canvas>
        
        `;

        var canvas_el = document.getElementById("canvas_ctrl");
        const ctx = canvas_el.getContext('2d');

        let navbar_height = document.querySelector('#navbar').offsetHeight;

        canvas_el.width = document.documentElement.clientWidth;
        canvas_el.height = document.documentElement.clientHeight - navbar_height;

        var background_img = null;

        document.getElementById("background_image").addEventListener("change", function (e) {

            const fileName = e.target.files[0].name;
            const reader = new FileReader();
            reader.readAsDataURL(e.target.files[0]);
            reader.onload = event => {
                const img = new Image();
                img.src = event.target.result;
                img.onload = () => {
                    compress_image(img, 0.5, 0.7);
                }
            }


        });

        let canvas = canvas_el;

        var cameraOffset = { x: canvas.width / 2, y: canvas.height / 2 }
        let cameraZoom = 1
        let MAX_ZOOM = 5
        let MIN_ZOOM = 0.1
        let SCROLL_SENSITIVITY = 0.0005

        async function draw() {

            canvas.width = document.documentElement.clientWidth;
            canvas.height = document.documentElement.clientHeight - navbar_height;

            // Translate to the canvas centre before zooming - so you'll always zoom on what you're looking directly at
            ctx.translate(canvas.width / 2, canvas.height / 2);
            ctx.scale(cameraZoom, cameraZoom);
            ctx.translate(-canvas.width / 2 + cameraOffset.x, -canvas.height / 2 + cameraOffset.y);
            ctx.clearRect(0, 0, canvas.width, canvas.height);

            if (background_img != null) {
                var scale = Math.min(canvas_el.width / background_img.width, canvas_el.height / background_img.height);

                var w = background_img.width * scale;
                var h = background_img.height * scale;

                var left = - w / 2;
                var top = - h / 2;

                ctx.drawImage(background_img, left, top, w, h);
            }


            //Draw test objects

            // Create gradient
            ctx.beginPath();

            if (ctx.filter === "none") {
                ctx.filter = "blur(15px)";
            }
            else { // Safari still doesn't support ctx.filter...
                ctx.shadowColor = "#e9c46a";
                ctx.shadowBlur = 10; // x2
                ctx.shadowOffsetX = 10;
                ctx.translate(-100, 0); // we draw the actual shape outside of the visible context
            }


            ctx.fillStyle = "#e9c46a";
            ctx.globalAlpha = 0.5;
            ctx.rect(-10, -120, 40, 80);

            //ctx.arc(0, -80, 20, 0, Math.PI*2);
            ctx.fill();
            ctx.globalAlpha = 1.0;

            ctx.beginPath();

            if (ctx.filter === "none") {
                ctx.filter = "blur(15px)";
            }
            else { // Safari still doesn't support ctx.filter...
                ctx.shadowColor = "#e9c46a";
                ctx.shadowBlur = 10; // x2
                ctx.shadowOffsetX = 10;
                ctx.translate(-100, 0); // we draw the actual shape outside of the visible context
            }

            ctx.fillStyle = "#e9c46a";
            ctx.globalAlpha = 0.5;
            ctx.rect(-10, -120, 40, 80);

            //ctx.arc(0, -80, 20, 0, Math.PI*2);
            ctx.fill();
            ctx.globalAlpha = 1.0;

            //requestAnimationFrame(draw)
        }

        // Gets the relevant location from a mouse or single touch event
        function getEventLocation(e) {
            if (e.touches && e.touches.length == 1) {
                return { x: e.touches[0].clientX, y: e.touches[0].clientY }
            }
            else if (e.clientX && e.clientY) {
                return { x: e.clientX, y: e.clientY }
            }
        }

        function drawRect(x, y, width, height) {
            ctx.fillRect(x, y, width, height)
        }

        function drawText(text, x, y, size, font) {
            ctx.font = `${size}px ${font}`
            ctx.fillText(text, x, y)
        }

        let isDragging = false
        let dragStart = { x: 0, y: 0 }

        function onPointerDown(e) {
            isDragging = true
            dragStart.x = getEventLocation(e).x / cameraZoom - cameraOffset.x
            dragStart.y = getEventLocation(e).y / cameraZoom - cameraOffset.y
        }

        function onPointerUp(e) {
            isDragging = false
            initialPinchDistance = null
            lastZoom = cameraZoom
        }

        function onPointerMove(e) {
            if (isDragging) {
                cameraOffset.x = getEventLocation(e).x / cameraZoom - dragStart.x
                cameraOffset.y = getEventLocation(e).y / cameraZoom - dragStart.y
            }
        }

        function handleTouch(e, singleTouchHandler) {
            if (e.touches.length == 1) {
                singleTouchHandler(e)
            }
            else if (e.type == "touchmove" && e.touches.length == 2) {
                isDragging = false
                handlePinch(e)
            }
        }

        let initialPinchDistance = null
        let lastZoom = cameraZoom

        function handlePinch(e) {
            e.preventDefault()

            let touch1 = { x: e.touches[0].clientX, y: e.touches[0].clientY }
            let touch2 = { x: e.touches[1].clientX, y: e.touches[1].clientY }

            // This is distance squared, but no need for an expensive sqrt as it's only used in ratio
            let currentDistance = (touch1.x - touch2.x) ** 2 + (touch1.y - touch2.y) ** 2

            if (initialPinchDistance == null) {
                initialPinchDistance = currentDistance
            }
            else {
                adjustZoom(null, currentDistance / initialPinchDistance)
            }
        }

        function adjustZoom(zoomAmount, zoomFactor) {
            if (!isDragging) {
                if (zoomAmount) {
                    cameraZoom += zoomAmount
                }
                else if (zoomFactor) {
                    console.log(zoomFactor)
                    cameraZoom = zoomFactor * lastZoom
                }

                cameraZoom = Math.min(cameraZoom, MAX_ZOOM)
                cameraZoom = Math.max(cameraZoom, MIN_ZOOM)

                console.log(zoomAmount)
            }
        }

        canvas.addEventListener('mousedown', onPointerDown)
        canvas.addEventListener('touchstart', (e) => handleTouch(e, onPointerDown))
        canvas.addEventListener('mouseup', onPointerUp)
        canvas.addEventListener('touchend', (e) => handleTouch(e, onPointerUp))
        canvas.addEventListener('mousemove', onPointerMove)
        canvas.addEventListener('touchmove', (e) => handleTouch(e, onPointerMove))
        canvas.addEventListener('wheel', (e) => adjustZoom(e.deltaY * SCROLL_SENSITIVITY))

        // Ready, set, go
        draw()

        function compress_image(imgToCompress, resizingFactor, quality) {
            // showing the compressed image
            const canvas = document.createElement("canvas");
            const context = canvas.getContext("2d");

            const originalWidth = imgToCompress.width;
            const originalHeight = imgToCompress.height;

            const canvasWidth = originalWidth * resizingFactor;
            const canvasHeight = originalHeight * resizingFactor;

            canvas.width = canvasWidth;
            canvas.height = canvasHeight;

            context.drawImage(
                imgToCompress,
                0,
                0,
                originalWidth * resizingFactor,
                originalHeight * resizingFactor
            );

            // reducing the quality of the image
            canvas.toBlob(
                (blob) => {
                    if (blob) {

                        var img = new Image();
                        img.crossOrigin = 'Anonymous';

                        img.onload = function () {
                            background_img = img;
                        }

                        img.src = URL.createObjectURL(blob);


                    }
                },
                "image/jpeg",
                quality
            );
        }

    }
}

customElements.define("visual-controller", VisualController);