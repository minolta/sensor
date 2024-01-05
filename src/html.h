#ifndef HTML_H
#define HTML_H
#include <Arduino.h>

const char addjob_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<style>
  html {font-family: Arial; display: inline-block; text-align: center;}
    h2 {font-size: 3.0rem;}
    p {font-size: 3.0rem;}
    body {max-width: 600px; margin:0px auto; padding-bottom: 25px;}
    .switch {position: relative; display: inline-block; width: 120px; height: 68px} 
    .switch input {display: none}
    .slider {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; border-radius: 6px}
    .slider:before {position: absolute; content: ""; height: 52px; width: 52px; left: 8px; bottom: 8px; background-color: #fff; -webkit-transition: .4s; transition: .4s; border-radius: 3px}
    input:checked+.slider {background-color: #b30000}
    input:checked+.slider:before {-webkit-transform: translateX(52px); -ms-transform: translateX(52px); transform: translateX(52px)}

#customers {
    /* font-family: 'Karla', Tahoma, Varela, Arial, Helvetica, sans-serif; */
    border-collapse: collapse;
    width: 100%%;
    /* font-size: 12px; */
}
#btn {
  border: 1px solid #777;
  background: #6e9e2d;
  color: #fff;
  font: bold 11px 'Trebuchet MS';
  padding: 4px;
  cursor: pointer;
  -moz-border-radius: 4px;
  -webkit-border-radius: 4px;
}
.button {
  background-color: #4CAF50; /* Green */
  border: none;
  color: white;
  padding: 15px 32px;
  text-align: center;
  text-decoration: none;
  display: inline-block;
  font-size: 16px;
}
#customers td,
#customers th {
    border: 1px solid #ddd;
    padding: 8px;
}


/* #customers tr:nth-child(even){background-color: #f2f2f2;} */

#customers tr:hover {
    background-color: #ddd;
}

#customers th {
    padding-top: 12px;
    padding-bottom: 12px;
    text-align: left;
    background-color: #4CAF50;
    color: white;
}
</style>
<script>
function deleteallconfig()
{
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/resetconfig", true); 
    xhr.send();
}
function remove(config)
{
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/removeconfig?configname="+config, true); 

     xhr.addEventListener("readystatechange", () => {
     console.log(xhr.readystate);
    if (xhr.readyState === 4 && xhr.status === 200) {
     console.log(xhr.responseText);
     location.reload();
     }
 });
    xhr.send();
}
function add()
{
  var xhr = new XMLHttpRequest();
  var input = document.getElementById('newconfigname');
  var value = document.getElementById('newvalue');
  xhr.open("GET", "/setconfig?configname="+input.value+"&value="+value.value, true); 
  xhr.addEventListener("readystatechange", () => {
     console.log(xhr.readystate);
    if (xhr.readyState === 4 && xhr.status === 200) {
     console.log(xhr.responseText);
     var o =  JSON.parse(xhr.responseText);
     var t = document.getElementById('customers');
     var row = t.insertRow();
     row.innerHTML = "<td>"+o.setconfig+"</td><td>"+o.value+"</td><td><input value="+o.value+"></td>";
     }
 });
  xhr.send();
}
function setvalue(element,configname,value) {
  console.log("Call",element);
  var xhr = new XMLHttpRequest();
  var input = document.getElementById(configname);

  xhr.open("GET", "/setconfig?configname="+configname+"&value="+input.value, true); 
  xhr.addEventListener("readystatechange", () => {
     console.log(xhr.readystate);
    if (xhr.readyState === 4 && xhr.status === 200) {
     console.log(xhr.responseText);
    var o =  JSON.parse(xhr.responseText);
  var showvalue = document.getElementById(configname+'value');  
  console.log('Showvalue',showvalue);
  console.log('O',o);
  showvalue.innerHTML = o.value
    } else if (xhr.readyState === 4) {
     console.log("could not fetch the data");
     }
        });
  xhr.send();
}

function deletejob(id)
{
   var xhr = new XMLHttpRequest();

let url ="/deletejob?id="+id;
console.log('deleteurl',url);

  xhr.open("GET",url , true); 
  xhr.addEventListener("readystatechange", () => {
    if (xhr.readyState === 4 && xhr.status === 200) {
        location.reload();
        console.log('delete id',xhr.responseText);
        }
        });
  xhr.send();
}

