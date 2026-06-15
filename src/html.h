#ifndef HTML_H
#define HTML_H
#include <Arduino.h>

const char addjob_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<style>
#customers{border-collapse:collapse;width:100%%}#customers td,#customers th{border:1px solid #ddd;padding:6px}#customers th{background:#4CAF50;color:#fff}#btn,.button{background:#4CAF50;color:#fff;border:none;padding:8px;cursor:pointer;font-size:14px}
body{font-family:Arial;max-width:600px;margin:auto;text-align:center}
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
<button id=btn onClick="reset()">Reset </button>
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
  <a href='/logs'>logs</a>
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
<meta charset="UTF-8">
<title>Config</title>
<meta name="viewport" content="width=device-width,initial-scale=1">
<style>
*{box-sizing:border-box}
body{font-family:Segoe UI,Roboto,Arial,sans-serif;margin:0;padding:16px;font-size:14px;color:#1a2b3c;background:linear-gradient(160deg,#e8f4fc 0%%,#f5f7fa 45%%,#eef2ff 100%%);min-height:100vh}
.wrap{max-width:720px;margin:0 auto}
.hdr{background:linear-gradient(135deg,#1b5e20 0%%,#2e7d32 55%%,#43a047 100%%);color:#fff;padding:18px 20px;border-radius:14px 14px 0 0;box-shadow:0 4px 14px rgba(27,94,32,.25)}
.hdr h1{margin:0;font-size:1.35rem;font-weight:600;letter-spacing:.02em}
.hdr p{margin:6px 0 0;opacity:.92;font-size:.85rem}
.nav{display:flex;flex-wrap:wrap;gap:8px;margin:14px 0 0}
.nav a{display:inline-block;padding:7px 14px;background:rgba(255,255,255,.18);color:#fff;text-decoration:none;border-radius:999px;font-size:.82rem;border:1px solid rgba(255,255,255,.35);transition:background .15s}
.nav a:hover{background:rgba(255,255,255,.32)}
.card{background:#fff;border-radius:0 0 14px 14px;box-shadow:0 8px 28px rgba(15,40,60,.08);padding:16px 18px 20px;margin-bottom:16px;border:1px solid rgba(0,0,0,.06)}
.card+.card{border-radius:14px;margin-top:0}
.sec{margin:0 0 10px;font-size:1rem;font-weight:600;color:#1b5e20;display:flex;align-items:center;gap:8px}
.sec:before{content:'';width:4px;height:18px;background:#43a047;border-radius:2px}
.tbl-wrap{overflow-x:auto;border-radius:10px;border:1px solid #e3eaf0;margin:8px 0 14px}
table{width:100%%;border-collapse:collapse;background:#fff;font-size:.82rem}
#customers td,#customers th,#status td,#status th{padding:10px 12px;text-align:left;border-bottom:1px solid #edf1f5;vertical-align:middle}
#customers tr:first-child td,#status tr:first-child td{background:linear-gradient(90deg,#2e7d32,#388e3c);color:#fff;font-weight:600;font-size:.78rem;text-transform:uppercase;letter-spacing:.04em;border:none}
#customers tr:not(:first-child):nth-child(even),#status tr:not(:first-child):nth-child(even){background:#f8fafb}
#customers tr:not(:first-child):hover,#status tr:not(:first-child):hover{background:#e8f5e9}
#customers td:first-child,#status td:first-child{font-weight:600;color:#37474f;white-space:nowrap}
.desc{font-size:.75rem;color:#607d8b;line-height:1.35;max-width:220px}
label.val{display:inline-block;padding:4px 8px;background:#f1f5f9;border-radius:6px;color:#1565c0;font-family:Consolas,monospace;font-size:.78rem;word-break:break-all;max-width:180px}
input[type=text]{padding:8px 10px;border:1px solid #cfd8dc;border-radius:8px;font-size:.82rem;min-width:100px;transition:border-color .15s,box-shadow .15s;background:#fafbfc}
input[type=text]:focus{outline:none;border-color:#43a047;box-shadow:0 0 0 3px rgba(67,160,71,.15);background:#fff}
.toolbar{display:flex;flex-wrap:wrap;align-items:center;gap:10px;padding:14px;background:linear-gradient(180deg,#f8fafc,#f1f5f9);border-radius:10px;border:1px dashed #c5d0db}
.toolbar span{font-weight:600;color:#455a64;font-size:.85rem}
.btn{display:inline-block;padding:8px 14px;border:none;border-radius:8px;cursor:pointer;font-size:.8rem;font-weight:600;transition:transform .1s,box-shadow .15s;margin:0}
.btn:active{transform:scale(.97)}
.btn-add{background:linear-gradient(180deg,#43a047,#2e7d32);color:#fff;box-shadow:0 2px 8px rgba(46,125,50,.35)}
.btn-add:hover{box-shadow:0 4px 12px rgba(46,125,50,.45)}
.btn-set{background:#1976d2;color:#fff;box-shadow:0 1px 4px rgba(25,118,210,.3)}
.btn-set:hover{background:#1565c0}
.btn-rm{background:#fff;color:#c62828;border:1px solid #ef9a9a}
.btn-rm:hover{background:#ffebee;border-color:#e57373}
.btn-reset{background:#fff;color:#6d4c41;border:1px solid #bcaaa4}
.btn-reset:hover{background:#efebe9}
.help{font-size:.78rem;color:#546e7a;line-height:1.55;margin:0;padding:14px 16px;background:#f8fafc;border-left:4px solid #81c784;border-radius:0 10px 10px 0}
@media(max-width:520px){body{padding:10px}#customers td,#customers th,#status td,#status th{padding:8px 6px;font-size:.75rem}input[type=text]{min-width:70px;max-width:100px}}
</style>

<script>
function deleteallconfig()
{
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/resetconfig", true); 
    xhr.send();
}
function reset()
{
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/restart", true); 
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
     row.innerHTML = "<td>"+o.setconfig+"</td><td class=\"desc\">Custom key</td><td><label class=\"val\">"+o.value+"</label></td><td><input value=\""+o.value+"\"></td><td></td><td></td>";
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

setInterval(function(){
  var x=new XMLHttpRequest();
  x.open("GET","/",true);
  x.onload=function(){
    if(x.status!=200)return;
    var o=JSON.parse(x.responseText);
    var m={name:"name",version:"version",heap:"heap",uptime:"uptime",d1:"d1",d2:"d2",d3:"d3",d4:"d4",d5:"d5",d6:"d6",d7:"d7",d8:"d8",a0:"a0",t:"t",h:"h",flow:"flow",message:"message",errormessage:"errormessage",timestamp:"localtimestamp",fd:"fulldate"};
    for(var i in m){var e=document.getElementById(i);if(e&&o[m[i]]!=null)e.innerHTML=o[m[i]];}
  };
  x.send();
},500);
</script>
</head><body>
<div class="wrap">
<div class="hdr">
<h1>Device configuration</h1>
<p>Manage settings and monitor live status</p>
<div class="nav"><a href="/">Status JSON</a><a href="/logs">Logs</a></div>
</div>
<div class="card">
<div class="sec">Parameters</div>
<div class="tbl-wrap">
<table id="customers">
<tr><td>Parameter</td><td>Description</td><td>Value</td><td>New value</td><td></td><td></td></tr>
%CONFIG%
</table>
</div>
<div class="toolbar">
<span>Add key</span>
<input id=newconfigname placeholder="configname" type="text">
<input id=newvalue placeholder="value" type="text">
<button class="btn btn-add" type="button" onClick="add()">+ Add</button>
<button class="btn btn-reset" type="button" onClick="deleteallconfig()">Reset all</button>
</div>
</div>
<div class="card">
<div class="sec">Live status</div>
<div class="tbl-wrap">
<table id="status"></table>
</div>
</div>
<script>
(function(){
 var k=["name","version","heap","uptime","d1","d2","d3","d4","d5","d6","d7","d8","a0","t","h","flow","message","errormessage","timestamp","fd"],s=document.getElementById("status"),h="<tr><td>Field</td><td>Value</td></tr>";
 for(var i=0;i<k.length;i++)h+="<tr><td>"+k[i]+"</td><td><label class=\"val\" id=\""+k[i]+"\">-</label></td></tr>";
 s.innerHTML=h;
})();
</script>
<p class="help"><b>Tip:</b> Each parameter shows a short description. GPIO mode/init changes need device restart. Changing <code>logslots</code> clears the log buffer.</p>
</div>
</body></html>)rawliteral";
const char logs_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
<meta charset="UTF-8">
<title>Logs</title>
<meta name="viewport" content="width=device-width,initial-scale=1">
<style>
body{font-family:Arial,sans-serif;font-size:12px;margin:8px;max-width:640px}
table{border-collapse:collapse;width:100%%}td,th{border:1px solid #ccc;padding:3px 5px;font-size:11px;text-align:left}
th{background:#4CAF50;color:#fff}.c{color:#060}.e{color:#c00}.o{color:#06c}.s{color:#684}.h{color:#609}.t{color:#666}
#btn{font-size:11px;padding:3px 8px;margin:4px 0}
#refresh{font-size:11px;color:#666}
</style>
</head><body>
<h3>Device logs</h3>
<a href="/setconfigwww">config</a> | <a href="/">status</a>
<p id=info></p>
<p id=refresh>auto refresh every 3s</p>
<table><tr><th>uptime s</th><th>type</th><th>code</th><th>message</th></tr>
<tbody id=rows></tbody></table>
<button id=btn onclick="clearLogs()">clear</button>
<script>
var refreshSec=3;
function logClass(t){if(t==='checkin')return 'c';if(t==='ota')return 'o';if(t==='soi')return 's';if(t==='sht')return 'h';if(t==='task')return 't';return 'e';}
function loadLogs(){
 fetch('/logs.json').then(function(r){return r.json()}).then(function(o){
  var h='',i,logs=Array.isArray(o.logs)?o.logs:[];
  for(i=0;i<logs.length;i++){
   var x=logs[i];
   h+='<tr><td>'+x.t+'</td><td class="'+logClass(x.type)+'">'+x.type+'</td><td>'+x.code+'</td><td>'+x.msg+'</td></tr>';
  }
  document.getElementById('rows').innerHTML=h||'<tr><td colspan=4>(empty)</td></tr>';
  var cnt=(o.count!=null)?o.count:logs.length;
  var mx=(o.max!=null)?o.max:'?';
  var hp=(o.heap!=null)?o.heap:'?';
  var ver=(o.version!=null&&o.version!=='')?o.version:'?';
  document.getElementById('info').innerHTML='fw '+ver+' &mdash; count '+cnt+' / '+mx+' &mdash; heap '+hp+' B &mdash; updated '+new Date().toLocaleTimeString();
 }).catch(function(){
  document.getElementById('info').innerHTML='load failed';
  document.getElementById('rows').innerHTML='<tr><td colspan=4>load failed</td></tr>';
 });
}
function clearLogs(){fetch('/logs/clear').then(loadLogs);}
loadLogs();
setInterval(loadLogs, refreshSec*1000);
</script>
</body></html>)rawliteral";
#endif