#include <Arduino.h>

const char* MAIN_PAGE PROGMEM = R"=====(
<!DOCTYPE html>
<html>
  <head>
    <script>

Date.prototype.today = function () { 
    return ((this.getDate() < 10)?"0":"") + this.getDate() +"/"+(((this.getMonth()+1) < 10)?"0":"") + (this.getMonth()+1) +"/"+ this.getFullYear();
}

// Date Picker Script

window.onload = function() {
  // IE11 forEach, padStart
  (function () {
    if ( typeof NodeList.prototype.forEach === "function" ) return false;
    NodeList.prototype.forEach = Array.prototype.forEach;
  })();

  (function () {
    if ( typeof String.prototype.padStart === "function" ) return false;
    String.prototype.padStart = function padStart(length, value) {
      var res = String(this);
      if(length >= (value.length + this.length)) {
        for (var i = 0; i <= (length - (value.length + this.length)); i++) {
          res = value + res;
        }
      }
      return res;
    };
  })();

  var datePickerTpl = '<div class="yearMonth"><a class="previous">&lsaquo;</a><span class="year">{y}</span>-<span class="month">{m}</span><a class="next">&rsaquo;</a></div><div class="days"><a>1</a><a>2</a><a>3</a><a>4</a><a>5</a><a>6</a><a>7</a><a>8</a><a>9</a><a>10</a><a>11</a><a>12</a><a>13</a><a>14</a><a>15</a><a>16</a><a>17</a><a>18</a><a>19</a><a>20</a><a>21</a><a>22</a><a>23</a><a>24</a><a>25</a><a>26</a><a>27</a><a>28</a><a>29</a><a>30</a><a>31</a>';

  function daysInMonth(month, year) {
    return new Date(year, month, 0).getDate();
  }

  function hideInvalidDays(dp, month, year){
    dp.querySelectorAll(".days a").forEach(function(a){
      a.style.display = "inline-block";
    });
    var days = daysInMonth(month, year);
    var invalidCount = 31 - days;
    if(invalidCount > 0) {
      for (var j = 1; j <= invalidCount; j++) {
        dp.querySelector(".days a:nth-last-child(" + j + ")").style.display = "none";
      }
    }
  }

  function clearSelected(dp) {
    dp.querySelectorAll(".days a.selected").forEach(function(e){
      e.classList.remove("selected");
    });
  }

  function setMonthYear(dp, month, year, input) {
    dp.querySelector(".month").textContent = String(month).padStart(2, "0");
    dp.querySelector(".year").textContent = year;
    clearSelected(dp);
    hideInvalidDays(dp, month, year);
    if(input && input.value) {
      var date = input.value.split("-");
      var curYear = parseInt(dp.querySelector(".year").textContent), curMonth = parseInt(dp.querySelector(".month").textContent);
      if(date[0] == curYear && date[1] == curMonth) {
        dp.querySelector(".days a:nth-child(" + parseInt(date[2]) + ")").className = "selected";
      }
    }
  }

  document.querySelectorAll(".datepicker").forEach(function(input) {
    input.setAttribute("readonly", "true");
    var dp = document.createElement("div");
    dp.className = "contextmenu";
    dp.style.left = input.offsetLeft + "px";
    dp.style.top = input.offsetTop + input.offsetHeight + "px";
    var now = new Date();
    dp.insertAdjacentHTML('beforeEnd', datePickerTpl.replace("{m}", String(now.getMonth() + 1).padStart(2, "0")).replace("{y}", now.getFullYear()));
    hideInvalidDays(dp, now.getMonth() + 1, now.getFullYear());

    dp.querySelector("a.previous").addEventListener("click", function(e){
      var curYear = parseInt(dp.querySelector(".year").textContent), curMonth = parseInt(dp.querySelector(".month").textContent);
      var firstMonth = curMonth - 1 == 0;
      setMonthYear(dp, firstMonth ? 12 : curMonth - 1, firstMonth ? curYear - 1 : curYear, input);
    });

    dp.querySelector("a.next").addEventListener("click", function(e){
      var curYear = parseInt(dp.querySelector(".year").textContent), curMonth = parseInt(dp.querySelector(".month").textContent);
      var lastMonth = curMonth + 1 == 13;
      setMonthYear(dp, lastMonth ? 1 : curMonth + 1, lastMonth ? curYear + 1 : curYear, input);
    });

    dp.querySelectorAll(".days a").forEach(function(a){
      a.addEventListener("click", function(e) {
        clearSelected(dp);
        e.target.className = "selected";
        input.value = dp.querySelector(".year").textContent + "-" + dp.querySelector(".month").textContent + "-" + this.text.padStart(2, "0");
      });
    });

    input.parentNode.insertBefore(dp, input.nextSibling);

    input.addEventListener("focus", function(){
      if (input.value){
        var date = input.value.split("-");
        setMonthYear(dp, date[1], date[0]);
        dp.querySelector(".days a:nth-child(" + parseInt(date[2]) + ")").className = "selected";
      }
    });
  });
};
// End date Picker Script




    
      var Socket;
      function init() {
        Socket = new WebSocket('ws://' + window.location.hostname + ':81/');
      
        Socket.onmessage = function(event){
          var heater = JSON.parse(event.data);
          var key;
          for(key in heater) {
            switch(key) {
              case "CurrentTemp":
                console.log("JSON Rx: CurrentTemp:", heater.CurrentTemp);
                document.getElementById("TempCurrent").innerHTML = heater.CurrentTemp;
                break;
              case "RunState":
                console.log("JSON Rx: RunState:", heater.RunState);
                if (heater.RunState == 0) {
                  document.getElementById("myonoffswitch").checked = false;
                  document.getElementById("myonoffswitch").style = "block";
                } else if(heater.RunState >= 7) {
                  document.getElementById("myonoffswitch").checked = false;
                  document.getElementById("myonoffswitch").style = "none";
                } else {
                  document.getElementById("myonoffswitch").checked = true;
                  document.getElementById("myonoffswitch").style = "block";               
                }
                break;
              case "DesiredTemp":
                console.log("JSON Rx: DesiredTemp:", heater.DesiredTemp);
                document.getElementById("slide") = heater.DesiredTemp;
                document.getElementById("sliderAmount").innerHTML = heater.DesiredTemp;
                break;
              case "ErrorState":
                console.log("JSON Rx: ErrorState:", heater.ErrorState);
                break;
            }
          }
        }
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
//  var checkBox = document.getElementById("myonoffswitch").value;
  var checkBox = document.getElementById("myonoffswitch");

  // Send a message to the Devel console of web browser for debugging
  console.log("OnOffCheck:", document.getElementById("myonoffswitch").checked);  

  // If the checkbox is checked, display the output text
  // We also need to send a message back into the esp as we cannot directly run Arduino Functions from within the javascript
  
  if (checkBox.checked == true){
    //Insert Code Here To Turn On The Heater
    console.log("Turning On Heater");

    const cmd = { RunState: 1 };
    var str = JSON.stringify(cmd);
    
    console.log("JSON Tx:", str);
    Socket.send(str);
  } 
  else{
    //Insert Code Here To Turn Off The Heater
    console.log("Turning Off Heater");

    const cmd = { RunState: 0 };
    var str = JSON.stringify(cmd);
    
    console.log("JSON Tx:", str);
    Socket.send(str);
  }
}

