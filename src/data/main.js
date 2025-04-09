let imgBatArr = ['battery-empty.png', 'battery-low.png', 'battery-medium.png', 'battery-high.png', 'battery-charging.png'];
let wifisignalArr = ['wifi_0.png','wifi_1.png', 'wifi_2.png', 'wifi_3.png', 'wifi_4.png'];
let imgLockArr = ['unlocked.png', 'locked.png'];
let status = {lock: -1, clock: -1, battery: -1};

window.addEventListener('load', onLoad);

function ActionToggleCard(event)
{
  card = event.srcElement;
  while ( !card.classList.contains('card') ) {
    card = card.parentNode;
  }

  for (var el of card.children ) {
    if ( !el.classList.contains('toggle_card') ) {
      el.classList.toggle('hide');
    }
  }
}

function onLoad(event) 
{
  console.log('...init');
  for (var el of document.querySelectorAll(".toggle_card") ) {
    el.addEventListener("click",  ActionToggleCard);
  }

  for (var btn of document.getElementsByTagName("button") ) {
    initButton(btn);
  }


  ActValues();
}
function initButton(btn)
{
  btn.addEventListener("mousedown",  ActionButtonOn);
  btn.addEventListener("mouseup",    ActionButtonOff);
  btn.addEventListener("mouseleave", ActionButtonOff);
}
function ActionButtonOn() 
{
  console.log(this.name + " pressed");
  fetch("/cmd?parm="+this.name+"&value=1");
}
function ActionButtonOff() 
{
  console.log(this.name + " released");
  fetch("/cmd?parm="+this.name+"&value=0");
}  
function ActionCheckbox() 
{
  var val = this.checked ? 1 : 0;
  console.log(this.name + " checked: " + val);
  fetch("/cmd?parm="+this.name+"&value="+val);
  this.indeterminate = true;
}    

let oldStatus = {};
function ActValues()
{
  var xhr = new XMLHttpRequest();

  fetch("/status")
    .then((response) => {
      return response.json()})
    .then((status) => {
      setTimeout(ActValues, 500);

      
      document.getElementById("seg1").textContent = status.digits[0];
      document.getElementById("seg2").textContent = status.digits[1];
      document.getElementById("seg3").textContent = status.digits[2];
      document.getElementById("seg4").textContent = status.digits[3];
      document.getElementById("gap2").textContent = status.point;
      if ( status.lock != oldStatus.lock )
        document.getElementById("lock").src = imgLockArr[status.lock];
      if ( status.clock != oldStatus.clock )
        document.getElementById("clock").style.visibility = (status.clock ? 'visible' : 'hidden');
      if ( status.mode == "LX790_CHARGING" )
        status.battery = 4;
      if ( status.battery != oldStatus.battery )
      document.getElementById("bat").src = imgBatArr[status.battery];

      var signal_level=0;
      /*-59 felett 4
      -70 felett 3
      -80 felett 2
      -90 felett 1*/
      if(status.rssi > -59)
        signal_level = 4;
      else if(status.rssi > -70)
        signal_level = 3;
      else if(status.rssi > -82)
        signal_level = 2;
      else if(status.rssi > -90)
        signal_level = 1;

      document.getElementById("wifi").src = wifisignalArr[signal_level];

      document.getElementById("msg").textContent = status.msg;

      document.getElementById("sw_version").textContent = "SW ver.: " + status.sw_version + " " + status.build;
      document.getElementById("uptime").textContent = "ESP32 up: " + status.uptime;
      
      document.getElementById("hostname").textContent = status.hostname;

      if ( status.cmdQueue != oldStatus.cmdQueue ) {
        const btns = document.getElementsByClassName("button");
        for (let i = 0; i < btns.length; i++) {
          if ( !btns[i].classList.contains("buttonred") ) {
            btns[i].disabled = (status.cmdQueue > 0);
          }
        }
      }

      oldStatus = status;

    })
    .catch((error) => {
      setTimeout(ActValues, 10000);
    });      
}