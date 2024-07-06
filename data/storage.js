
var project_settings = null;
var current_decoder_id = null;

export function get_project_settings(){
    return project_settings;
}

export function set_decoder_id(decoder_id){
    current_decoder_id = decoder_id;
}

export function load_local_project(){

    project_settings = localStorage.getItem(current_decoder_id);
    if (project_settings == undefined){
        project_settings = {};
        project_settings.states = [];
    }else{
        project_settings = JSON.parse(project_settings);
    }

    console.log("Loaded a project:" + JSON.stringify(project_settings));
    return project_settings;

}

export function save_local_project(){
    localStorage.setItem(current_decoder_id, JSON.stringify(project_settings));
}

export function add_state(state) {
    state.id = generate_id();
    project_settings.states.push(state);
    save_local_project();
    return state
}

function generate_id() {
    var firstPart = (Math.random() * 46656) | 0;
    var secondPart = (Math.random() * 46656) | 0;
    firstPart = ("000" + firstPart.toString(36)).slice(-3);
    secondPart = ("000" + secondPart.toString(36)).slice(-3);
    return firstPart + secondPart;
}