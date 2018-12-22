/*
 * This file is part of the "bluetoothheater" distribution 
 * (https://gitlab.com/mrjones.id.au/bluetoothheater) 
 *
 * Copyright (C) 2018  Ray Jones <ray@mrjones.id.au>
 * Copyright (C) 2018  James Clark
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * 
 */

#include <Arduino.h>

const char* MAIN_PAGE PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="utf-8"/>
    <link rel="icon" href="data;,">
    <script>

      var Socket;
      function init() {
        Socket = new WebSocket('ws://' + window.location.hostname + ':81/');
      
        Socket.onmessage = function(event){
          var heater = JSON.parse(event.data);
          var key;
          for(key in heater) {
            console.log("JSON decode:", key, heater[key]);
            switch(key) {
              case "RunState":
                if (heater[key] == 0) {
                  document.getElementById("myonoffswitch").checked = false;
                  document.getElementById("myonoffswitch").style = "block";
                  document.getElementById("onoffswitch").style.visibility = "visible";
                } else if(heater[key] >= 7) {
                  document.getElementById("myonoffswitch").checked = false;
                  document.getElementById("myonoffswitch").style = "none";
                  document.getElementById("onoffswitch").style.visibility = "hidden";
                } else {
                  document.getElementById("myonoffswitch").checked = true;
                  document.getElementById("myonoffswitch").style = "block";             
                  document.getElementById("onoffswitch").style.visibility = "visible";
                }
                document.getElementById("RunString").style.visibility = (heater[key] == 5 || heater[key] == 0) ? "hidden" : "visible";
                break;
              case "ErrorString":
              case "PumpFixed":
              case "RunString":
              case "TempCurrent":
                document.getElementById(key).innerHTML = heater[key];
                break;
              case "TempDesired":
                document.getElementById("slide").value = heater[key];
                document.getElementById(key).innerHTML = heater[key];
                break;
              case "ErrorState":
                document.getElementById("ErrorDiv").hidden = heater[key] <= 1;
                break;
				case "TempBody":
				//The threshold levels for each bar to come on are: 21°C, 41°C, 61°C, 81°C, 101°C, 121°C
							if(heater[key] > 120){
						document.getElementById("TopBar").className = "active121";
						}
						else if(heater[key] > 100){
						document.getElementById("TopBar").className = "active101";
						}
							else if(heater[key] > 80){
						document.getElementById("TopBar").className = "active81";
    				}
							else if(heater[key] > 60){
						document.getElementById("TopBar").className = "active61";
						}
						else if(heater[key] > 40){
						document.getElementById("TopBar").className = "active41";
						}
					else if(heater[key] > 20){
						document.getElementById("TopBar").className = "active21";
						}
					else {
						document.getElementById("TopBar").className = "active0";
						}
				break;
              case "Thermostat":
                if(heater[key] != 0) {
                  document.getElementById("FixedDiv").hidden = true;
                  document.getElementById("ThermoDiv").hidden = false;
                }
                else {
                  document.getElementById("FixedDiv").hidden = false;
                  document.getElementById("ThermoDiv").hidden = true;
                }
                break;
            }
          }
        }
      }

function setSchedule(){
//clearly need to add some code here to send the Json formatted data to the esp
    console.log("Set Schedule Button Press")
}
function sendJSONobject(obj){
  var str = JSON.stringify(obj);
  console.log("JSON Tx:", str);
  Socket.send(str);
}

// Scripts for date handling
Date.prototype.today = function () { 
    return ((this.getDate() < 10)?"0":"") + this.getDate() +"/"+(((this.getMonth()+1) < 10)?"0":"") + (this.getMonth()+1) +"/"+ this.getFullYear();
}

// Scripts for setting date and time

function setcurrenttime(){
  var cmd = {};
  cmd.Time = document.getElementById("curtime").value;
  sendJSONobject(cmd);


}

function setcurrentdate(){
  var cmd = {};
  cmd.Date =  document.getElementById("curdate").value;
  sendJSONobject(cmd);

}

function funcNavLinks() {
  var x = document.getElementById("myLinks");
  if (x.style.display === "block") {
    x.style.display = "none";
  } else {
    x.style.display = "block";
  }
}

function checkTime(i)
{
if (i<10)
  {
  i="0" + i;
  }
return i;
}

function funcdispSettings() {
    document.getElementById("Settings").style.display = "block";
    currentTime = new Date();
	  var h = currentTime.getHours();
	  var m = currentTime.getMinutes();
	  var s = currentTime.getSeconds();
	  // add a zero in front of numbers<10
	  h = checkTime(h);
	  m = checkTime(m);
	  s = checkTime(s);

    console.log("Hours",h);
    console.log("Minutes",m);
    console.log("Seconds",s);
	  document.getElementById("curtime").value = h + ":" + m + ":" + s;
	  document.getElementById("curdate").value = currentTime.today()
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
  var checkBox = document.getElementById("myonoffswitch");

  // Send a message to the Devel console of web browser for debugging
  console.log("OnOffCheck:", document.getElementById("myonoffswitch").checked);  

  // If the checkbox is checked, display the output text
  // We also need to send a message back into the esp as we cannot directly run Arduino Functions from within the javascript
  
  var cmd = {};
  if (checkBox.checked){
    //Insert Code Here To Turn On The Heater
    console.log("Turning On Heater");

    cmd.RunState = 1;
    sendJSONobject(cmd);
  } 
  else{
    //Insert Code Here To Turn Off The Heater
    console.log("Turning Off Heater");

    cmd.RunState = 0;
    sendJSONobject(cmd);
  }
}

