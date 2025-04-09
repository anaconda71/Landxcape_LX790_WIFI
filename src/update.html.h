R"(
<html>
<head>
  <title>LX790 lawn mower Web</title>
  <meta name='viewport' content='width=device-width, initial-scale=1'>
  <meta content='en' http-equiv='Content-Language' />
  <meta content='text/html; charset=utf-8' http-equiv='Content-Type' />
  <link rel='icon' type='image/png' href='robot-mower.png'>
  <link rel='stylesheet' href='style_new.css'>

  <script>
    function uploadBin() {
      var formdata = new FormData();
      if ( espUpdate.files[0] ) {
        formdata.append('file', espUpdate.files[0]);
        var ajax = new XMLHttpRequest();
        ajax.upload.addEventListener('progress', ()=>{
          var percent = Math.round((event.loaded / event.total) * 100);
          document.getElementById('espProgress').textContent = percent + '% uploading ...';
         }, false);
        ajax.addEventListener('load', ()=>{
          document.getElementById('espProgress').textContent = 'upload finished';
          //alert('Upload Finished');
          espUpdate.value = null;
        }, false);
        ajax.addEventListener('error', errorHandler, false);
        ajax.addEventListener('abort', errorHandler, false);
        ajax.open('POST', 'execupdate');
        ajax.send(formdata);
      } else {
        alert('Please select a new ESP32 firmware (*.bin)!');
      }
    }

    function uploadFile() {
      var formdata = new FormData();
      if ( fileupload.files[0] ) {
        formdata.append('file', fileupload.files[0]);
        var ajax = new XMLHttpRequest();
        ajax.upload.addEventListener('progress', ()=>{
          var percent = Math.round((event.loaded / event.total) * 100);
          document.getElementById('fileProgress').textContent = percent + '% uploading ...';
        }, false);
        ajax.addEventListener('load', ()=>{
          document.getElementById('fileProgress').textContent = 'upload finished';
          //alert('Upload Finished');          
          fileupload.value = null;
        }, false);
        ajax.addEventListener('error', errorHandler, false);
        ajax.addEventListener('abort', errorHandler, false);
        ajax.open('POST', '/fileupload');
        ajax.send(formdata);
      } else {
        alert('Please select a file!');
      }
    }

    function errorHandler(event) {
      alert('Upload Error');
    }
  </script>  
</head>

 <body>


  <div style='overflow: hidden;'>

    <div style='float:left; width: 250px; border-right: 5px solid #FFA500; height: 100%; background-color: #b4b4b4; padding: 5px; '>

          <h3><img src='robot-mower.png' class='icon'>LX790 lawn mower</h3>
          <a href='/' class='menu_button'><img src='control.png' class='icon'>Main page</a>
          <a href='/config' class='menu_button'><img src='cogs.png' class='icon'>Settings</a>
          <a href='/update' class='menu_button'><img src='source-branch-sync.png' class='icon'>Firmware upgrade</a>
          <button name='reboot' id='btn_reboot' class='menu_button'><img src='restart.png' class='icon'>Reboot</button>


          <div class='icon_row'>
            <div class='icon_column'>
              <img id='clock' title='Clock' style='display:block; height: 25px;' src='clock.png' />
            </div>
            <div class='icon_column'>
              <img id='wifi' title='Wifi sinal strength' style='display:block; height: 25px;' src='wifi_1.png' />
            </div>
            <div class='icon_column'>
              <img id='lock' title='Unlock PIN status' style='display:block; height: 25px;' src='unlocked.png' />
            </div>
            <div class='icon_column'>
              <img id='bat' title='Battery state' style='display:block; height: 25px;' src='battery-low.png' />
            </div>
          </div>

          <div id='msg' style='padding-left: 10px'>---</div>


          <button name='stop' id='btn_stop' class='emo_menu_button' style='font-size: 100%;'>Emergency stop</button>

          <div id='uptime' style='position: absolute; bottom: 20px; font-size: 13px;'>Loading...</div>
          <div id='sw_version' style='position: absolute; bottom: 5px; font-size: 13px;'>Loading...</div>

    </div>
    <div style='background-color: #cccccc; height: 100%;overflow-y: scroll;overflow-x: scroll;'>

        <div>


          <h2 style='text-align: center;'>LX790.1 ESP32 service menu</h2>


          <div id='container'>


  <h3>ESP32 firmware upgrade</h2><br>
  <div>
    <input id='espUpdate' type='file' accept='.bin'>
    <button onclick='uploadBin()'>Update</button>
    <i id='espProgress'></i>
    <div style='margin: 10px; padding: 5px; background-color: white; border-left:#9c9c9c solid 5px;'>Do the update at your own risk. If there are problem (wrong firmware, low wifi signal), you have to update manually with cable.</div>
  </div>
  <br><hr><br>
  <h3>ESP32 file system management</h2><br>
  <div>
    <input id='fileupload' type='file' onchange='uploadFile();'>
    <i id='fileProgress'></i>
  </div>



  <table class='table' width='80%' cellpadding='0' cellspacing='0'>
                  <tr>
                    <td>Filename</td>
                    <td>Size</td>
                    <td>Delete</td>
                  </tr>

)"