function savejob() {
  var xhr = new XMLHttpRequest();
  var hlow = document.getElementById('hlow');
  var hhigh = document.getElementById('hhigh');
var output = document.getElementById('output');
var port = document.getElementById('port');
var runtime = document.getElementById('runtime');
var waittime = document.getElementById('waittime');
var stime = document.getElementById('stime');
var etime = document.getElementById('etime');

let url ="/savejob?hlow="+hlow.value+"&hhigh="+hhigh.value+"&output="+output.value+"&port="+port.value+"&runtime="+runtime.value+"&waittime="+waittime.value+"&enable=1&stime="+stime.value+"&etime="+etime.value;


  xhr.open("GET",url , true); 
  xhr.addEventListener("readystatechange", () => {
    if (xhr.readyState === 4 && xhr.status === 200) {
        // location.reload();
      //  setTimeout(function(){location.reload(); }, 2000);
        hlow.innerHTML = "";
        hhigh.innerHTML = "";
        runtime.innerHTML = "";
        waittime.innerHTML="";
        console.log('saveurl',url);
        console.log('save',xhr.responseText);
        }
        });
  xhr.send();
}



setInterval(()=>{
  
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/time", true); 
  xhr.addEventListener("readystatechange", () => {
    if (xhr.readyState === 4 && xhr.status === 200) {
    // console.log(xhr.responseText);
    var o =  JSON.parse(xhr.responseText);
    console.debug('O',o);
    
    var uptime = document.getElementById("devicedatetime"); 
     
    uptime.innerHTML = new Date(o.timestamp  * 1000).toLocaleString();

    uptime = document.getElementById("uptime"); 
    uptime.innerHTML = " Uptime:"+o.uptime

        uptime = document.getElementById("heap"); 
    uptime.innerHTML = " Heap:"+o.heap

var ht = document.getElementById("ht");
    ht.innerHTML = "T: "+o.t+" H: "+o.h;
    } else if (xhr.readyState === 4) {
     console.debug("could not fetch the data");
     }
    });
  xhr.send();
  // console.log('Call refresh');
}
, 500);



//for task
setInterval(()=>{
  
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/task", true); 
  xhr.addEventListener("readystatechange", () => {
    if (xhr.readyState === 4 && xhr.status === 200) {
    // console.log(xhr.responseText);
    var o =  JSON.parse(xhr.responseText);
    console.debug('task list',o);
    
    var tasktable = document.getElementById("tasklist"); 
    let b = "<tr><td><h1>Task</h1></td></tr><tr><td>id</td><td>runtime</td><td>waittim</td><td>hlow</td><td>hhigh</td><td>run</td></tr>";
    for(let task of o)
     b+= "<tr><td>"+task.id+"</td><td>"+task.runtime+"</td><td>"+task.waittime+"</td><td>"+task.hlow+"</td><td>"+task.hhigh+"</td><td>"+task.status+"</td></tr>";
  tasktable.innerHTML = b;
    } else if (xhr.readyState === 4) {
     console.debug("could not fetch the data");
     }
    });
  xhr.send();
  // console.log('Call refresh');
}
, 1000);




</script>

<head>
    <title>Add job</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <meta charset="UTF-8">
</head><body>
<b><lable id=devicedatetime>Datetime</lable></b> <i><label id=uptime>0</label></i> <i><label id=ht>0</label></i> <i><label id=heap>0</label></i>
<br>
<h1>New Job</h1>
<table id=customers>

<tr>
<td>PORT</td>
<td>
  <select name="port" id=port>
    <option value="D4">D4</option>
    <option value="D5">D5</option>
    <option value="D6">D6</option>
    <option value="D7">D7</option>
  </select>
  </td>
</tr>
<tr><td>Output</td><td>
  <select name="output" id=output>
    <option value="0">0</option>
    <option value="1">1</option>
  </select>
  </td>
  </tr>
  
  <tr><td>ความชื้นขั้นตำที่จะทำงาน </td><td><label> <input id=hlow type='number'  name="hlow"></label></td></tr>


 <tr><td>ความชื้นสูงสุดที่จะทำงาน</td><td> <label> <input id=hhigh type="number" name="hhigh"></td></tr>

 <tr><td>เวลาในการทำงาน </td><td><label><input id=runtime type="number" name="runtime"></label></td></tr>

 <tr><td>เวลาหยุดพัก </td><td><label><input id=waittime  type="number" name="waittime"></label></td></tr>
 <tr><td>เวลาเริ่มช่วงการทำงาน</td><td><input id=stime type="time"  value="00:00" name="stime"></td></tr>
 <tr><td>เวลาสิ้นสุดช่วงการทำงาน</td><td> <input id=etime type="time" value="00:00" nmae="etime"></td></tr>
  <tr><td colspan="2" align=right><button onClick='savejob()'>Add</button></td></tr>

  </table>