function onSlide(newVal, JSONKey) {
//elementid must equal the JSON name for each setting
  
  document.getElementById(JSONKey).innerHTML = newVal;

  var cmd = {};
  cmd[JSONKey] = newVal;  // note: variable name needs []
  cmd.NVsave = 8861;      // named variable DOESN'T !!
  sendJSONobject(cmd);
}

     

</script>

    <meta name="viewport" content="height=device-height, width=device-width, initial-scale=1">
  <style>

  .throb_me {
    animation: throbber 1s linear infinite;
  }

  @keyframes throbber {
    50% {
      opacity: 0;
    }
  }

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

.active0 {
  background-color: #40ffff;
  color: #000000;
}
.active21 {
  background-color: #5f9ea0;
  color: #ffffff;
}
.active41 {
  background-color: #4CAF50;
  color: #ffffff;
}
.active61 {
  background-color: #eeee00;
  color: #000000;
}
.active81 {
  background-color: #8b0000;
  color: #ffffff;
}
.active101 {
  background-color: #cd0000;
  color: #ffffff;
}
.active121 {
  background-color: #ff0000;
  color: #ffffff;
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
  <div id="TopBar" style="padding-left:30px"><a href="javascript:void(0);" onclick="funcdispHome()" >Chinese Diesel Heater Web Control</a></div>
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

<div class="onoffswitch" id="onoffswitch">
    <input type="checkbox" onclick="OnOffCheck()" name="onoffswitch" class="onoffswitch-checkbox" id="myonoffswitch" clicked>
    <label class="onoffswitch-label" for="myonoffswitch">
        <span class="onoffswitch-inner"></span>
        <span class="onoffswitch-switch"></span>
    </label>
</div>
<span class="throb_me" id="RunString" style="visibility:hidden"></span>

<div>
<h2>Temperature Control</h2>
</div>
<input type="range" id="slide" min="8" max="35" step="1" value="22" oninput="onSlide(this.value, 'TempDesired')" onchange="onSlide(this.value, 'TempDesired')">
<div id="ThermoDiv">
<b>Desired Temp: </b>
<span id="TempDesired"></span>
</div>
<div id="FixedDiv">
<b>Fixed Hz: </b>
<span id="PumpFixed"></span>
</div>
<div>
<b>Current Temp: </b><span id="TempCurrent">
</div>
<div id="ErrorDiv" style="color:crimson" hidden>
<b>Error <span id="ErrorString"> </b>
</div>
</span>

<div id="Advanced">
Advanced Settings
<b>Pump Min</b>
<input type="range" id="PumpMinSlide" min=".5" max="35" step=".5" value="22" oninput="onSlide(this.value, 'PumpMin')" onchange="onSlide(this.value, 'PumpMin')"> <span id="PumpMin"></span>
<div>
<b>Pump Max</b>
<input type="range" id="PumpMaxSlide" min=".5" max="5" step=".5" value="22" oninput="onSlide(this.value, 'PumpMax')" onchange="onSlide(this.value, 'PumpMax')"> <span id="PumpMax"></span>
</div>
<div>
<b>Fan Min</b>
<input type="range" id="FanMinSlide" min="1000" max="5000" step="50" value="22" oninput="onSlide(this.value, 'FanMin')" onchange="onSlide(this.value, 'FanMin')"> <span id="FanMin"></span>
</div>
<div>
<b>Fan Max</b>
<input type="range" id="FanMaxSlide" min="1000" max="5000" step="50" value="22" oninput="onSlide(this.value, 'FanMax')" onchange="onSlide(this.value, 'FanMax')"> <span id="FanMax"></span>
</div>
</div>


<Div id="Settings">
  Current Date:<br>
  <input type="text" id="curdate"><input type="button" Value="Set Date" onclick="setcurrentdate()">

  <br>
  Current Time (24 Hour Format):<br>
  <input type="text" id="curtime"> <input type="button" Value="Set Time" onclick="setcurrenttime()">

<hr></hr>
<br><br>
Mon:<input type="checkbox" border-radius="4px" name="Mon" value="Mon"> <input type="text" class="schedule" id="Mon">    <input type="text" id="curtime"> <br>
Tue:<input type="checkbox" border-radius="4px" name="Tue" value="Tue">  <input type="text" class="schedule" id="Tue">    <input type="text" id="curtime"><br>
Wed:<input type="checkbox" border-radius="4px" name="Wed" value="Wed"> <input type="text" class="schedule" id="Wed">    <input type="text" id="curtime"><br>
Thu:<input type="checkbox" border-radius="4px" name="Thu" value="Thu"> <input type="text" class="schedule" id="Thu"     <input type="text" id="curtime"><br>
Fri:<input type="checkbox" border-radius="4px" name="Fri" value="Fri"> <input type="text" class="schedule" id="Fri">    <input type="text" id="curtime"><br>
Sat:<input type="checkbox" border-radius="4px" name="Sat" value="Sat"> <input type="text" class="schedule" id="Sat">    <input type="text" id="curtime"><br>
Sun:<input type="checkbox" border-radius="4px" name="Sun" value="Sun"> <input type="text" class="schedule" id="Sun">    <input type="text" id="curtime"><br>
<input type="button" Value="Save Schedule" onclick="setSchedule()">
</Div>
</body>
</html> 

)=====";
