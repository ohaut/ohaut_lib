function load_config()
{
  $.ajax({
    url: "/config.json",
    dataType: "json",
    jsonp: false,
    success: function( data ) {
        for (var key in data) {
          $('#main').find('input[name="'+key+'"]').val(data[key]);
        }
        $("#startupModal").modal("hide");
        $("#startupModal").css("display", "none");
    }
  });

  $("#config-form").ajaxForm({url: "/config.json", type:"post",
                              success: function () {
                                alert("config saved, rebooting");
                                location.reload();
                              }});
}

function check_for_updates(ohaut_project) {
  function check_latest_version(my_version, my_app_version, commit_id) {
    function parse_csv_and_check(csv) {
        arrayOfLines = csv.match(/[^\r\n]+/g);
        latest_version = "0";
        latest_app_version = "0";
        arrayOfLines.shift(); // remove the description line
        for (var line in arrayOfLines) {
          values = arrayOfLines[line].split(',');
          version = values[0];
          firmware_file = values[1];
          srv_app_version = values[2];
          app_file = values[3];
          commit_id = values[4];
          if (version > latest_version) {
            latest_version = version;
            latest_commit = commit_id;
          }
          if (srv_app_version > latest_app_version ) {
            latest_app_version = srv_app_version;
            latest_commit = commit_id;
          }
        }

        $("span.server_version").html(
          "<a href=\"https://github.com/ohaut/" + ohaut_project + "/commit/" +
          commit_id +"\" target=\"_blank\">FW:" + latest_version + " HTML:" +
          latest_app_version + "</a>");

        if ((latest_version > my_version) ||
            (latest_app_version > my_app_version))  {
          $("#newer_version").show();
        } else {
          $("#up_to_date").show();
        }


    }
    $.ajax({url: "http://ohaut.org/" + ohaut_project + "/firmware/master/firmware.csv?r="+
                Math.random(),
            success: parse_csv_and_check});
  }

  $.ajax({url: "/update/status",
          dataType: "json",
          jsonp: false,
          success: function( data ) {
                      for (var key in data)
                        $('#'+key).html(data[key]);
                      check_latest_version(data['firmware_version'], app_version);
                    }});
}

var update_check_timer;
var current_try;
var max_update_check_tries;

function check_update_finished() {

  console.log("Checking update status, try counter: "+ current_try);

  $.ajax({url: "/update/status",
          dataType: "json",
          jsonp: false,
          success: function(data) {
            $("#updateModal").modal("hide");
            clearInterval(update_check_timer);
            console.log("Finished updating firmware");
            location.reload();

          }})

  if (current_try++ > max_update_check_tries)  {
    $("#updateModal").modal("hide");
    clearInterval(update_check_timer);
    location.reload();
  }

  value=(current_try * 100)/max_update_check_tries;
  $('#update_progress').css('width', value+'%').attr('aria-valuenow', value);
}

function start_update_check_timer() {
  update_check_timer = setInterval(check_update_finished, 5000);
  max_update_check_tries = (60*4)/5; /* 4 minutes max */
  current_try = 0;
}

// HTML onclick handlers //////////////////////////////////////////////////////
function start_firmware_update() {
  $.ajax({url: "/update/all",
          success: function( data ) {
            $("#updateModal").modal("show");
            start_update_check_timer();
          }})
  return false;
}
