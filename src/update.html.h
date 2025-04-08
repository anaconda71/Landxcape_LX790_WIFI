R"(
<!DOCTYPE HTML><html>
<head>
  <title>LX790 lawn mower update</title>
  <meta name='viewport' content='width=device-width, initial-scale=1'>
  <meta content='de' http-equiv='Content-Language' />
  <meta content='text/html; charset=utf-8' http-equiv='Content-Type' />
  <link rel='icon' type='image/png' href='robomower.png'>
  <style>
    div {
      margin: 1em;
    }
    i {
      margin: 0 1em;
    }
  </style>
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
  <h3><a href="/">[return]</a></h3>
  <h2>LX790 service menu</h2>
  <h3>ESP32 firmware upgrade</h2>
  <div>
    <input id='espUpdate' type='file' accept='.bin'>
    <button onclick='uploadBin()'>Update</button>
    <i id='espProgress'></i>
  </div>
  <br><hr><br>
  <h3>ESP32 file system management</h2>
  <div>
    <input id='fileupload' type='file' onchange='uploadFile();'>
    <i id='fileProgress'></i>
  </div>

)"
