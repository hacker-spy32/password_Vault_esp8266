// =============================================================
//  portal_html.h  —  WiFi vault-manager web page (served from flash)
//
//  Kept in a .h (NOT a .ino) on purpose: Arduino's .ino auto-prototype
//  generator mis-parses the JavaScript "function foo(){...}" patterns
//  inside this raw string and emits bogus C++ prototypes. Header files
//  are not scanned by that generator, so the page lives here.
// =============================================================
#pragma once

static const char PORTAL_HTML[] PROGMEM = R"HTML(<!DOCTYPE html><html><head>
<meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1,viewport-fit=cover">
<title>SecureKey Portal</title><style>
:root{--bg:#0b0e13;--card:#161b24;--card2:#1c222d;--line:#272e3a;--txt:#eef1f6;--mut:#8a93a4;--accent:#4d9fff;--ok:#36d67a;--err:#ff5f6d;--warn:#f5a623}
*{box-sizing:border-box;-webkit-tap-highlight-color:transparent}
body{margin:0;background:radial-gradient(1000px 500px at 50% -10%,#15243b,#0b0e13) fixed;color:var(--txt);font-family:-apple-system,Segoe UI,Roboto,sans-serif}
.wrap{max-width:580px;margin:0 auto;padding:16px}
h1{font-size:20px;margin:0}.mut{color:var(--mut);font-size:13px}
input,textarea{width:100%;background:#0c0f15;color:var(--txt);border:1px solid var(--line);border-radius:10px;padding:11px;font-size:15px;font-family:inherit;outline:none}
input:focus,textarea:focus{border-color:var(--accent)}
button{border:0;border-radius:10px;padding:11px 14px;font-size:14px;font-weight:700;cursor:pointer;background:linear-gradient(140deg,#4d9fff,#2d77df);color:#04101f;transition:transform .1s,opacity .1s}
button:active{transform:scale(0.98)}
button.ghost{background:#0c0f15;color:var(--txt);border:1px solid var(--line)}
button.danger{background:#2a1416;color:var(--err);border:1px solid #4a2226}
.card{background:var(--card);border:1px solid var(--line);border-radius:14px;padding:14px;margin-bottom:12px}
.row{display:flex;gap:8px;align-items:center}
.head{display:flex;justify-content:space-between;align-items:center;margin:6px 0 14px}
.bar{display:flex;gap:8px;margin-bottom:12px}.bar input{flex:1}
.entry .t{font-size:16px;font-weight:700}.entry .u{color:var(--mut);font-size:13px;margin-top:2px}
.pw{font-family:ui-monospace,Consolas,monospace;background:#0c0f15;border:1px solid var(--line);border-radius:8px;padding:8px;margin-top:8px;display:flex;justify-content:space-between;align-items:center;gap:8px}
.pw span{overflow:hidden;text-overflow:ellipsis;white-space:nowrap}
.acts{display:flex;gap:8px;margin-top:10px}.acts button{flex:1;padding:9px}
.mini{padding:6px 9px;font-size:12px;border-radius:7px}
.tabs{display:flex;gap:6px;background:#0c0f15;border:1px solid var(--line);border-radius:12px;padding:4px;margin-bottom:14px}
.tab-btn{flex:1;background:transparent;color:var(--mut);border:0;border-radius:8px;padding:10px;font-weight:600;font-size:14px;cursor:pointer;transition:all .2s}
.tab-btn.active{background:var(--card);color:var(--txt);box-shadow:0 2px 8px rgba(0,0,0,0.3)}
.badge{padding:3px 8px;border-radius:6px;font-size:11px;font-weight:700;display:inline-block}
.badge-active{background:rgba(54,214,122,0.15);color:var(--ok);border:1px solid rgba(54,214,122,0.3)}
.badge-used{background:rgba(255,95,109,0.15);color:var(--err);border:1px solid rgba(255,95,109,0.3)}
.pin-display{font-family:ui-monospace,Consolas,monospace;font-size:22px;font-weight:800;letter-spacing:6px;color:var(--accent);background:#0c0f15;padding:8px 12px;border-radius:8px;border:1px solid var(--line);display:inline-block}
.tog{position:relative;display:inline-block;width:46px;height:24px}.tog-cb{opacity:0;width:0;height:0}
.tog-sw{position:absolute;inset:0;background:#0c0f15;border:1px solid var(--line);border-radius:24px;cursor:pointer;transition:background .2s}
.tog-sw:before{content:"";position:absolute;height:16px;width:16px;left:3px;top:3px;background:var(--mut);border-radius:50%;transition:.2s}
.tog-cb:checked+.tog-sw{background:var(--accent)}.tog-cb:checked+.tog-sw:before{transform:translateX(22px);background:#04101f}
.overlay{position:fixed;inset:0;background:rgba(5,8,13,.85);display:flex;align-items:center;justify-content:center;padding:18px;z-index:9}
.modal{background:var(--card2);border:1px solid var(--line);border-radius:16px;padding:18px;width:100%;max-width:420px}
label{display:block;font-size:12px;color:var(--mut);margin:10px 2px 5px;font-weight:600}
.gate input{letter-spacing:8px;text-align:center;font-size:24px}
.hidden{display:none}.toast{position:fixed;bottom:16px;left:50%;transform:translateX(-50%);background:#1c222d;border:1px solid var(--line);border-radius:10px;padding:10px 16px;font-size:13px;z-index:20}
</style></head><body>

<div id="gate" class="overlay"><div class="modal">
 <h1>SecureKey</h1><p class="mut">Enter the 6-digit code shown on your device.</p>
 <input id="gcode" class="gate" inputmode="numeric" maxlength="6" placeholder="------">
 <button style="width:100%;margin-top:14px" onclick="unlock()">Unlock</button>
 <div id="gerr" class="mut" style="color:var(--err);margin-top:8px"></div>
</div></div>

<div id="app" class="wrap hidden">
 <div class="head">
  <h1>SecureKey</h1>
  <div class="row">
   <button class="ghost mini" onclick="openSettings()">Settings</button>
  </div>
 </div>

 <!-- Navigation Tabs -->
 <div class="tabs">
  <button id="tabBtnVault" class="tab-btn active" onclick="switchTab('vault')">🔑 Password Vault</button>
  <button id="tabBtnTemp" class="tab-btn" onclick="switchTab('temp')">⏱️ Temp Keypad PINs</button>
 </div>

 <!-- TAB 1: VAULT -->
 <div id="tabVault">
  <div class="bar">
   <input id="q" placeholder="Search..." oninput="render()">
   <button class="ghost" onclick="openImport()">Import</button>
   <button class="ghost mini" onclick="doExport()">Export</button>
   <button class="mini" onclick="openAdd()">+ Add Entry</button>
  </div>
  <div id="list"></div>
 </div>

 <!-- TAB 2: TEMP KEYPAD PINS -->
 <div id="tabTemp" class="hidden">
  <div class="card" style="margin-bottom:14px">
   <div style="display:flex;justify-content:space-between;align-items:center">
    <div>
     <div style="font-weight:700;font-size:16px">Keypad Passcodes</div>
     <div class="mut" style="margin-top:2px">Temp 4-digit PINs for physical device unlock. Once typed on the keypad, access expires.</div>
    </div>
   </div>
   <div class="row" style="margin-top:12px">
    <button onclick="openAddTemp()"><span style="font-size:16px">+</span> Add Temp PIN</button>
    <button class="ghost" onclick="clearExpiredTemp()">Clear Expired</button>
   </div>
  </div>
  <div id="tempList"></div>
 </div>

 <p class="mut" style="text-align:center;margin-top:18px">Nothing leaves this device WiFi. Turn off Web UI when done.</p>
</div>

<!-- VAULT ADD/EDIT MODAL -->
<div id="modal" class="overlay hidden"><div class="modal">
 <h1 id="mtitle">Add</h1><input type="hidden" id="mid">
 <label>Title</label><input id="mt" placeholder="YouTube">
 <label>Username</label><input id="mu" placeholder="you@example.com">
 <label>Password</label>
 <div class="row"><input id="mp"><button class="ghost mini" onclick="genInto()">Gen</button></div>
 <label>URL</label><input id="murl" placeholder="youtube.com">
 <div class="acts" style="margin-top:16px"><button class="ghost" onclick="closeModal()">Cancel</button><button onclick="save()">Save</button></div>
</div></div>

<!-- ADD TEMP PIN MODAL -->
<div id="modalTemp" class="overlay hidden"><div class="modal">
 <h1>New Temp Keypad PIN</h1>
 <p class="mut">Create a 4-digit PIN to access the device's physical keypad. Once entered on the device, it expires immediately.</p>
 
 <label>4-Digit Keypad PIN</label>
 <div class="row">
  <input id="tpPin" class="gate" inputmode="numeric" maxlength="4" placeholder="1234">
  <button class="ghost mini" style="padding:11px" onclick="genTempPinDigits()">🎲 Random</button>
 </div>
 
 <label>Label / Note</label>
 <input id="tpNote" placeholder="Guest Access / Delivery / Friend">
 
 <div class="acts" style="margin-top:16px">
  <button class="ghost" onclick="closeAddTemp()">Cancel</button>
  <button onclick="saveTempPIN()">Create PIN</button>
 </div>
</div></div>

<!-- IMPORT MODAL -->
<div id="imp" class="overlay hidden"><div class="modal">
 <h1>Import Passwords</h1><p class="mut">One per line: title, username, password, url. Chrome/Google CSV supported.</p>
 <textarea id="ibulk" style="min-height:140px;white-space:pre;font-family:ui-monospace,monospace;font-size:13px" placeholder="YouTube,me@x.com,pass,youtube.com"></textarea>
 <div class="acts"><button class="ghost" onclick="closeImport()">Cancel</button><button onclick="doImport()">Import</button></div>
</div></div>

<!-- DELETE CONFIRMATION -->
<div id="confirm" class="overlay hidden"><div class="modal">
 <h1 id="cfTitle">Delete?</h1><p class="mut" id="cfMsg"></p>
 <div class="acts"><button class="ghost" onclick="cfNo()">Cancel</button><button class="danger" onclick="cfYes()">Delete</button></div>
</div></div>

<!-- WEB UI SETTINGS MODAL -->
<div id="webui" class="overlay hidden"><div class="modal">
 <h1>Web UI Settings</h1>
 <p class="mut">Configure WiFi hotspot and auto-start behavior.</p>
 
 <label>WiFi Name (SSID)</label>
 <input id="wuiSSID" placeholder="SecureKey" maxlength="32">
 
 <label>Pair Code</label>
 <input id="wuiCode" placeholder="123456" inputmode="numeric" maxlength="6">
 
 <label>WiFi Password</label>
 <input id="wuiPass" type="password" placeholder="Minimum 8 characters" maxlength="63">
 
 <label style="display:flex;justify-content:space-between;align-items:center;margin-top:14px">
  <span>Run in Background</span>
  <div class="tog"><input type="checkbox" id="wuiBg" class="tog-cb"><label for="wuiBg" class="tog-sw"></label></div>
 </label>
 <p class="mut" style="font-size:12px;margin:6px 2px 0">When ON, Web UI starts automatically on boot. When OFF, start manually from device.</p>
 
 <div class="acts" style="margin-top:16px">
  <button class="ghost" onclick="closeSettings()">Cancel</button>
  <button onclick="saveSettings()">Save</button>
 </div>
</div></div>

<div id="toast" class="toast hidden"></div>

<script>
var CODE="", DATA=[], TEMP_DATA=[], CURRENT_TAB="vault";

function $(i){return document.getElementById(i)}
function toast(m){var t=$("toast");t.textContent=m;t.classList.remove("hidden");setTimeout(function(){t.classList.add("hidden")},1800)}
function api(p){return fetch(p+(p.indexOf("?")<0?"?":"&")+"code="+encodeURIComponent(CODE))}
function post(p,o){o.code=CODE;return fetch(p,{method:"POST",body:new URLSearchParams(o)})}
function esc(s){return(s||"").replace(/[&<>"]/g,function(c){return{"&":"&amp;","<":"&lt;",">":"&gt;","\"":"&quot;"}[c]})}

function unlock(){CODE=$("gcode").value.trim();load(true)}

function load(first){
  api("/list").then(function(r){if(!r.ok)throw 0;return r.json()}).then(function(d){
    DATA=d;
    $("gate").classList.add("hidden");
    $("app").classList.remove("hidden");
    render();
    if(CURRENT_TAB==="temp") loadTempPINs();
  }).catch(function(){if(first)$("gerr").textContent="Wrong code or not connected."});
}

function switchTab(tab){
  CURRENT_TAB=tab;
  if(tab==="vault"){
    $("tabVault").classList.remove("hidden");
    $("tabTemp").classList.add("hidden");
    $("tabBtnVault").classList.add("active");
    $("tabBtnTemp").classList.remove("active");
  }else{
    $("tabVault").classList.add("hidden");
    $("tabTemp").classList.remove("hidden");
    $("tabBtnVault").classList.remove("active");
    $("tabBtnTemp").classList.add("active");
    loadTempPINs();
  }
}

// ========== RANDOMIZERS WITH HTTP FALLBACK ==========
function genPw(){
  var L=16, ch="ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnpqrstuvwxyz23456789!@#$%&*?", s="";
  try{
    if(window.crypto && window.crypto.getRandomValues){
      var a=new Uint32Array(L);
      window.crypto.getRandomValues(a);
      for(var i=0;i<L;i++) s+=ch[a[i]%ch.length];
      return s;
    }
  }catch(e){}
  for(var i=0;i<L;i++) s+=ch[Math.floor(Math.random()*ch.length)];
  return s;
}

function genTempPinDigits(){
  var pin="";
  try{
    if(window.crypto && window.crypto.getRandomValues){
      var a=new Uint32Array(4);
      window.crypto.getRandomValues(a);
      for(var i=0;i<4;i++) pin+=(a[i]%10).toString();
      $("tpPin").value=pin;
      return pin;
    }
  }catch(e){}
  for(var i=0;i<4;i++) pin+=Math.floor(Math.random()*10).toString();
  $("tpPin").value=pin;
  return pin;
}

function genInto(){
  $("mp").value=genPw();
  $("mp").type="text";
}

// ========== VAULT LOGIC ==========
function render(){
  var q=$("q").value.toLowerCase(), L=$("list"); L.innerHTML=""; var n=0;
  DATA.filter(function(e){return !q||(e.title+e.user+e.url).toLowerCase().indexOf(q)>=0}).forEach(function(e){
    n++; var c=document.createElement("div"); c.className="card entry";
    c.innerHTML='<div class="t">'+esc(e.title)+'</div>'+
      (e.user?'<div class="u">'+esc(e.user)+'</div>':'')+
      (e.url?'<div class="u">'+esc(e.url)+'</div>':'')+
      '<div class="pw"><span data-pw="0">&bull;&bull;&bull;&bull;&bull;&bull;&bull;&bull;</span>'+
      '<span style="flex:0 0 auto"><button class="ghost mini" onclick="rev(this)">Show</button> '+
      '<button class="ghost mini" onclick="cp(this)">Copy</button></span></div>'+
      '<div class="acts"><button class="ghost" onclick="openEdit('+e.id+')">Edit</button>'+
      '<button class="danger" onclick="del('+e.id+')">Delete</button></div>';
    c._pw=e.pass; L.appendChild(c);
  });
  if(!n) L.innerHTML='<p class="mut" style="text-align:center">No entries.</p>';
}

function rev(b){var c=b.closest(".entry"),s=c.querySelector("[data-pw]");
  if(s.getAttribute("data-pw")=="0"){s.textContent=c._pw;s.setAttribute("data-pw","1");b.textContent="Hide"}
  else{s.innerHTML="&bull;&bull;&bull;&bull;&bull;&bull;&bull;&bull;";s.setAttribute("data-pw","0");b.textContent="Show"}}
function fallbackCopy(t){
  var ta=document.createElement("textarea");ta.value=t;
  ta.style.position="fixed";ta.style.top="0";ta.style.left="0";ta.style.opacity="0";
  document.body.appendChild(ta);ta.focus();ta.select();
  ta.setSelectionRange(0,t.length);
  var ok=false;try{ok=document.execCommand("copy")}catch(e){}
  document.body.removeChild(ta);
  toast(ok?"Password copied":"Copy blocked — long-press to copy")}
function copyText(t){
  if(window.isSecureContext&&navigator.clipboard&&navigator.clipboard.writeText){
    navigator.clipboard.writeText(t).then(function(){toast("Password copied")},function(){fallbackCopy(t)});
  }else{fallbackCopy(t)}}
function cp(b){copyText(b.closest(".entry")._pw)}

function openAdd(){
  $("mtitle").textContent="Add Entry";
  $("mid").value="";
  $("mt").value="";
  $("mu").value="";
  $("mp").value=genPw(); // Auto pre-fill randomized password
  $("murl").value="";
  $("modal").classList.remove("hidden");
}

function openEdit(id){var e=DATA.find(function(x){return x.id==id});if(!e)return;
  $("mtitle").textContent="Edit Entry";$("mid").value=e.id;$("mt").value=e.title;$("mu").value=e.user;$("mp").value=e.pass;$("murl").value=e.url;$("modal").classList.remove("hidden")}
function closeModal(){$("modal").classList.add("hidden")}
function save(){
  var id=$("mid").value;
  var t=$("mt").value.trim(),u=$("mu").value.trim(),pw=$("mp").value,url=$("murl").value;
  if(!t){toast("Title required");$("mt").focus();return}
  if(!u){toast("Username required");$("mu").focus();return}
  if(!pw){toast("Password required");$("mp").focus();return}
  var o={title:t,user:u,pass:pw,url:url};
  var p;if(id){o.id=id;p=post("/edit",o)}else{p=post("/save",o)}
  p.then(function(){closeModal();toast("Saved");setTimeout(load,400)})}

// ========== TEMP KEYPAD PINS LOGIC ==========
function loadTempPINs(){
  api("/temp_pins/list").then(function(r){
    if(!r.ok) throw 0;
    return r.json();
  }).then(function(d){
    TEMP_DATA=d;
    renderTempPINs();
  }).catch(function(){
    toast("Failed to load temporary PINs");
  });
}

function renderTempPINs(){
  var L=$("tempList"); L.innerHTML=""; var n=0;
  TEMP_DATA.forEach(function(e){
    n++; var c=document.createElement("div"); c.className="card entry";
    var badge = e.isUsed 
      ? '<span class="badge badge-used">🔴 USED / EXPIRED</span>' 
      : '<span class="badge badge-active">🟢 ACTIVE</span>';
    
    c.innerHTML = 
      '<div style="display:flex;justify-content:space-between;align-items:center">' +
        '<div>' +
          '<div class="pin-display">' + esc(e.pin) + '</div>' +
          '<div class="u" style="margin-top:6px;font-weight:600;font-size:14px">' + esc(e.note || "Temp PIN") + '</div>' +
        '</div>' +
        '<div>' + badge + '</div>' +
      '</div>' +
      '<div class="acts" style="margin-top:12px">' +
        '<button class="danger" onclick="delTempPIN(' + e.id + ')">Delete PIN</button>' +
      '</div>';
    L.appendChild(c);
  });
  if(!n) L.innerHTML='<p class="mut" style="text-align:center;padding:20px 0">No temporary PINs created.</p>';
}

function openAddTemp(){
  $("tpPin").value="";
  $("tpNote").value="";
  genTempPinDigits(); // Auto pre-fill randomized 4-digit PIN
  $("modalTemp").classList.remove("hidden");
}
function closeAddTemp(){$("modalTemp").classList.add("hidden")}

function saveTempPIN(){
  var pin = $("tpPin").value.trim();
  var note = $("tpNote").value.trim();
  if(!/^\d{4}$/.test(pin)){
    toast("PIN must be exactly 4 digits");
    $("tpPin").focus();
    return;
  }
  post("/temp_pins/add", {pin: pin, note: note}).then(function(r){
    if(!r.ok) throw 0;
    closeAddTemp();
    toast("Temp PIN Created");
    loadTempPINs();
  }).catch(function(){
    toast("Failed to add PIN");
  });
}

function delTempPIN(id){
  confirmBox("Delete this temporary PIN?", function(){
    post("/temp_pins/delete", {id: id}).then(function(){
      toast("Deleted");
      loadTempPINs();
    });
  });
}

function clearExpiredTemp(){
  post("/temp_pins/clear_expired", {}).then(function(){
    toast("Expired PINs cleared");
    loadTempPINs();
  });
}

// ========== DIALOGS & UTILS ==========
var cfCb=null;
function confirmBox(msg,cb){$("cfMsg").textContent=msg;cfCb=cb;$("confirm").classList.remove("hidden")}
function cfNo(){$("confirm").classList.add("hidden");cfCb=null}
function cfYes(){$("confirm").classList.add("hidden");var c=cfCb;cfCb=null;if(c)c()}
function del(id){confirmBox("Delete this entry? This cannot be undone.",function(){
  post("/delete",{id:id}).then(function(){toast("Deleted");setTimeout(load,400)})})}

function openImport(){$("ibulk").value="";$("imp").classList.remove("hidden")}
function closeImport(){$("imp").classList.add("hidden")}
function doImport(){post("/save",{bulk:$("ibulk").value}).then(function(){closeImport();toast("Imported");setTimeout(load,600)})}
function doExport(){window.location="/export?code="+encodeURIComponent(CODE)}
$("gcode").addEventListener("keydown",function(e){if(e.key=="Enter")unlock()});

// ========== WEB UI SETTINGS ==========
function openSettings(){
  api("/webui/settings").then(function(r){
    if(!r.ok)throw new Error("Failed to load settings");
    return r.json();
  }).then(function(d){
    $("wuiSSID").value=d.ssid||"SecureKey";
    $("wuiCode").value=d.pairCode||"";
    $("wuiPass").value=d.wifiPassword||"";
    $("wuiBg").checked=d.runInBackground||false;
    $("webui").classList.remove("hidden");
  }).catch(function(err){
    toast("Failed to load settings");
    console.error(err);
  });
}
function closeSettings(){$("webui").classList.add("hidden")}
function saveSettings(){
  var ssid=$("wuiSSID").value.trim();
  var code=$("wuiCode").value.trim();
  var pass=$("wuiPass").value;
  var bg=$("wuiBg").checked;
  
  if(!ssid){toast("WiFi Name required");$("wuiSSID").focus();return}
  if(ssid.length<1||ssid.length>32){toast("WiFi Name: 1-32 chars");$("wuiSSID").focus();return}
  
  if(!code){toast("Pair Code required");$("wuiCode").focus();return}
  if(!/^\d{6}$/.test(code)){toast("Pair Code: 6 digits required");$("wuiCode").focus();return}
  
  if(!pass){toast("WiFi Password required");$("wuiPass").focus();return}
  if(pass.length<8){toast("WiFi Password: min 8 characters");$("wuiPass").focus();return}
  
  post("/webui/settings",{
    ssid:ssid,
    pairCode:code,
    wifiPassword:pass,
    runInBackground:bg?"1":"0"
  }).then(function(r){
    if(!r.ok)throw new Error("Save failed");
    closeSettings();
    toast("Settings saved! Restart device to apply.");
  }).catch(function(err){
    toast("Failed to save settings");
    console.error(err);
  });
}
</script></body></html>)HTML";