<h1>Config</h1>
<table id='customers'>
%CONFIG%
</table>

<h1>Job List</h1>
<table id='customers'>
    <tr>
        <td colspan="11">Jobs</td>
    </tr>
    <tr>
      <td>ID</td>
      <td>H low</td>    
      <td>H high</td>
      <td>Port</td>  
      <td>Runtime</td>    
      <td>Waittime</td>    
      <td>Out</td>    
      <td>Enable</td>
      <td>Stime</td>
      <td>Etime</td>

      <td>option</td>


    </tr>
    <tr>
        %list%
    </tr>
</table>

<table id="tasklist">
</table>
</body>)rawliteral";
const char indexstanalone_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>ESP WIFI </title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  <a href='/addjob'>Add job</a>
  <hr>
  <a href='/setconfigwww'>config</a>
  <hr>
  <a href='/restart'>restart</a>
 <br> contract ky@pixka.me 
</body></html>)rawliteral";
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>ESP WIFI </title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  <form action="/get">
    SSID: <input type="text" name="ssid">
    PASSWORD: <input type="password" name="password">
    <input type="submit" value="Submit">
  </form><br> contract ky@pixka.me 
</body></html>)rawliteral";
const char configfile_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>Config</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    h2 {font-size: 3.0rem;}
    p {font-size: 3.0rem;}
    body {max-width: 600px; margin:0px auto; padding-bottom: 25px;}
    .switch {position: relative; display: inline-block; width: 120px; height: 68px} 
    .switch input {display: none}
    .slider {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; border-radius: 6px}
    .slider:before {position: absolute; content: ""; height: 52px; width: 52px; left: 8px; bottom: 8px; background-color: #fff; -webkit-transition: .4s; transition: .4s; border-radius: 3px}
    input:checked+.slider {background-color: #b30000}
    input:checked+.slider:before {-webkit-transform: translateX(52px); -ms-transform: translateX(52px); transform: translateX(52px)}

#customers {
    /* font-family: 'Karla', Tahoma, Varela, Arial, Helvetica, sans-serif; */
    border-collapse: collapse;
    width: 100%%;
    /* font-size: 12px; */
}
#btn {
  border: 1px solid #777;
  background: #6e9e2d;
  color: #fff;
  font: bold 11px 'Trebuchet MS';
  padding: 4px;
  cursor: pointer;
  -moz-border-radius: 4px;
  -webkit-border-radius: 4px;
}
.button {
  background-color: #4CAF50; /* Green */
  border: none;
  color: white;
  padding: 15px 32px;
  text-align: center;
  text-decoration: none;
  display: inline-block;
  font-size: 16px;
}
#customers td,
#customers th {
    border: 1px solid #ddd;
    padding: 8px;
}


/* #customers tr:nth-child(even){background-color: #f2f2f2;} */

#customers tr:hover {
    background-color: #ddd;
}

#customers th {
    padding-top: 12px;
    padding-bottom: 12px;
    text-align: left;
    background-color: #4CAF50;
    color: white;
}
</style>

<script>
function deleteallconfig()
{
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/resetconfig", true); 
    xhr.send();
}
function remove(config)
{
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/removeconfig?configname="+config, true); 

     xhr.addEventListener("readystatechange", () => {
     console.log(xhr.readystate);
    if (xhr.readyState === 4 && xhr.status === 200) {
     console.log(xhr.responseText);
     location.reload();
     }
 });
    xhr.send();
}
function add()
{
  var xhr = new XMLHttpRequest();
  var input = document.getElementById('newconfigname');
  var value = document.getElementById('newvalue');
  xhr.open("GET", "/setconfig?configname="+input.value+"&value="+value.value, true); 
  xhr.addEventListener("readystatechange", () => {
     console.log(xhr.readystate);
    if (xhr.readyState === 4 && xhr.status === 200) {
     console.log(xhr.responseText);
     var o =  JSON.parse(xhr.responseText);
     var t = document.getElementById('customers');
     var row = t.insertRow();
     row.innerHTML = "<td>"+o.setconfig+"</td><td>"+o.value+"</td><td><input value="+o.value+"></td>";
     }
 });
  xhr.send();
}
function setvalue(element,configname,value) {
  console.log("Call",element);
  var xhr = new XMLHttpRequest();
  var input = document.getElementById(configname);

  xhr.open("GET", "/setconfig?configname="+configname+"&value="+input.value, true); 
  xhr.addEventListener("readystatechange", () => {
     console.log(xhr.readystate);
    if (xhr.readyState === 4 && xhr.status === 200) {
     console.log(xhr.responseText);
    var o =  JSON.parse(xhr.responseText);
  var showvalue = document.getElementById(configname+'value');  
  console.log('Showvalue',showvalue);
  console.log('O',o);
  showvalue.innerHTML = o.value
    } else if (xhr.readyState === 4) {
     console.log("could not fetch the data");
     }
        });
  xhr.send();
}