function onSlide(newVal) {
  document.getElementById("sliderAmount").innerHTML = newVal;

  const cmd = { DesiredTemp: 0 };
  cmd.DesiredTemp = newVal;
  var str = JSON.stringify(cmd);

  console.log("JSON Tx:", str);
  Socket.send(str);
}

// var slide = document.getElementById("slide");
//     sliderDiv = document.getElementById("sliderAmount");

// slide.oninput = function() {
//     sliderDiv.innerHTML = this.value;
// //    Socket.send("TempDesired," + document.getElementById("slide").value);
//     Socket.send("[CMD]degC" + document.getElementById("slide").value);
//     console.log("Sending desired temp", document.getElementById("slide").value, this.value);
// }
      

</script>

    <meta name="viewport" content="height=device-height, width=device-width, initial-scale=1">
  <style>


  // Date Picker Styles

  .contextmenu {
  position: absolute;
  line-height: 20px;
  font-size: 12px;
  width: 140px;
  display: none;
  background: #f6f6f6;
  box-shadow: 0 4px 8px 0 rgba(0, 0, 0, 0.2), 0 6px 20px 0 rgba(0, 0, 0, 0.19);
}

a.previous, a.next, .days a {
  cursor: pointer;
  text-align: center;
  display: inline-block;
  width: 20px;
}

.selected {
  font-weight: bold;
  background: #8a46ff;
  color: #ffffff;
}

input.datepicker:focus + div.contextmenu {
  display: block;
}
input.datepicker:focus + div.contextmenu, div.contextmenu:hover {
  display: block;
}

// End Date Picker Styles

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
<input type="range" id="slide" min="8" max="35" step="1" value="22" oninput="onSlide(this.value)" onchange="onSlide(this.value)">
<div>
<b>Desired Temp: </b>
<span id="sliderAmount"></span>
<div>
</div>
<b>Current Temp: </b><span id="TempCurrent">
</div>
</span>

<div id="Advanced" class="AdvSettings">
Place holder for ADVANCED SETTINGS page
</div>


<Div ID="Settings">
  Current Date:<br>
  <input type="text" id="curdate" value="00:00:00">
  <br>
  Current Time:<br>
  <input type="text" id="curtime" value=time>

  <input type="text" class="datepicker">

<hr></hr>
Settings will go here for automatic mode schedule	
	
<br><br>

<input type="submit" value="Submit" onload="funcgettime()" action="SendSettings()">
   
</Div>
</body>
</html> 

)=====";
