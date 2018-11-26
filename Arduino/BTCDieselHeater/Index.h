const char MAIN_PAGE[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
	<head>
		<script>
			var Socket;
			function init() {
			Socket = new WebSocket('ws://' + window.location.hostname + ':81/');
			Socket.onmessage = function(event){
			document.getElementById("TempCurrent").innerHTML = event.data;
			}
		}



			</script>

		<meta name="viewport" content="width=device-width, initial-scale=1">
	<style>
	.switch {
		position: relative;
		display: inline-block;
		width: 60px;
		height: 34px;
	}

	.switch input { 
		opacity: 0;
		width: 0;
		height: 0;
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
</style>
<title>Chinese Diesel Heater Web Controller Interface</title>
</head>
<body onload="javascript:init()">

<h1>Chinese Diesel Heater Web Control</h1>
<div>OFF <label class="switch">
<input type="checkbox" id="onofftoggle" onclick="OnOffCheck()">
<span class="slider round"></span>
</label> ON 
</div>
<p id="text" style="display:none">Heater Is ON</p>
<div>
<h2>Temperature Control</h2>
</div>
<input id="slide" type="range" min="1" max="100" step="1" value="10">
<div>
<b>Desired Temp: </b>
<Span id="sliderAmount"></Span>
<div>
</div>
<b>Current Temp: </b><span id="TempCurrent">




<script>


	// Function to check the power on/off slide switch.
	function OnOffCheck(){
	// Get the checkbox status and place in the checkbox variable
	var checkBox = document.getElementById("onofftoggle");
	// Send a message to the Devel console of web browser for debugging

	console.log(document.getElementById("onofftoggle").checked);  
	// Get the output text
	var text = document.getElementById("text");
	// If the checkbox is checked, display the output text
	// We also need to send a message back into the esp as we cannot directly run Arduino Functions from within the javascript
	
	if (checkBox.checked == true){
		//Insert Code Here To Turn On The Heater
//		Socket.send("P1");
		Socket.send("[CMD]ON");
		text.style.display = "block";
		} 
	else{
		//Insert Code Here To Turn Off The Heater
		text.style.display = "none";
//		Socket.send("P0");
		Socket.send("[CMD]OFF");

	}
}

var slide = document.getElementById('slide');
    sliderDiv = document.getElementById("sliderAmount");

slide.oninput = function() {
    sliderDiv.innerHTML = this.value;
}

</script>
</body>
</html> 

)=====";