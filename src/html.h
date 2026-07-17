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
<title>Sensor Telemetry</title>
<meta name="viewport" content="width=device-width,initial-scale=1">
<style>
*{box-sizing:border-box}
body{font-family:Segoe UI,Roboto,Arial,sans-serif;margin:0;padding:20px;font-size:14px;color:#e2e8f0;background:#0a0e1a;min-height:100vh}
.page{max-width:1200px;margin:0 auto}
.topbar{margin-bottom:20px}
.title-row{display:flex;flex-wrap:wrap;align-items:center;gap:12px;margin-bottom:18px}
.title-row h1{margin:0;font-size:1.5rem;font-weight:600;color:#22d3ee;letter-spacing:.01em}
.badge{display:inline-flex;align-items:center;gap:6px;padding:4px 12px;border-radius:999px;font-size:.75rem;font-weight:600}
.badge.online{background:rgba(34,197,94,.15);color:#4ade80;border:1px solid rgba(74,222,128,.35)}
.badge.offline{background:rgba(239,68,68,.15);color:#f87171;border:1px solid rgba(248,113,113,.35)}
.badge:before{content:'';width:8px;height:8px;border-radius:50%;background:currentColor}
.btn-logs{margin-left:auto;padding:8px 16px;background:#7c3aed;color:#fff;text-decoration:none;border-radius:8px;font-size:.82rem;font-weight:600}
.btn-logs:hover{background:#6d28d9}
.meta-row{display:grid;grid-template-columns:repeat(4,1fr);gap:16px;padding:16px 18px;background:#121826;border:1px solid #1e293b;border-radius:12px}
.meta-row label{display:block;font-size:.65rem;color:#64748b;letter-spacing:.06em;margin-bottom:4px}
.meta-row span{font-size:1.05rem;font-weight:600;color:#f1f5f9}
.main-grid{display:grid;grid-template-columns:1fr 1.4fr;gap:20px;align-items:start}
.panel{background:#121826;border:1px solid #1e293b;border-radius:12px;padding:18px}
.panel h2{margin:0 0 14px;font-size:1rem;font-weight:600;color:#f8fafc;display:flex;align-items:center;gap:8px}
.panel h2:before{content:'';width:4px;height:18px;background:#3b82f6;border-radius:2px}
.tel-grid{display:grid;grid-template-columns:1fr 1fr;gap:12px}
.tel-card{position:relative;background:#151c2e;border:1px solid #1e293b;border-left:3px solid #3b82f6;border-radius:10px;padding:14px 14px 12px;min-height:88px}
.tel-card .lbl{font-size:.62rem;color:#64748b;letter-spacing:.05em;margin-bottom:6px}
.tel-card .val{font-size:1.35rem;font-weight:700;color:#f8fafc;line-height:1.2}
.tel-card .ico{position:absolute;top:12px;right:12px;font-size:1.1rem;opacity:.85}
.tbl-wrap{overflow-x:auto;border-radius:10px;border:1px solid #1e293b}
table{width:100%;border-collapse:collapse;font-size:.8rem}
#customers thead th{background:#0f172a;color:#64748b;font-weight:600;font-size:.68rem;text-transform:uppercase;letter-spacing:.05em;padding:10px 8px;text-align:left;border-bottom:1px solid #1e293b}
#customers tbody td{padding:8px;border-bottom:1px solid #1e293b;vertical-align:middle;color:#cbd5e1}
#customers tbody tr:hover{background:rgba(59,130,246,.06)}
#customers tbody td:first-child{font-weight:600;color:#e2e8f0;white-space:nowrap;font-family:Consolas,monospace;font-size:.78rem}
.cfg-val{color:#38bdf8;font-family:Consolas,monospace;font-size:.78rem;word-break:break-all}
input[type=text]{width:100%;padding:7px 10px;border:1px solid #334155;border-radius:6px;font-size:.78rem;background:#0f172a;color:#e2e8f0;min-width:80px}
input[type=text]:focus{outline:none;border-color:#3b82f6;box-shadow:0 0 0 2px rgba(59,130,246,.25)}
.toolbar{display:flex;flex-wrap:wrap;align-items:center;gap:10px;margin-top:14px;padding:12px;background:#0f172a;border-radius:8px;border:1px dashed #334155}
.toolbar span{font-weight:600;color:#94a3b8;font-size:.82rem}
.btn{display:inline-block;padding:6px 12px;border:none;border-radius:6px;cursor:pointer;font-size:.75rem;font-weight:600;margin:0}
.btn-add{background:#2563eb;color:#fff}
.btn-add:hover{background:#1d4ed8}
.btn-set{background:#2563eb;color:#fff;padding:5px 14px}
.btn-set:hover{background:#1d4ed8}
.btn-rm{background:transparent;color:#f87171;border:1px solid #7f1d1d;padding:5px 12px}
.btn-rm:hover{background:rgba(239,68,68,.12)}
.btn-reset{background:transparent;color:#fca5a5;border:1px solid #7f1d1d;padding:8px 18px}
.btn-reset:hover{background:rgba(239,68,68,.12)}
.footer-actions{margin-top:16px;display:flex;justify-content:flex-end}
.navlinks{margin-top:10px;font-size:.78rem}
.navlinks a{color:#64748b;margin-right:12px;text-decoration:none}
.navlinks a:hover{color:#38bdf8}
.sub-h{margin:18px 0 10px;font-size:.9rem;font-weight:600;color:#f8fafc;display:flex;align-items:center;gap:8px}
.sub-h:before{content:'';width:4px;height:16px;background:#3b82f6;border-radius:2px}
#status{width:100%;border-collapse:collapse;font-size:.78rem}
#status td{padding:7px 8px;border-bottom:1px solid #1e293b;vertical-align:top}
#status td:first-child{color:#64748b;font-weight:600;white-space:nowrap;width:42%}
#status .sv{color:#38bdf8;font-family:Consolas,monospace;word-break:break-all}
@media(max-width:900px){.main-grid{grid-template-columns:1fr}.meta-row{grid-template-columns:repeat(2,1fr)}.tel-grid{grid-template-columns:1fr}}
@media(max-width:520px){body{padding:12px}.meta-row{grid-template-columns:1fr}.title-row h1{font-size:1.2rem}.btn-logs{margin-left:0}}
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
function escHtml(s){
  return String(s).replace(/&/g,'&amp;').replace(/"/g,'&quot;').replace(/'/g,'&#39;').replace(/</g,'&lt;');
}
function makeConfigRow(k,v,desc){
  k=String(k);
  v=String(v);
  desc=desc||'';
  var tr=document.createElement('tr');
  var td0=document.createElement('td'); td0.textContent=k; if(desc)td0.title=desc;
  var tdDesc=document.createElement('td'); tdDesc.textContent=desc;
  var td1=document.createElement('td');
  var lbl=document.createElement('span'); lbl.className='cfg-val'; lbl.id=k+'value'; lbl.textContent=v;
  td1.appendChild(lbl);
  var td2=document.createElement('td');
  var inp=document.createElement('input'); inp.id=k; inp.type='text'; inp.value=v;
  td2.appendChild(inp);
  var td3=document.createElement('td');
  var btnSave=document.createElement('button');
  btnSave.className='btn btn-set'; btnSave.type='button'; btnSave.textContent='Set';
  btnSave.onclick=function(){setvalue(this,k,'');};
  td3.appendChild(btnSave);
  var td4=document.createElement('td');
  var btnDel=document.createElement('button');
  btnDel.className='btn btn-rm'; btnDel.type='button'; btnDel.textContent='Remove';
  btnDel.onclick=function(){remove(k);};
  td4.appendChild(btnDel);
  tr.appendChild(td0); tr.appendChild(tdDesc); tr.appendChild(td1); tr.appendChild(td2);
  tr.appendChild(td3); tr.appendChild(td4);
  return tr;
}
function insertConfigRow(k,v,desc){
  var rows=document.getElementById('cfgrows');
  if(!rows)return;
  rows.appendChild(makeConfigRow(k,v,desc));
}
function add()
{
  var xhr = new XMLHttpRequest();
  var input = document.getElementById('newconfigname');
  var value = document.getElementById('newvalue');
  xhr.open("GET", "/setconfig?configname="+encodeURIComponent(input.value)+"&value="+encodeURIComponent(value.value), true); 
  xhr.addEventListener("readystatechange", () => {
     console.log(xhr.readystate);
    if (xhr.readyState === 4 && xhr.status === 200) {
     console.log(xhr.responseText);
     var o =  JSON.parse(xhr.responseText);
     insertConfigRow(o.setconfig,o.value,'Custom config key');
     input.value='';
     value.value='';
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

function loadConfigTable(){
  var rows=document.getElementById('cfgrows');
  if(!rows)return;
  fetch('/config').then(function(r){
    if(!r.ok)throw new Error('config '+r.status);
    return r.text();
  }).then(function(text){
    if(!text||!text.trim())throw new Error('empty config response');
    var cfg=JSON.parse(text);
    return fetch('/configdesc.json').then(function(r){
      return r.ok?r.json():{};
    }).catch(function(){return{};}).then(function(desc){
      var keys=Object.keys(cfg).sort(),i,k,v;
      rows.innerHTML='';
      for(i=0;i<keys.length;i++){
        k=keys[i];
        v=String(cfg[k]);
        rows.appendChild(makeConfigRow(k,v,desc[k]));
      }
    });
  }).catch(function(e){
    rows.innerHTML='<tr><td colspan="6">Failed to load config ('+escHtml(String(e))+' )</td></tr>';
  });
}
function fmtVal(v,unit){
  if(v===null||v===undefined||v==='')return '—';
  if(typeof v==='number'&&isNaN(v))return '—';
  return unit?(v+' '+unit):String(v);
}
function buildTelemetry(){
  var g=document.getElementById('telGrid');
  if(!g)return;
  var items=[
    {k:'t',lbl:'TEMPERATURE',u:'°C',i:'🌡'},
    {k:'h',lbl:'HUMIDITY',u:'%',i:'💧'},
    {k:'psi',lbl:'PRESSURE',u:'psi',i:'🎈'},
    {k:'volts',lbl:'VOLTAGE',u:'V',i:'⚡'},
    {k:'currentflow',lbl:'FLOW RATE',u:'Hz',i:'🌊'},
    {k:'totalflow',lbl:'TOTAL FLOW',u:'',i:'📊'},
    {k:'pm25',lbl:'PM 2.5',u:'',i:'🌫'},
    {k:'signal',lbl:'WIFI RSSI',u:'dBm',i:'📶'}
  ],h='',i;
  for(i=0;i<items.length;i++){
    h+='<div class="tel-card"><div class="lbl">'+items[i].lbl+'</div><div class="val" id="tel_'+items[i].k+'">—</div><div class="ico">'+items[i].i+'</div></div>';
  }
  g.innerHTML=h;
}
function buildStatusTable(){
  var s=document.getElementById('statusRows');
  if(!s)return;
  var fields=[
    ['name','name'],['version','version'],['heap','heap'],['uptime','uptime'],
    ['d1','d1'],['d2','d2'],['d3','d3'],['d4','d4'],['d5','d5'],['d6','d6'],['d7','d7'],['d8','d8'],
    ['a0','a0'],['t','t'],['h','h'],['flow','flow'],
    ['message','message'],['errormessage','errormessage'],
    ['timestamp','localtimestamp'],['fd','fulldate']
  ],h='',i;
  for(i=0;i<fields.length;i++){
    h+='<tr><td>'+fields[i][0]+'</td><td><span class="sv" id="lv_'+fields[i][0]+'">—</span></td></tr>';
  }
  s.innerHTML=h;
}
function updateStatus(o){
  var el,n=document.getElementById('devName'),v=document.getElementById('devVersion');
  var u=document.getElementById('devUptime'),h=document.getElementById('devHeap');
  var b=document.getElementById('onlineBadge');
  if(n)n.textContent=('name' in o&&o.name!=='')?o.name:(('description' in o)?o.description:'—');
  if(v&&'version' in o)v.textContent=o.version;
  if(u&&'uptime' in o)u.textContent=o.uptime;
  if(h&&'heap' in o)h.textContent=o.heap;
  if(b){b.className='badge online';b.textContent='Online';}
  var tel=[
    {k:'t',u:'°C'},{k:'h',u:'%'},{k:'psi',u:'psi'},{k:'volts',u:'V'},
    {k:'currentflow',u:'Hz'},{k:'totalflow',u:''},{k:'pm25',u:''},{k:'signal',u:'dBm'}
  ],i;
  for(i=0;i<tel.length;i++){
    el=document.getElementById('tel_'+tel[i].k);
    if(el&&tel[i].k in o)el.textContent=fmtVal(o[tel[i].k],tel[i].u);
  }
  var live=[
    ['name','name'],['version','version'],['heap','heap'],['uptime','uptime'],
    ['d1','d1'],['d2','d2'],['d3','d3'],['d4','d4'],['d5','d5'],['d6','d6'],['d7','d7'],['d8','d8'],
    ['a0','a0'],['t','t'],['h','h'],['flow','flow'],
    ['message','message'],['errormessage','errormessage'],
    ['timestamp','localtimestamp'],['fd','fulldate']
  ];
  for(i=0;i<live.length;i++){
    el=document.getElementById('lv_'+live[i][0]);
    if(el&&live[i][1] in o)el.textContent=String(o[live[i][1]]);
  }
}
function pollStatus(){
  var x=new XMLHttpRequest();
  x.open('GET','/',true);
  x.onload=function(){
    if(x.status!==200){setOffline();return;}
    try{updateStatus(JSON.parse(x.responseText));}
    catch(e){setOffline();}
  };
  x.onerror=setOffline;
  x.send();
}
function setOffline(){
  var b=document.getElementById('onlineBadge');
  if(b){b.className='badge offline';b.textContent='Offline';}
}
function initPage(){
  buildTelemetry();
  buildStatusTable();
  loadConfigTable();
  pollStatus();
  setInterval(pollStatus,3000);
}
if(document.readyState==='loading')document.addEventListener('DOMContentLoaded',initPage);
else initPage();
</script>
</head><body>
<div class="page">
<header class="topbar">
<div class="title-row">
<h1>ESP8266 Sensor Telemetry</h1>
<span class="badge online" id="onlineBadge">Online</span>
<a class="btn-logs" href="/logs">View Full Logs</a>
</div>
<div class="meta-row">
<div><label>DEVICE NAME</label><span id="devName">—</span></div>
<div><label>VERSION</label><span id="devVersion">—</span></div>
<div><label>UPTIME</label><span id="devUptime">—</span></div>
<div><label>FREE HEAP</label><span id="devHeap">—</span></div>
</div>
<div class="navlinks"><a href="/">Status JSON</a><a href="/ping">Ping test</a></div>
</header>
<div class="main-grid">
<section class="panel telemetry">
<h2>Live Telemetry</h2>
<div class="tel-grid" id="telGrid"></div>
<h2 class="sub-h">All Live Values</h2>
<div class="tbl-wrap">
<table id="status"><tbody id="statusRows"></tbody></table>
</div>
</section>
<section class="panel config">
<h2>System Configuration</h2>
<div class="tbl-wrap">
<table id="customers">
<thead><tr><th>Config</th><th>Description</th><th>Value</th><th>Edit</th><th>Set</th><th>Remove</th></tr></thead>
<tbody id="cfgrows"><tr><td colspan="6">Loading config...</td></tr></tbody>
</table>
</div>
<div class="toolbar">
<span>Add key</span>
<input id=newconfigname placeholder="configname" type="text">
<input id=newvalue placeholder="value" type="text">
<button class="btn btn-add" type="button" onClick="add()">+ Add</button>
</div>
<div class="footer-actions">
<button class="btn btn-reset" type="button" onClick="deleteallconfig()">Reset all</button>
</div>
</section>
</div>
</div>
</body></html>)rawliteral";
const char logs_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
<meta charset="UTF-8">
<title>Logs</title>
<meta name="viewport" content="width=device-width,initial-scale=1">
<style>
body{font-family:Arial,sans-serif;font-size:12px;margin:8px;max-width:640px}
table{border-collapse:collapse;width:100%}td,th{border:1px solid #ccc;padding:3px 5px;font-size:11px;text-align:left}
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