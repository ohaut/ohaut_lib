// CH sliders /////////////////////////////////////////////////////////////////
var slider_timer;
var ch_slider = [-1, -1, -1];
var MAX_SLIDERS = 3;

function check_setled_updates()
{
    for (i=0; i<MAX_SLIDERS; i++)
      if (ch_slider[i] >= 0){
          $.ajax({url:"/setLed?ch="+ i +"&val="+ch_slider[i]});
          ch_slider[i] = -1;
      }
}

function start_slider_timer() {
    slider_timer = setInterval(check_setled_updates, 100);
}

function set_ch_sliders(data) {
    for (i = 0 ; i < MAX_SLIDERS; i++)
      $("#ch"+i+"_slider").slider(
        { min: 0,
          max: 100,
          value: data["ch"+i] ,
          change: (function(idx){
                    return function( event, ui ) {
                              console.log("slider "+idx);
                              ch_slider[idx] = ui.value;
                            }
                  })(i)
        });
    start_slider_timer();
}

function load_sliders() {
    $.ajax({
      url: "/getLeds",
      dataType: "json",
      jsonp: false,
      success: set_ch_sliders
    });
}

// MAIN ///////////////////////////////////////////////////////////////////////

$(function() {
  load_config();
  check_for_updates('ray');
  load_sliders();
});
