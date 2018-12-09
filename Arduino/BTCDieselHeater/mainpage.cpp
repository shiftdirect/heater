#include <Arduino.h>

const char* MAIN_PAGE PROGMEM = R"=====(
<!DOCTYPE html>
<html>
  <head>
    <script>
      var Socket;
      function init() {
        Socket = new WebSocket('ws://' + window.location.hostname + ':81/');
      
        Socket.onmessage = function(event){
          console.log("msg rec", event.data);
          var heater = JSON.parse(event.data);
          console.log("JSON current temp", heater.CurrentTemp);
          console.log("JSON run state", heater.RunState);
          console.log("JSON desired temp", heater.DesiredTemp);
          document.getElementById("TempCurrent").innerHTML = heater.CurrentTemp;
          document.getElementById("RunState").innerHTML = heater.CurrentTemp;
          document.getElementById("TempDesired").innerHTML = heater.DesiredTemp;
          
//          var msgArray = event.data.split(","); // split message by delimiter into a string array
//          console.log("msgArray", msgArray[0]);
//          console.log("msgArray", msgArray[1]);
//          console.log("msgArray", msgArray[2]);
//          console.log("msgArray", msgArray[3]);
//          var indicator = msgArray[1]; // the first element in the message array is the ID of the object to update
//          console.log("indicator", indicator);
//          if (indicator) // if an object by the name of the message exists, update its value or its attributes
//          {
//            switch (msgArray[0])
//            {
//              case "CurrentTemp":
//                document.getElementById("TempCurrent").innerHTML = msgArray[1];
//                break;
//              case "PowerState":
//                if (msgArray[1] == 0){
//                  document.getElementById("myonoffswitch").checked = false;
//                  break;
//                } else {
//                  document.getElementById("myonoffswitch").checked = true;
//                  break;
//                }
//               case "TempDesired":
//                document.getElementById("slide").value = msgArray[1];
//                break;
//            }
//          }
        }
      }

      </script>

    <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
  

  .slider {
    position: absolute;
    cursor: pointer;
    top: 0;
    left: 0;
    right: 0;
    bottom: 0;
    background-color: #ccc;
    -webkit-transition: .4s;
    transition: .4s;
  }

  .slider:before {
    position: absolute;
    content: "";
    height: 26px;
    width: 26px;
    left: 4px;
    bottom: 4px;
    background-color: white;
    -webkit-transition: .4s;
    transition: .4s;
  }
body {
  font-family: Arial, Helvetica, sans-serif;
}

.onoffswitch {
    position: relative; width: 90px;
    -webkit-user-select:none; -moz-user-select:none; -ms-user-select: none;
}
.onoffswitch-checkbox {
    display: none;
}
.onoffswitch-label {
    display: block; overflow: hidden; cursor: pointer;
    border: 2px solid #999999; border-radius: 20px;
}
.onoffswitch-inner {
    display: block; width: 200%; margin-left: -100%;
    transition: margin 0.3s ease-in 0s;
}
.onoffswitch-inner:before, .onoffswitch-inner:after {
    display: block; float: left; width: 50%; height: 30px; padding: 0; line-height: 30px;
    font-size: 14px; color: white; font-family: Trebuchet, Arial, sans-serif; font-weight: bold;
    box-sizing: border-box;
}
.onoffswitch-inner:before {
    content: "ON";
    padding-left: 10px;
    background-color: #34A7C1; color: #FFFFFF;
}
.onoffswitch-inner:after {
    content: "OFF";
    padding-right: 10px;
    background-color: #EEEEEE; color: #999999;
    text-align: right;
}
.onoffswitch-switch {
    display: block; width: 18px; margin: 6px;
    background: #FFFFFF;
    position: absolute; top: 0; bottom: 0;
    right: 56px;
    border: 2px solid #999999; border-radius: 20px;
    transition: all 0.3s ease-in 0s; 
}
.onoffswitch-checkbox:checked + .onoffswitch-label .onoffswitch-inner {
    margin-left: 0;
}
.onoffswitch-checkbox:checked + .onoffswitch-label .onoffswitch-switch {
    right: 0px; 
}

.mobile-container {
  
  margin: auto;
  background-color: #555;
  height: 500px;
  color: white;
  border-radius: 10px;
}

.topnav {
  overflow: hidden;
  background-color: #333;
  position: relative;
}

.topnav #myLinks {
  display: none;
}

.topnav a {
  color: white;
  padding: 14px 16px;
  text-decoration: none;
  font-size: 17px;
  display: block;
}