setInterval(()=>{
  
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/", true); 
  xhr.addEventListener("readystatechange", () => {
    if (xhr.readyState === 4 && xhr.status === 200) {
    console.log(xhr.responseText);
    var o =  JSON.parse(xhr.responseText);
    console.log('O',o);
    
    var uptime = document.getElementById("uptime"); 
    uptime.innerHTML = o.uptime 

    var d1 = document.getElementById("d1"); 
    d1.innerHTML = o.d1 

var d2 = document.getElementById("d2"); 
    d2.innerHTML = o.d2 

var d3 = document.getElementById("d3"); 
    d3.innerHTML = o.d3
var d4 = document.getElementById("d4"); 
    d4.innerHTML = o.d4
var d5 = document.getElementById("d5"); 
    d5.innerHTML = o.d5 

var d6 = document.getElementById("d6"); 
    d6.innerHTML = o.d6
var d7 = document.getElementById("d7"); 
    d7.innerHTML = o.d7
var d8 = document.getElementById("d8"); 
    d8.innerHTML = o.d8
var t = document.getElementById("t"); 
    t.innerHTML = o.t
    var h = document.getElementById("h"); 
    h.innerHTML = o.h
    var version = document.getElementById("version"); 
    version.innerHTML = o.version
    var heap = document.getElementById("heap"); 
    heap.innerHTML = o.heap
       var name = document.getElementById("name"); 
    name.innerHTML = o.name

    } else if (xhr.readyState === 4) {
     console.log("could not fetch the data");
     }
    });
  xhr.send();
  console.log('Call refresh');
}
, 500); // 3000 milliseconds = 3 seconds
</script>
  </head><body>
 <table id="customers">
  <tr>
  <td>Config</td><td>value</td><td>Set</td><td>#</td><td>x</td>
  </tr>
  %CONFIG%
 </table>
<hr>
New Config <input id=newconfigname> <input id=newvalue> <button  id=btn onClick="add()">add </button>
<hr>
<button id=btn onClick="deleteallconfig()">Reset Config</button>
<table id="customers">
<tr>
  <td>version</td><td><label id="name">0</label></td>
    </tr>
<tr>
  <td>version</td><td><label id="version">0</label></td>
    </tr><tr> 
    <tr>
  <td>heap</td><td><label id="heap">0</label></td>
    </tr><tr> 
 <tr>
  <td>uptime</td><td><label id="uptime">0</label></td>
    </tr><tr> 
  <td>D1</td><td><label id="d1">0</label></td>
    </tr><tr> 
  <td>D2</td><td><label id="d2">0</label></td>
    </tr><tr> 
  <td>D3</td><td><label id="d3">0</label></td>
    </tr><tr> 
  <td>D4</td><td><label id="d4">0</label></td>
   </tr><tr> 
  <td>D5</td><td><label id="d5">0</label></td>
    </tr><tr> 
  <td>D6</td><td><label id="d6">0</label></td>
    </tr><tr> 
  <td>D7</td><td><label id="d7">0</label></td>
   </tr><tr> 
  <td>D8</td><td><label id="d8">0</label></td>
    </tr>
    <tr>
  <td>a0</td><td><label id="a0">0</label></td>
     </tr><tr>  
  <td>t</td><td><label id="t">0</label></td>
  </tr>
       <tr>
  <td>h</td><td><label id="h">0</label></td>
  </tr>

  </tr>
 </table>
</body></html>)rawliteral";
#endif