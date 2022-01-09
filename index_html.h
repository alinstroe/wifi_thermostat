const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <style>
    html {
     font-family: Arial;
     margin: 0px auto;
     text-align: center;
     
    }
    h2 { 
      font-size: 3.0rem; 
      padding: 10px;
     
    }
    p { font-size: 1.0rem; }
    .units {
       font-size: 1.5rem; 
      }
    .bmp-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
   body {
    max-width: 600px; margin:0px auto; padding-bottom: 25px;
    box-shadow: 5px 5px 5px 5px #96a8a2;
    border-radius: 15px 15px 15px 15px ;
   }

 .buttons {
    box-shadow: 0 8px 16px 0 rgba(0,0,0,0.2), 0 6px 20px 0 rgba(0,0,0,0.19);
    color:rgb(5, 10, 15);
    font-size: 20px;
    border-radius: 5px 5px 5px 5px ;
            
 }
.var{
    font-size: 2rem;
}

.slider {
  -webkit-appearance: none;  
  height: 25px; 
  width: 400px;
  background: #d3d3d3; 
  outline: none; 
  border-radius: 5px; 
  opacity: 0.7; 
  -webkit-transition: .2s;
  transition: opacity .2s;
}


.slider:hover {
  opacity: 1;
}

.slider::-webkit-slider-thumb {
  -webkit-appearance: none; 
  appearance: none;
  width: 25px;
  height: 25px; 
  border-radius: 10px;
  background: rgb(5, 10, 15);
  cursor: pointer; 
}

.slider::-moz-range-thumb {
  width: 25px;
  height: 25px; 
  background: rgb(5, 10, 15);
  cursor: pointer; 
}   
</style>
</head>
<body >
  <h2>Wi-fi thermostat </h2>
  <p>
    <i class="fas fa-thermometer-half fa-2x" style="color:rgb(187, 50, 25);"></i> 
    <span class="bmp-labels ">Temperature :</span> 
    <span  id="temperature" class="var">%TEMPERATURE%</span>
    <sup class="units">&deg;C</sup>
  </p>


  <p class = "my-auto"> 
    <i class="fas fa-tint fa-2x" style="color:#00add6;"></i> 
    <span class="bmp-labels">Pressure :</span>
    <span id="pressure"  class="var">%PRESSURE%  </span>
    <sup class="units" style="text-align: justify;">hPA</sup>
  </p>

  <p>
     <i class="fas fa-fire fa-2x " style="color:#aa2504;"></i>
     <span class="bmp-labels">Heating state : </span>
     <span id="heating" class="var"> %HEATING_STATE% </span>
  </p>
  <p>
    <i class="fas fa-wind fa-2x" style="color:#2795df;"></i>
    <span class="bmp-labels">AC state : </span>
     <span id="ac" class="var">%AC_STATE% </span>
  </p>

  <div class="slidecontainer">
  <p> <span class="bmp-labels"> Desired temperature </span> </p>
   <input type="range" onchange="updateDesiredTemp(this)" min="0" max="50" value="%DESIRED_TEMP%" step = "0.5" class="slider" id="desired_temp">
  <p> <span id="set_temp_value" class="var">%DESIRED_TEMP%</span> </p>
  </div>
  <p>
    Hystersis value : 
    <input id="hist25" type="button" class="buttons" value="0.25&deg;C" onclick="setHistValue25();"  />
    <input id="hist50" type="button" class="buttons" value="0.50&deg;C"  onclick="setHistValue50();" />
  </p>

</body>

<script>
setInterval(function ( ) {
  updateTemperature();
  updatePressure();
  getDesiredTemp();
}, 1000 ) ;

setInterval(function ( ) {
  updateHeating();
  updateAC();  
}, 2000 ) ;


function updateHeating() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("heating").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "heating", true);
  xhttp.send();
}

function updateAC() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("ac").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "ac", true);
  xhttp.send();
}

function updatePressure() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("pressure").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/pressure", true);
  xhttp.send();
}

function updateTemperature()
{
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperature").innerHTML = this.responseText;
    }
  };

  xhttp.open("GET", "/temperature", true);
  xhttp.send();

}

function updateDesiredTemp(element)
{

  var xhttp = new XMLHttpRequest();
  var sliderValue = document.getElementById("desired_temp").value;
  var output = document.getElementById("set_temp_value");
  output.innerHTML = sliderValue;
  xhttp.open("GET", "/desired_temp?value="+sliderValue, true);
  xhttp.send();
  };

function getDesiredTemp(){
  var xhttp = new XMLHttpRequest();
   var output = document.getElementById("set_temp_value");
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      output.innerHTML = this.responseText;
   }
  };
  
  xhttp.open("GET", "/desired_temp", true);
  xhttp.send(); 
}

function setHistValue25(){

   
  var xhttp = new XMLHttpRequest();
  xhttp.open("GET", "/set_hist?value=0.25", true);
  xhttp.send();
  document.getElementById("hist25").disabled=true
  document.getElementById("hist50").disabled=false;

}

function setHistValue50(){

   var xhttp = new XMLHttpRequest();
   xhttp.open("GET", "/set_hist?value=0.50", true);
   xhttp.send();
  document.getElementById("hist25").disabled=false;
  document.getElementById("hist50").disabled=true;

}

</script>
</html>
)rawliteral";