.topnav a.icon {
  background: black;
  display: block;
  position: absolute;
  left: 0;
  top: 0;
}

.topnav a:hover {
  background-color: #ddd;
  color: black;
}

.active {
  background-color: #4CAF50;
  color: white;
}

  input:checked + .slider {
    background-color: #2196F3;
  }

  input:focus + .slider {
    box-shadow: 0 0 1px #2196F3;
  }

  input:checked + .slider:before {
    -webkit-transform: translateX(26px);
    -ms-transform: translateX(26px);
    transform: translateX(26px);
  }

  .slider.round {
    border-radius: 34px;
  }

  .slider.round:before {
    border-radius: 50%;
  }

MainPage {
   display: block
}
#Advanced {
  display: none
}
#Settings {
  display: none
}

}

</style>
<title>Chinese Diesel Heater Web Controller Interface</title>
</head>
<body onload="javascript:init()">
<div class="mobile-container">

<!-- Top Navigation Menu -->
<div class="topnav">
  <div style="padding-left:30px"><a href="javascript:void(0);" onclick="funcdispHome()" class="active">Chinese Diesel Heater Web Control</a></div>
  <div id="myLinks">
    <a href="javascript:void(0);" onclick="funcdispHome()">Home</a>
    <a href="javascript:void(0);" onclick="funcdispSettings()">Settings</a>
    <a href="javascript:void(0);" onclick="funcdispAdvanced()">Advanced Settings</a>
  </div>
  <a href="javascript:void(0);" class="icon" onclick="funcNavLinks()">
    </i>=
  </a>
</div>
<div style="padding-left:16px">
<span class="MaingPage" id="Home">
<div><H2>Power Control</H2></div>

<div class="onoffswitch">
    <input type="checkbox" onclick="OnOffCheck()" name="onoffswitch" class="onoffswitch-checkbox" id="myonoffswitch" clicked>
    <label class="onoffswitch-label" for="myonoffswitch">
        <span class="onoffswitch-inner"></span>
        <span class="onoffswitch-switch"></span>
    </label>
</div>

<div>
<h2>Temperature Control</h2>
</div>
<input id="slide" type="range" min="8" max="35" step="1" value="22">
<div>
<b>Desired Temp: </b>
<Span id="sliderAmount"></Span>
<div>
</div>
<b>Current Temp: </b><span id="TempCurrent">
</div>
</span>

<div id="Advanced" class="AdvSettings">
Place holder for ADVANCED SETTINGS page
</div>


<Div ID="Settings">
Place hold for SETTINGS page
</Div>


<script>

function funcNavLinks() {
  var x = document.getElementById("myLinks");
  if (x.style.display === "block") {
    x.style.display = "none";
  } else {
    x.style.display = "block";
  }
}

function funcdispSettings() {
    document.getElementById("Settings").style.display = "block";
    document.getElementById("Home").style.display = "none";
    document.getElementById("Advanced").style.display = "none"; 
    document.getElementById("myLinks").style.display ="none";    
}
function funcdispHome(){
    document.getElementById("Settings").style.display = "none";
    document.getElementById("Home").style.display = "block";
    document.getElementById("Advanced").style.display = "none";     
    document.getElementById("myLinks").style.display ="none";    

}

function funcdispAdvanced(){
    document.getElementById("Settings").style.display = "none";
    document.getElementById("Home").style.display = "none";
    document.getElementById("Advanced").style.display = "block";   
    document.getElementById("myLinks").style.display ="none";    

}



// Function to check the power on/off slide switch.
function OnOffCheck(){

  // Get the checkbox status and place in the checkbox variable
//  var checkBox = document.getElementById("myonoffswitch").value;
  var checkBox = document.getElementById("myonoffswitch");

  // Send a message to the Devel console of web browser for debugging
  console.log(document.getElementById("myonoffswitch").checked);  
 
  // If the checkbox is checked, display the output text
  // We also need to send a message back into the esp as we cannot directly run Arduino Functions from within the javascript
  
  if (checkBox.checked == true){
    //Insert Code Here To Turn On The Heater
    console.log("Turning On Heater");
    Socket.send("[CMD]ON");
  } 
  else{
    //Insert Code Here To Turn Off The Heater
    console.log("Turning Off Heater");
    Socket.send("[CMD]OFF");
  }
}

var slide = document.getElementById("slide");
    sliderDiv = document.getElementById("sliderAmount");

slide.oninput = function() {
    sliderDiv.innerHTML = this.value;
    Socket.send("TempDesired," + document.getElementById("slide").value);
}

</script>
</body>
</html> 

)=====";


