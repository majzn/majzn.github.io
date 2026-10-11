'use strict';
const $=id=>document.getElementById(id),score=new LivingScore($('scoreViewport')),audio=new FugueAudio();
let config=Fugue.settings({}),worker=new Worker('worker.js'),requestId=0,generation=0,cycle=null,nextCycle=null,playing=false,loading=false,baseTime=0,pausedAt=0,lastTick=0,eventIndex=0,flat=[],names=[],muted=[],lastSection=null,frame=0;
window.expressiveAudio=audio;VoiceEditor.restore(audio);const voiceSpace=new VoiceSpace((()=>{try{return window.localStorage;}catch{return null;}})()),history=new Map();
// On narrow screens, initially fit the circles around the microphone without changing their acoustic coordinates.
if(innerWidth<620&&!voiceSpace.storage?.getItem('continuum-voice-space-v1'))voiceSpace.setView({zoom:Math.max(.3,Math.min(1,(innerWidth-15)/900)),panX:0,panY:0},false);
const entropyTokens=new Map();let entropyState=0,entropyGestureAt=0;
function mixEntropy(value){entropyState=Math.imul(entropyState^Number(value),1664525)+1013904223|0;}
try{const entropyBytes=new Uint32Array(2);crypto.getRandomValues(entropyBytes);entropyBytes.forEach(mixEntropy);}catch{mixEntropy(performance.now()*1000);}
document.addEventListener('pointermove',e=>{const now=performance.now();if(now-entropyGestureAt>90){entropyGestureAt=now;mixEntropy((e.clientX|0)*65537+(e.clientY|0));mixEntropy(Math.floor(now*10));}},{passive:true});
document.addEventListener('keydown',e=>{mixEntropy(e.keyCode*977+Math.floor(performance.now()));},{passive:true});
function entropySettings(index,original){if(!$('liveEntropy')?.checked||String(original.seed).includes(':entropy:'))return original;let token=entropyTokens.get(index);if(token===undefined){const bits=new Uint32Array(2);try{crypto.getRandomValues(bits);}catch{bits[0]=Date.now();bits[1]=entropyState;}token=[bits[0],bits[1],entropyState>>>0,index].map(x=>x.toString(16)).join('-');entropyTokens.set(index,token);while(entropyTokens.size>64)entropyTokens.delete(entropyTokens.keys().next().value);}return{...original,seed:original.seed+':entropy:'+token};}
const requests=new Map();worker.onmessage=e=>{const {id,cycle,error}=e.data,job=requests.get(id);if(job){requests.delete(id);error?job.reject(new Error(error)):job.resolve(cycle);}};worker.onerror=e=>{for(const job of requests.values())job.reject(new Error(e.message||'The composer could not start.'));requests.clear();};
function compose(index,settings=config){return new Promise((resolve,reject)=>{const id=++requestId;requests.set(id,{resolve,reject});worker.postMessage({id,index,settings:entropySettings(index,settings)});});}
function status(s){$('status').textContent=/^(Ready|Playing|Paused)/.test(s)&&!s.includes('fallback')?'':s;}
// The title is clocked by musical playback time: pauses freeze the dissolve,
// and a cycle boundary always receives a new deterministic composition title.
let activeCycleTitle=null;
function changeCycleTitle(c){
 activeCycleTitle=CycleTitles.title(c);
 $('cycleTitleText').textContent=activeCycleTitle.name;
 $('cycleTitleSubtitle').textContent=activeCycleTitle.subtitle;
 $('cycleTitle').style.opacity='0';
 positionCycleTitle();
}
function positionCycleTitle(){
 const viewport=$('scoreViewport').getBoundingClientRect();
 // Keep the caption above the actual first visible staff, not inside the scrolling SVG.
 const label=$('cycleTitle'),height=label.getBoundingClientRect().height;
 label.style.top=Math.max(innerWidth<530?51:59,viewport.top-height-8)+'px';
}
function animateCycleTitle(seconds=0){
 const active=!!cycle&&document.body.classList.contains('minimalist-mode');
 const opacity=active?CycleTitles.opacityAt(seconds,cycle.duration):0;
 const element=$('cycleTitle');
 if(Math.abs(+(element.style.opacity||0)-opacity)>.002)element.style.opacity=String(opacity);
}
window.addEventListener('resize',()=>requestAnimationFrame(positionCycleTitle),{passive:true});
new ResizeObserver(()=>positionCycleTitle()).observe($('scoreViewport'));
function settingsFromUI(){return Fugue.settings({...config,seed:$('seed').value.trim()||'counterpoint',voices:+$('voices').value,tempo:+$('tempo').value,density:+$('density').value/100,adventure:+$('adventure').value/100,themeCount:+$('themeCount').value,thematicInterplay:+$('thematicInterplay').value/100,callResponse:+$('callResponse').value/100,development:+$('development').value/100,mood:+$('mood').value/100,openingMode:$('openingMode').value,openingTexture:$('openingTexture').value,openingVoices:$('openingVoices').value,counterpointStyle:$('counterpointStyle').value,formMode:$('formMode').value});}
function setCycle(c,continuous=false){cycle=c;changeCycleTitle(c);flat=c.events.flat().sort((a,b)=>a.at-b.at||a.voice-b.voice);eventIndex=0;pausedAt=0;lastTick=0;lastSection=null;score.setCycle(c,continuous);if(!history.has(c.index))history.set(c.index,{base:c.settings,edits:[],subject:c.subject,home:c.home});while(history.size>128)history.delete(history.keys().next().value);if(names.length!==c.roles.length){names=paletteNames($('palette').value,c.roles.length);audio.muted=Array.from({length:c.roles.length},(_,v)=>!!audio.muted[v]);audio.voiceLevels=Array.from({length:c.roles.length},(_,v)=>audio.voiceLevels[v]??1);if(audio.ctx)audio.load(names,soundStatus);audio.syncSequencePatches();}renderMix();renderDock();if(!staffPaletteInitialized||staffColors&&staffColors.length!==c.roles.length){updatePalette();staffPaletteInitialized=true;}updateDisplay(0);}
let pendingNextId=0;async function prepareNext(version){const intent=++pendingNextId;try{const c=await compose(cycle.index+1);if(version===generation&&intent===pendingNextId&&c.index===cycle.index+1){nextCycle=c;score.prepareNext(c);}}catch(e){if(version===generation)status('Next cycle could not be composed: '+e.message);}}
async function reset(resume=false){const version=++generation;pause(false);loading=true;nextCycle=null;history.clear();entropyTokens.clear();$('scoreShell').classList.add('recomposing');$('cycleTitle').style.opacity='0';$('play').disabled=true;$('playLabel').textContent='Composing';status('Composing a new tonal journey…');config=settingsFromUI();$('seed').value=config.seed;if(names.length!==config.voices){names=paletteNames($('palette').value,config.voices);audio.muted=Array.from({length:config.voices},(_,v)=>!!audio.muted[v]);audio.voiceLevels=Array.from({length:config.voices},(_,v)=>audio.voiceLevels[v]??1);}renderMix();renderDock();try{const [c]=await Promise.all([compose(0),new Promise(resolve=>setTimeout(resolve,cycle?280:0))]);if(version!==generation)return;setCycle(c);$('scoreShell').classList.remove('recomposing');requestAnimationFrame(()=>$('scoreShell').classList.add('ready'));loading=false;$('play').disabled=false;$('playLabel').textContent='Play';status('Ready · press Play to begin');prepareNext(version);if(resume)await play();}catch(e){if(version!==generation)return;loading=false;$('playLabel').textContent='Unavailable';status('Unable to prepare the score: '+e.message);}}
async function play(){if(loading||!cycle||playing)return;const version=generation;try{await audio.init();if(version!==generation)return;if((!audio.instruments.length||audio.names.join('|')!==names.join('|')||audio.loadedBank!==audio.bank)){
  loading=true;$('play').disabled=true;$('playLabel').textContent='Loading choir';status('Preparing the ensemble…');
  // Do not start playback using an unrelated loud synth before choir patches finish.
  // Wait for the selected bank; fallback vowels are used only if loading fails.
  let loadTimer;const loaded=audio.load(names,soundStatus).then(()=>true).catch(()=>false);
  const finished=await Promise.race([loaded,new Promise(resolve=>{loadTimer=setTimeout(()=>resolve(false),5500);})]);clearTimeout(loadTimer);
  if(!finished){audio.loadVersion++;audio.instruments=Array(names.length).fill(null);soundStatus('Sample server unavailable; using the soft choir synthesizer.');}
  if(version!==generation)return;
  loading=false;$('play').disabled=false;
 }audio.syncSequencePatches();baseTime=audio.ctx.currentTime+.08-pausedAt;playing=true;score.setPlaying(true);for(const e of flat){const start=e.at,end=e.until;if(start<pausedAt-.001&&end>pausedAt+.02)audio.play(e,baseTime+pausedAt,end-pausedAt,cycle.roles.length,cycle);}$('playIcon').textContent='Ⅱ';$('playLabel').textContent='Pause';status(audio.fallback?'Playing · built-in organ fallback':'Playing · phrase-shaped rubato');schedule();}catch(e){if(version!==generation)return;loading=false;$('play').disabled=false;status('Audio could not start: '+e.message);$('playLabel').textContent='Play';}}
function pause(announce=true){if(playing){pausedAt=Math.max(0,audio.ctx.currentTime-baseTime);playing=false;score.setPlaying(false);audio.stop();fadeConstellation();eventIndex=flat.findIndex(e=>e.at>=pausedAt-.001);if(eventIndex<0)eventIndex=flat.length;}cancelAnimationFrame(frame);animateCycleTitle(pausedAt);$('playIcon').textContent='▶';$('playLabel').textContent='Play';if(announce&&cycle)status('Paused · the score will wait');}
function tickAt(seconds){const timing=cycle.timing;let lo=0,hi=timing.length-1;while(lo<hi){const mid=Math.ceil((lo+hi)/2);if(timing[mid]<=seconds)lo=mid;else hi=mid-1;}return Math.min(cycle.length-.001,lo+(seconds-timing[lo])/(timing[lo+1]-timing[lo]||1));}
function schedule(){if(!playing)return;const now=audio.ctx.currentTime,elapsed=now-baseTime;
 if(elapsed>=cycle.duration){if(!nextCycle){pause(false);status('Waiting for the next cycle. Press Play when ready.');return;}const end=baseTime+cycle.duration,c=nextCycle;nextCycle=null;setCycle(c,true);baseTime=end;prepareNext(generation);}
 const time=audio.ctx.currentTime-baseTime;
 // A suspended/throttled browser must not dump a backlog of notes into the audio graph.
 if(eventIndex<flat.length&&flat[eventIndex].at<time-.35){pause();status('Playback paused while the browser was busy. Press Play to continue.');return;}
 while(eventIndex<flat.length&&baseTime+flat[eventIndex].at<now+.16){const e=flat[eventIndex++],at=baseTime+e.at,duration=e.until-e.at;if(at>=now-.035)audio.play(e,Math.max(now,at),duration,cycle.roles.length,cycle);}
 audio.clean();lastTick=tickAt(Math.max(0,time));score.draw(lastTick);animateCycleTitle(Math.max(0,time));updateConstellation(now);if(now-lastDisplayAt>.12){updateDisplay(lastTick);lastDisplayAt=now;}frame=requestAnimationFrame(schedule);
}
let pendingBar=null,lastDisplayAt=-Infinity;
function updateDisplay(t){
 const m=Fugue.measureAt(cycle,t),segments=cycle.controlSegments||[{bar:0,settings:cycle.settings}],active=segments.filter(x=>x.bar<=m.index).at(-1)?.settings||cycle.settings;
 if(pendingBar!=null&&m.index>=pendingBar){status('');pendingBar=null;}
 for(const [id,param]of [['tempo','tempo'],['density','density'],['adventure','adventure']]){
  const base=param==='tempo'?config[param]:Math.round(config[param]*100),current=Fugue.automated(active,param,cycle.index+t/cycle.length,m.index,m.section),unit=param==='tempo'?'':'%',actual=param==='tempo'?Math.round(current):Math.round(current*100);
  const label=String(base)+(active[param+'Mode']==='fixed'&&base===actual?'': ' / '+actual)+unit;if($(id+'Value').textContent!==label)$(id+'Value').textContent=label;
 }
 lastSection=cycle.sections.find(s=>t>=s.start&&t<s.end)||cycle.sections.at(-1);
 if(window.updateAnalysis)window.updateAnalysis(cycle,m,lastSection);
}
function paletteNames(p,n){return ContinuumEnsembles.instruments(p,n);}
function soundStatus(s){$('soundStatus').textContent=s;}
async function reloadInstruments(){if(!audio.ctx)return;await audio.load(names,soundStatus);}
async function applySettings(){if(!cycle||loading){config=settingsFromUI();return;}config=settingsFromUI();const snapshot=structuredClone(config),version=++generation,index=cycle.index;status('Preparing upcoming music…');try{
 if(snapshot.voices!==cycle.roles.length){await prepareNext(version);if(version===generation)status('Voice count changes at the next cycle.');return;}
 const fresh=await compose(index,snapshot);if(version!==generation)return;if(cycle.index!==index){prepareNext(version);return;}
 // Everything already on the screen, plus the scheduling lookahead, is immutable.
 const displayed=[...score.cache.values()].filter(i=>i.record===score.current).map(i=>i.bar),bar=Math.max(Fugue.measureAt(cycle,lastTick).index+2,...displayed.map(b=>b+1));
 if(bar<Math.min(cycle.bars,fresh.bars)-2){const merged=Fugue.spliceFuture(cycle,fresh,bar);cycle=merged;flat=merged.events.flat().sort((a,b)=>a.at-b.at||a.voice-b.voice);score.replaceFuture(merged,bar);const record=history.get(index);record.edits.push({bar,settings:snapshot});pendingBar=bar;status('Changes begin at bar '+(bar+1)+'.');}else status('Changes begin with the next cycle.');
 await prepareNext(version);
 }catch(e){if(version===generation)status('Could not update upcoming music: '+e.message);}}
async function cycleForExport(index){if(cycle?.index===index)return structuredClone(cycle);const record=history.get(index);let result=await compose(index,record?.base||structuredClone(config));for(const edit of record?.edits||[])result=Fugue.spliceFuture(result,await compose(index,edit.settings),edit.bar);return result;}
const GM_FAMILIES=['Pianos','Chromatic percussion','Organs','Guitars','Basses','Strings','Ensembles & choirs','Brass','Reeds','Pipes & flutes','Synth leads','Synth pads','Synth effects','World instruments','Percussion','Sound effects'];
function addGMOptions(select,prefix=''){
 for(let family=0;family<16;family++){const group=document.createElement('optgroup');group.label=GM_FAMILIES[family];for(const [id,label,program]of INSTRUMENTS){if(id==='synth'||program<family*8||program>family*8+7)continue;group.append(new Option(label,prefix+id));}select.append(group);}
}
function addImportedOptions(select){for(const entry of audio.customFonts){const group=document.createElement('optgroup');group.label='SoundFont · '+entry.name;entry.font.presets.forEach((patch,i)=>{if(patch.zones.length)group.append(new Option(`${patch.name} · bank ${patch.bank}, program ${patch.program+1}`,'sf2:'+entry.id+':'+i));});select.append(group);}}
function renderMix(){
 const root=$('voiceMix');if($('inspectorContent').firstElementChild)root.append($('inspectorContent').firstElementChild);root.replaceChildren();
 Fugue.roles(cycle?.roles.length||config.voices).forEach((role,v)=>{
  const row=document.createElement('div');row.className='mix-row';const label=document.createElement('label');label.textContent=role.name;label.htmlFor='instrument'+v;
  const select=document.createElement('select');select.id='instrument'+v;select.setAttribute('aria-label',role.name+' instrument');
  select.add(new Option('Built-in organ (offline)','synth'));addGMOptions(select);addImportedOptions(select);select.value=names[v];select.onchange=()=>{names[v]=select.value;renderDock();reloadInstruments().catch(e=>soundStatus(e.message));};
  const level=document.createElement('label');level.className='voice-level';const range=document.createElement('input');range.type='range';range.min='0';range.max='150';range.step='1';range.value=String(Math.round((audio.voiceLevels[v]??1)*100));range.setAttribute('aria-label',role.name+' level');const output=document.createElement('output');output.textContent=range.value+'%';range.oninput=()=>{audio.setVoiceLevel(v,+range.value/100);output.textContent=range.value+'%';};level.append(range,output);
  const muteLabel=document.createElement('label');muteLabel.className='voice-mute';const check=document.createElement('input');check.type='checkbox';check.setAttribute('aria-label','Mute '+role.name);check.checked=!!audio.muted[v];check.onchange=()=>{audio.setVoiceMute(v,check.checked);renderDock();};muteLabel.append(check,'Mute');row.append(label,select,level,muteLabel);root.append(row);VoiceEditor.render(row,v);
 });
 renderDock();window.refreshEnsembleLineup?.();
}
$('play').onclick=()=>playing?pause():play();$('restart').onclick=()=>reset(playing);
// Minimalist view is the default on every new load; users can return to the
// complete workstation without resetting their score or audio position.
const syncMinimalistTransport=()=>{
 $('minimalPlay').disabled=$('play').disabled;
 $('minimalPlayIcon').textContent=$('playIcon').textContent;
 const isPlaying=$('playIcon').textContent==='Ⅱ';
 $('minimalPlayLabel').textContent=isPlaying?'Pause':loading?'Preparing':'Play';
 $('minimalPlay').setAttribute('aria-label',isPlaying?'Pause':'Play');
 $('minimalPlay').title=isPlaying?'Pause (Space)':'Play (Space)';
 $('minimalRandomize').disabled=$('randomizePlay').disabled;
};
function setMinimalistMode(enabled){
 const active=!!enabled;
 document.body.classList.toggle('minimalist-mode',active);
 if(active){
  for(const dialog of document.querySelectorAll('dialog[open]'))dialog.close();
  $('instrumentInspector').hidden=true;$('voiceContextMenu').hidden=true;
 }
 $('minimalistControls').setAttribute('aria-hidden',String(!active));
 $('enterMinimalist').setAttribute('aria-pressed',String(active));
 updateToolbarHeight();score.dirty=true;if(cycle)score.draw(lastTick);requestAnimationFrame(()=>{positionCycleTitle();animateCycleTitle(playing?Math.max(0,audio.ctx.currentTime-baseTime):pausedAt);});
 if(!active)$('enterMinimalist').focus({preventScroll:true});
}
$('minimalPlay').onclick=()=>$('play').click();
$('minimalRestart').onclick=()=>$('restart').click();
$('minimalRandomize').onclick=()=>$('randomizePlay').click();
$('exitMinimalist').onclick=()=>setMinimalistMode(false);
$('enterMinimalist').onclick=()=>setMinimalistMode(true);
new MutationObserver(syncMinimalistTransport).observe($('play'),{attributes:true,attributeFilter:['disabled']});
new MutationObserver(syncMinimalistTransport).observe($('playIcon'),{childList:true,characterData:true,subtree:true});
new MutationObserver(syncMinimalistTransport).observe($('randomizePlay'),{attributes:true,attributeFilter:['disabled']});
syncMinimalistTransport();
document.addEventListener('keydown',e=>{
 if(e.key==='Escape'&&document.body.classList.contains('minimalist-mode')){
  e.preventDefault();setMinimalistMode(false);
 }
});

// A full reset is intentionally a clean restart: reinitialize all in-memory engine,
// audio, loaded SoundFonts and dialogs from the original HTML defaults. Only this
// application's preference keys are deleted; downloaded sample caches remain intact.
$('resetAll').onclick=()=>{
 if(audio.recorder){status('Stop and save the WAV recording before resetting all settings.');return;}
 const approved=window.confirm('Restore all Continuum defaults?\n\nThis resets the composition, choir ensemble, mix, instrument rotations, evolution, theme selection, layout, zoom and imported SoundFonts. Saved themes, voice expression presets and instrument positions will be deleted. Downloaded files are not affected.');
 if(!approved)return;
 pause(false);audio.stop();++generation;
 try{
  for(const key of ['continuum-voice-space-v1','continuum-expressive-voices-v1','continuum-themes-v1'])window.localStorage.removeItem(key);
 }catch(e){status('Local settings could not be cleared: '+e.message);return;}
 window.location.reload();
};$('seedForm').onsubmit=e=>{e.preventDefault();applySettings();};$('newSeed').onclick=()=>{const a=new Uint32Array(2);crypto.getRandomValues(a);$('seed').value=[...a].map(x=>x.toString(36)).join('-');applySettings();};$('randomizePlay').onclick=async()=>{if(loading||audio.recorder)return;const a=new Uint32Array(2);crypto.getRandomValues(a);$('seed').value=[...a].map(x=>x.toString(36)).join('-');$('randomizePlay').disabled=true;try{await reset(true);}finally{$('randomizePlay').disabled=false;}};
for(const id of ['tempo','density','adventure','thematicInterplay','callResponse','development']){$(id).oninput=()=>{$(id+'Value').textContent=$(id).value+(id==='tempo'?'':'%');};$(id).onchange=()=>applySettings();}$('voices').onchange=()=>applySettings();for(const id of ['mood']){$(id).oninput=()=>{$(id+'Value').textContent=+$(id).value===0?'Auto':(+$(id).value>0?'+':'')+$(id).value+'%';};$(id).onchange=()=>applySettings();}$('counterpointStyle').onchange=()=>applySettings();$('formMode').onchange=()=>applySettings();$('openingMode').onchange=()=>applySettings();$('openingTexture').onchange=()=>applySettings();$('openingVoices').onchange=()=>applySettings();$('themeCount').onchange=()=>applySettings();$('liveEntropy').onchange=()=>{entropyTokens.clear();if(cycle)prepareNext(generation);status($('liveEntropy').checked?'Live entropy enabled · random influences are applied on the next restart or upcoming cycle.':'Reproducible seeded generation enabled · restart to apply.');};
function assignChoirFont(){
 const font=audio.customFonts.at(-1);if(!font)return;
 const patches=font.font.presets.map((p,i)=>({...p,i})).filter(p=>p.zones.length),choose=rx=>patches.find(p=>rx.test(p.name))||null;
 const ah=choose(/\b(?:choir\s*)?a+hs?\b|\bchorus\b|\bchoir\b/i)||patches.find(p=>p.program===52)||patches[0];
 const oo=choose(/\b(?:voice\s*)?o+ohs?\b|\bo+hs?\b|\bhum(?:ming)?\b/i)||patches.find(p=>p.program===53)||ah;
 if(!ah||!oo)return;
 const count=cycle?.roles.length||config.voices;
 for(let v=0;v<count;v++){
  const patch=v/(count-1)<=.34?oo:ah,id='sf2:'+font.id+':'+patch.i;
  names[v]=id;
 }
 renderMix();reloadInstruments().catch(e=>soundStatus(e.message));
 $('soundfontInfo').textContent=`Assigned ${font.name}: upper voices → ${oo.name}; middle/lower voices → ${ah.name}. Import remains in memory until this page is closed.`;
}
$('assignChoirFont').onclick=assignChoirFont;
$('soundfontFile').onchange=async()=>{const picker=$('soundfontFile'),file=picker.files[0];if(!file)return;try{if(!/\.sf2$/i.test(file.name))throw Error('Choose an uncompressed .sf2 SoundFont. SF3 is not supported.');if(file.size>300*1024*1024)throw Error('SoundFont exceeds the 300 MB size limit.');if(audio.customFonts.length>=8)throw Error('Up to 8 SoundFonts may be loaded per session.');picker.disabled=true;$('soundfontInfo').textContent='Reading '+file.name+'…';const result=audio.addSoundFont(await file.arrayBuffer(),file.name);$('assignChoirFont').disabled=false;const first=[...audio.customInstruments.keys()].find(k=>k.startsWith('sf2:'+result.id+':'));if(!first)throw Error('No playable instruments in this SoundFont.');names[0]=first;renderMix();await reloadInstruments();$('soundfontInfo').textContent=`Loaded ${result.name}: ${result.count} presets. Voice 1 now uses its first preset; choose any imported preset per voice.`;}catch(e){$('soundfontInfo').textContent='Could not load SoundFont: '+e.message;}finally{picker.disabled=false;picker.value='';}};
const setScoreZoom=z=>{score.setZoom(z);$('zoomReadout').textContent=Math.round(score.zoom*100)+'%';};
$('zoomIn').onclick=()=>setScoreZoom(score.zoom*1.2);$('zoomOut').onclick=()=>setScoreZoom(score.zoom/1.2);$('zoomReset').onclick=()=>setScoreZoom(1);
$('scoreViewport').addEventListener('wheel',e=>{if(!e.ctrlKey&&!e.metaKey)return;e.preventDefault();setScoreZoom(score.zoom*(e.deltaY<0?1.11:1/1.11));},{passive:false});
document.addEventListener('keydown',e=>{if(e.ctrlKey||e.metaKey||e.altKey||['INPUT','SELECT','BUTTON','TEXTAREA'].includes(document.activeElement.tagName)||document.querySelector('dialog[open]'))return;if(e.key==='+'||e.key==='='){e.preventDefault();setScoreZoom(score.zoom*1.2);}else if(e.key==='-'){e.preventDefault();setScoreZoom(score.zoom/1.2);}else if(e.key==='0'){e.preventDefault();setScoreZoom(1);}});
$('palette').onchange=()=>{names=paletteNames($('palette').value,cycle?.roles.length||config.voices);renderMix();reloadInstruments();};$('volume').oninput=()=>{audio.setVolume(+$('volume').value/100);$('volumeValue').textContent=$('volume').value+'%';};$('reverb').oninput=()=>{audio.setReverb(+$('reverb').value/100);$('reverbValue').textContent=$('reverb').value+'%';};$('sampleBank').onchange=()=>{audio.bank=$('sampleBank').value;reloadInstruments();};
document.querySelectorAll('dialog .close').forEach(b=>b.onclick=()=>b.closest('dialog').close());document.querySelectorAll('dialog').forEach(d=>d.addEventListener('click',e=>{if(e.target===d){const r=d.getBoundingClientRect();if(e.clientX<r.left||e.clientX>r.right||e.clientY<r.top||e.clientY>r.bottom)d.close();}}));
$('connectMidi').onclick=async()=>{try{if(!navigator.requestMIDIAccess)throw new Error('This browser does not support Web MIDI. Try Chrome or Edge.');audio.access=await navigator.requestMIDIAccess({sysex:false});const fill=()=>{const select=$('midiOutput');select.replaceChildren(new Option('Browser audio',''));for(const [id,out]of audio.access.outputs)select.add(new Option(out.name,id));soundStatus(audio.access.outputs.size?'Choose a MIDI output below.':'No MIDI output detected. Connect your synthesizer and try again.');};fill();audio.access.onstatechange=fill;}catch(e){soundStatus('MIDI: '+e.message);}};
$('midiOutput').onchange=()=>{audio.stop();audio.midi=audio.access?.outputs.get($('midiOutput').value)||null;audio.programs();};
// One compact top row remains visible; all specialist controls live in the Advanced drawer.
$('advancedToggle').onclick=()=>{
 const expanded=document.body.classList.toggle('advanced-open');
 $('advancedToggle').setAttribute('aria-expanded',String(expanded));
 $('advancedToggle').lastElementChild.textContent=expanded?'⌃':'⌄';
 updateToolbarHeight();score.dirty=true;if(cycle)score.draw(lastTick);requestAnimationFrame(()=>{positionCycleTitle();animateCycleTitle(playing?Math.max(0,audio.ctx.currentTime-baseTime):pausedAt);});
};
const updateToolbarHeight=()=>document.documentElement.style.setProperty('--toolbar-height',Math.ceil($('topToolbar').getBoundingClientRect().height)+'px');
new ResizeObserver(updateToolbarHeight).observe($('topToolbar'));
updateToolbarHeight();
$('analysisToggle').onclick=()=>{const active=document.body.classList.toggle('analysis-visible');$('analysisToggle').setAttribute('aria-pressed',String(active));if(active&&cycle){$('analysisOverlay').dataset.key='';window.updateAnalysis?.(cycle,Fugue.measureAt(cycle,lastTick),lastSection);}};
document.addEventListener('visibilitychange',()=>{if(document.hidden&&playing){pause();status('Paused while the tab is hidden · press Play to resume');}});
document.addEventListener('keydown',e=>{if(e.code==='Space'&&!['INPUT','SELECT','BUTTON','TEXTAREA'].includes(document.activeElement.tagName)&&!document.querySelector('dialog[open]')){e.preventDefault();playing?pause():play();}});
new ResizeObserver(()=>{if(cycle){score.dirty=true;score.draw(lastTick);}}).observe($('scoreViewport'));
let selectedVoice=-1,contextTarget=-1,draggingVoice=null,lastConstellationUpdate=0;
function positionDisc(button,v,count,updateAudio=true){const p=voiceSpace.get(v,count);const xy=VoiceSpaceMath.project(p,voiceSpace.view);button.style.left=`calc(50% + ${xy.x.toFixed(2)}px)`;button.style.top=`calc(50% + ${xy.y.toFixed(2)}px)`;if(updateAudio)audio.setVoicePosition(v,p,count);const spatial=VoiceSpaceMath.params(p);button.dataset.position=`${spatial.pan<0?'L':spatial.pan>0?'R':'C'} ${Math.abs(Math.round(spatial.pan*100))}  ·  ${Math.round(spatial.gain*100)}%`;button.title=`${Fugue.roles(count)[v]?.name||'Voice'} · Pan ${Math.round(spatial.pan*100)} · Level ${Math.round(spatial.gain*100)}% · drag to position`;}
function fadeConstellation(){for(const node of $('voiceDock').querySelectorAll('.voice-circle')){node.dataset.level='0';const disc=node.querySelector('.voice-disc');if(disc){disc.style.backgroundColor='';disc.style.borderColor='';disc.style.boxShadow='';}}}
function updateConstellation(now=0){
 if(now-lastConstellationUpdate<.045)return;lastConstellationUpdate=now;
 for(const button of $('voiceDock').querySelectorAll('.voice-circle')){const v=+button.dataset.voice,meter=audio.readVoiceMeter(v),disc=button.querySelector('.voice-disc');if(!disc)continue;
  const target=Math.min(1,Math.max(0,(20*Math.log10(Math.max(meter.level/3,0.00001))+56)/46)),previous=+button.dataset.level||0,level=previous+(target-previous)*(target>previous?.43:.19);button.dataset.level=level.toFixed(3);
  const color=staffColors?.[v]||'#9b8170';disc.style.backgroundColor=`color-mix(in srgb, ${color} ${Math.round(10+level*73)}%, #f8f4ed)`;disc.style.borderColor=`color-mix(in srgb, ${color} ${Math.round(30+level*65)}%, #b0a393)`;disc.style.boxShadow=`0 0 ${Math.round(2+level*23)}px color-mix(in srgb, ${color} ${Math.round(8+level*60)}%, transparent)`;
 }
}
function attachVoiceHandlers(button,v,count){
 let gesture=null,ignoreClick=false;const dock=$('voiceDock');
 button.addEventListener('pointerdown',e=>{if(e.button!==0)return;ignoreClick=false;const p=voiceSpace.get(v,count);gesture={id:e.pointerId,x:e.clientX,y:e.clientY,start:p,dragged:false};button.setPointerCapture(e.pointerId);});
 button.addEventListener('pointermove',e=>{if(!gesture||gesture.id!==e.pointerId)return;
  if(!gesture.dragged&&Math.hypot(e.clientX-gesture.x,e.clientY-gesture.y)>5){gesture.dragged=true;button.classList.add('dragging');}
  if(!gesture.dragged)return;const rect=dock.getBoundingClientRect(),p=VoiceSpaceMath.fromPointer(rect,e.clientX,e.clientY,voiceSpace.view);voiceSpace.set(v,p,false);positionDisc(button,v,count);
 });
 function release(e){if(!gesture||gesture.id!==e.pointerId)return;if(gesture.dragged){ignoreClick=true;voiceSpace.save();button.classList.remove('dragging');}gesture=null;}
 button.addEventListener('pointerup',release);button.addEventListener('pointercancel',release);
 button.addEventListener('click',()=>{if(ignoreClick){ignoreClick=false;return;}openVoice(v);});
 button.addEventListener('dblclick',e=>{e.preventDefault();voiceSpace.reset(v);positionDisc(button,v,count);});
 button.addEventListener('keydown',e=>{if(!['ArrowLeft','ArrowRight','ArrowUp','ArrowDown'].includes(e.key))return;e.preventDefault();const p=voiceSpace.get(v,count),step=e.shiftKey?.035:.012;const move={ArrowLeft:[-step,0],ArrowRight:[step,0],ArrowUp:[0,-step],ArrowDown:[0,step]}[e.key];voiceSpace.set(v,{x:p.x+move[0],y:p.y+move[1]});positionDisc(button,v,count);});
}
function renderDock(){
 const dock=$('voiceDock');if(!dock)return;for(const button of [...dock.querySelectorAll('.voice-circle')])button.remove();
 const roles=Fugue.roles(names.length||config.voices),count=roles.length;
 roles.forEach((role,v)=>{
  const button=document.createElement('button');button.type='button';button.className='voice-circle';button.dataset.voice=v;button.setAttribute('aria-label','Move or edit '+role.name+' instrument');button.setAttribute('aria-pressed',String(selectedVoice===v&&!$('instrumentInspector').hidden));if(audio.muted[v])button.classList.add('is-muted');
  const disc=document.createElement('span');disc.className='voice-disc';const name=names[v]||'choir_aahs',id=INSTRUMENTS.find(i=>i[0]===name)?.[1]||audio.customInstruments.get(name)?.font?.name||name;
  const number=document.createElement('span');number.className='voice-number';number.textContent=String(v+1).padStart(2,'0');disc.append(number);
  const caption=document.createElement('span');caption.className='voice-name';caption.textContent=id;
  button.append(disc,caption);dock.append(button);positionDisc(button,v,count);attachVoiceHandlers(button,v,count);
 });
 if(selectedVoice>=count){selectedVoice=-1;$('instrumentInspector').hidden=true;}
 if(!$('instrumentInspector').hidden&&selectedVoice>=0)showVoiceEditor();
 syncWorldView();
}
function showVoiceEditor(){const v=selectedVoice,host=$('inspectorContent'),root=$('voiceMix');
 if(host.firstElementChild)root.append(host.firstElementChild);const row=[...root.querySelectorAll('.mix-row')].find(x=>x.querySelector('select')?.id==='instrument'+v);if(!row)return;
 host.replaceChildren(row);$('inspectorTitle').textContent=Fugue.roles(names.length)[v]?.name+' · instrument & expression';
}
function openVoice(v){if(selectedVoice===v&&!$('instrumentInspector').hidden){closeVoice();return;}selectedVoice=v;$('instrumentInspector').hidden=false;showVoiceEditor();for(const b of $('voiceDock').querySelectorAll('.voice-circle'))b.setAttribute('aria-pressed',String(+b.dataset.voice===v));}
function closeVoice(){if($('inspectorContent').firstElementChild)$('voiceMix').append($('inspectorContent').firstElementChild);$('instrumentInspector').hidden=true;for(const b of $('voiceDock').querySelectorAll('.voice-circle'))b.setAttribute('aria-pressed','false');}
$('closeInspector').onclick=closeVoice;
function hideContext(){$('voiceContextMenu').hidden=true;}
const dockTarget=e=>e.target.closest('.voice-circle');
function updateVoiceCount(delta,target=-1){const n=+($('voices').value),next=n+delta;if(next<2||next>8){hideContext();return;}
 const replace=delta>0?paletteNames($('palette').value,next).at(-1):null;
 if(delta>0){names.push(replace);voiceSpace.insert(next-1);}else{const index=target<0?n-1:target;voiceSpace.remove(index);names.splice(index,1);audio.voicePositions.splice(index,1);audio.voiceMeters.splice(index,1);audio.voiceMeterBuffers.splice(index,1);audio.voiceMeterSpectra.splice(index,1);audio.voiceLevels.splice(index,1);audio.muted.splice(index,1);for(const store of [audio.voiceSequences.normal,audio.expressionSettings.normal]){const shifted={};for(const [k,v] of Object.entries(store))if(+k!==index)shifted[+k>index?+k-1:+k]=v;Object.keys(store).forEach(key=>delete store[key]);Object.assign(store,shifted);}}
 $('voices').value=String(next);config.voices=next;selectedVoice=-1;closeVoice();hideContext();reset(playing);
}
$('addVoiceMenu').onclick=()=>updateVoiceCount(1);
$('removeVoiceMenu').onclick=()=>updateVoiceCount(-1,contextTarget);
// A camera moves the workspace without altering acoustic world positions.
let autoFitWorld=false;try{autoFitWorld=!voiceSpace.storage?.getItem('continuum-voice-space-v1');}catch{}
function fitInitialWorkspace(){if(!autoFitWorld)return;const rect=$('voiceDock').getBoundingClientRect(),count=names.length||config.voices;let maxX=1,maxY=1;for(let i=0;i<count;i++){const p=voiceSpace.get(i,count);maxX=Math.max(maxX,Math.abs((p.x-.5)*850));maxY=Math.max(maxY,Math.abs((p.y-.5)*480));}const fit=Math.max(.3,Math.min(1,(rect.width-90)/(2*maxX),(rect.height-90)/(2*maxY)));voiceSpace.setView({...voiceSpace.view,zoom:fit,panX:0,panY:0},false);syncWorldView();}
function syncWorldView(){const v=voiceSpace.view,grid=$('worldGrid'),mic=$('microphoneMarker');if(grid){grid.style.backgroundSize=`${Math.round(50*v.zoom)}px ${Math.round(50*v.zoom)}px`;grid.style.backgroundPosition=`calc(50% + ${v.panX}px) calc(50% + ${v.panY}px)`;}if(mic){mic.style.left=`calc(50% + ${v.panX}px)`;mic.style.top=`calc(50% + ${v.panY}px)`;}if($('worldZoomValue'))$('worldZoomValue').textContent=Math.round(v.zoom*100)+'%';for(const b of $('voiceDock').querySelectorAll('.voice-circle'))positionDisc(b,+b.dataset.voice,names.length||config.voices,false);}
function workspaceCamera(change,save=true){autoFitWorld=false;voiceSpace.setView({...voiceSpace.view,...change},save);syncWorldView();}
function workspaceZoom(mult,x,y){const rect=$('voiceDock').getBoundingClientRect(),v=voiceSpace.view,z=Math.max(.3,Math.min(4,v.zoom*mult)),px=(x??rect.left+rect.width/2)-(rect.left+rect.width/2),py=(y??rect.top+rect.height/2)-(rect.top+rect.height/2);workspaceCamera({zoom:z,panX:px-(px-v.panX)*z/v.zoom,panY:py-(py-v.panY)*z/v.zoom});}
$('worldZoomIn').onclick=()=>workspaceZoom(1.25);$('worldZoomOut').onclick=()=>workspaceZoom(.8);$('worldFrame').onclick=()=>workspaceCamera({zoom:1,panX:0,panY:0});
let worldWheelSave=null;
$('voiceDock').addEventListener('wheel',e=>{
 e.preventDefault();e.stopPropagation();if(e.ctrlKey||e.metaKey)return;
 const strength=e.deltaMode===1?16:e.deltaMode===2?Math.max(200,$('voiceDock').clientHeight):1;
 workspaceZoom(Math.exp(-e.deltaY*strength*.0014),e.clientX,e.clientY);
 clearTimeout(worldWheelSave);worldWheelSave=setTimeout(()=>voiceSpace.save(),160);
},{passive:false});
let panGesture=null;
$('voiceDock').addEventListener('pointerdown',e=>{if(e.target.closest('.voice-circle,#worldControls')||e.button!==0&&e.button!==1)return;panGesture={id:e.pointerId,x:e.clientX,y:e.clientY,start:{...voiceSpace.view}};$('voiceDock').setPointerCapture(e.pointerId);});
$('voiceDock').addEventListener('pointermove',e=>{if(panGesture?.id!==e.pointerId)return;workspaceCamera({panX:panGesture.start.panX+e.clientX-panGesture.x,panY:panGesture.start.panY+e.clientY-panGesture.y},false);});
$('voiceDock').addEventListener('pointerup',e=>{if(panGesture?.id===e.pointerId){voiceSpace.save();panGesture=null;}});$('voiceDock').addEventListener('pointercancel',e=>{if(panGesture?.id===e.pointerId){voiceSpace.save();panGesture=null;}});$('voiceDock').addEventListener('lostpointercapture',e=>{if(panGesture?.id===e.pointerId){voiceSpace.save();panGesture=null;}});
// Resize the real divider, not just the (possibly flex-grown) preferred size.
const divider=$('worldResize'),stage=document.querySelector('.score-stage');
function workspaceBounds(){return VoiceSpaceMath.heightBounds(stage.clientHeight);}
function updateDividerAccessibility(){const {min,max}=workspaceBounds();divider.setAttribute('aria-valuemin',String(Math.round(min)));divider.setAttribute('aria-valuemax',String(Math.round(max)));divider.setAttribute('aria-valuenow',String(Math.round($('voiceDock').getBoundingClientRect().height)));divider.setAttribute('aria-valuetext',`${Math.round($('voiceDock').getBoundingClientRect().height)} pixels for the instrument workspace`);}
function workspaceHeight(height,save=true){const bounds=workspaceBounds();const next=Math.max(bounds.min,Math.min(bounds.max,Math.round(height)));voiceSpace.setHeight(next,save);document.documentElement.style.setProperty('--world-height',voiceSpace.height+'px');fitInitialWorkspace();updateDividerAccessibility();}
let resizing=null;
function finishWorkspaceResize(e){if(!resizing||e.pointerId!==resizing.id)return;voiceSpace.save();resizing=null;divider.classList.remove('is-resizing');document.body.classList.remove('is-world-resizing');updateDividerAccessibility();}
divider.addEventListener('pointerdown',e=>{if(e.button!==0)return;resizing={id:e.pointerId,y:e.clientY,height:$('voiceDock').getBoundingClientRect().height};e.preventDefault();divider.setPointerCapture(e.pointerId);divider.classList.add('is-resizing');document.body.classList.add('is-world-resizing');});
divider.addEventListener('pointermove',e=>{if(resizing?.id===e.pointerId)workspaceHeight(resizing.height+resizing.y-e.clientY,false);});
divider.addEventListener('pointerup',finishWorkspaceResize);
divider.addEventListener('pointercancel',finishWorkspaceResize);
divider.addEventListener('dblclick',()=>workspaceHeight(260));
divider.addEventListener('lostpointercapture',finishWorkspaceResize);
divider.addEventListener('keydown',e=>{const delta=e.key==='ArrowUp'?20:e.key==='ArrowDown'?-20:e.key==='Home'?workspaceBounds().max:e.key==='End'?-workspaceBounds().max:0;if(!delta)return;e.preventDefault();workspaceHeight($('voiceDock').getBoundingClientRect().height+delta);});
workspaceHeight(voiceSpace.height,false);if(autoFitWorld)voiceSpace.save();syncWorldView();
new ResizeObserver(()=>updateDividerAccessibility()).observe(stage);
function arrangeFormation(kind){const count=+$('voices').value,points=VoiceSpaceMath.formation(kind,count,config.seed+':'+(cycle?.index||0));points.forEach((point,v)=>voiceSpace.set(v,point,false));voiceSpace.save();renderDock();hideContext();}
$('formationMenu').onchange=()=>{const kind=$('formationMenu').value;if(kind)arrangeFormation(kind);$('formationMenu').value='';};
// Horizontal engraving density is independent of both world zoom and score zoom.
function setScoreDensity(next){score.setDensity(next);$('densityReadout').textContent=Math.round(score.horizontalDensity*100)+'%';}
$('densityIn').onclick=()=>setScoreDensity(score.horizontalDensity*1.2);$('densityOut').onclick=()=>setScoreDensity(score.horizontalDensity/1.2);$('densityReset').onclick=()=>setScoreDensity(1);
// Seeded palettes obey basic harmony relationships and remain individually editable.
let paletteSeed=0,staffColors=null,staffPaletteInitialized=false;
function updatePalette(){const n=cycle?.roles.length||+ $('voices').value,enabled=$('staffColorToggle').checked,scheme=$('staffPalette').value,colors=StaffColorTheory.palette(config.seed+':'+paletteSeed,scheme,n);if(staffColors?.length===n&&enabled&&$('staffColorSwatches').dataset.manual==='1')colors.splice(0,colors.length,...staffColors);staffColors=enabled?colors:null;score.setStaffColors(staffColors);const holder=$('staffColorSwatches');holder.replaceChildren();if(enabled)for(let i=0;i<n;i++){const label=document.createElement('label'),swatch=document.createElement('input');swatch.type='color';swatch.setAttribute('aria-label','Voice '+(i+1)+' color');const convert=(color)=>{const div=document.createElement('div');div.style.color=color;document.body.append(div);const rgb=getComputedStyle(div).color.match(/\d+/g).slice(0,3).map(Number);div.remove();return '#'+rgb.map(x=>x.toString(16).padStart(2,'0')).join('');};swatch.value=convert(colors[i]);swatch.oninput=()=>{staffColors[i]=swatch.value;holder.dataset.manual='1';score.setStaffColors(staffColors);renderDock();};label.append(swatch);holder.append(label);}else holder.dataset.manual='0';renderDock();}
$('staffColorToggle').onchange=()=>{if(!$('staffColorToggle').checked)$('staffColorSwatches').dataset.manual='0';updatePalette();};$('staffPalette').onchange=()=>{$('staffColorSwatches').dataset.manual='0';updatePalette();};$('randomizePalette').onclick=()=>{paletteSeed++;$('staffColorSwatches').dataset.manual='0';updatePalette();};
document.addEventListener('contextmenu',e=>{const voice=dockTarget(e);if(!voice&&!e.target.closest('#voiceDock,#scoreShell,.score-stage'))return;if(e.target.closest('button,select,input')&&!voice)return;e.preventDefault();contextTarget=voice?+voice.dataset.voice:-1;const menu=$('voiceContextMenu');$('addVoiceMenu').hidden=+$('voices').value>=8;$('removeVoiceMenu').hidden=contextTarget<0||+$('voices').value<=2;menu.hidden=false;menu.style.left=Math.max(5,Math.min(innerWidth-195,e.clientX))+'px';menu.style.top=Math.max(5,Math.min(innerHeight-320,e.clientY))+'px';});
document.addEventListener('pointerdown',e=>{if(!e.target.closest('#voiceContextMenu'))hideContext();});
if('serviceWorker'in navigator)navigator.serviceWorker.register('sw.js').catch(()=>{});
reset();
// Read-only diagnostic surface for reproducibility and runtime verification.
window.continuum={get cycle(){return cycle;},get playing(){return playing;},get tick(){return lastTick;},get nextCycle(){return nextCycle;},get audio(){return audio;}};
const modelContext=document.modelContext;
if(modelContext?.registerTool){
 const read=()=>({ready:!!cycle&&!loading,playing,settings:config,cycle:cycle?cycle.index+1:null,measure:cycle?Fugue.measureAt(cycle,lastTick).index+1:1,section:lastSection?.name,key:lastSection?.key.name,meter:lastSection?.meter.id,current:cycle?Object.fromEntries(['tempo','density','adventure'].map(p=>[p,Fugue.automated(config,p,cycle.index+lastTick/cycle.length)])):null,audit:cycle?.audit,nextCycleReady:!!nextCycle,instruments:names,samplesLoaded:audio.instruments.filter(Boolean).length,fallback:audio.fallback,volume:audio.volume,reverb:audio.reverb,sampleBank:audio.bank});
 const choices={counterpointStyle:['strict','balanced','free'],formMode:['canonical','guided','free'],meter:Array.from({length:16},(_,i)=>[2,4,8,16].map(d=>(i+1)+'/'+d)).flat(),meterMode:['fixed','cycle','section','bar','bars','range'],microtiming:['off','light','expressive'],tempoMode:['fixed','drift','cycle','section','bar','bars','range'],densityMode:['fixed','drift','cycle','section','bar','bars','range'],adventureMode:['fixed','drift','cycle','section','bar','bars','range']};
 const bounds={voices:[2,8],tempo:[48,144],density:[15,100],exploration:[0,100]};
 const properties={seed:{type:'string',minLength:1,maxLength:80},...Object.fromEntries(Object.entries(bounds).map(([k,[minimum,maximum]])=>[k,{type:k==='voices'?'integer':'number',minimum,maximum}])),...Object.fromEntries(Object.entries(choices).map(([k,values])=>[k,{type:'string',enum:values}]))};
 const configure=async input=>{if(!input||typeof input!=='object'||Array.isArray(input))throw new Error('Expected settings object');for(const [k,v]of Object.entries(input)){if(k==='seed'){if(typeof v!=='string'||!v.trim()||v.length>80)throw new Error('Seed must have 1–80 characters');}else if(choices[k]){if(!choices[k].includes(v))throw new Error('Invalid choice: '+k);}else if(!bounds[k]||typeof v!=='number'||!Number.isFinite(v)||v<bounds[k][0]||v>bounds[k][1]||k==='voices'&&!Number.isInteger(v))throw new Error('Invalid setting: '+k);}for(const [k,v]of Object.entries(input)){$(k==='exploration'?'adventure':k).value=String(v);if(choices[k])config[k]=v;}for(const id of ['tempo','density','adventure','thematicInterplay','callResponse','development'])$(id+'Value').textContent=$(id).value+(id==='tempo'?'':'%');await applySettings();return read();};
 const tools=[{name:'read_fugue',title:'Read the fugue',description:'Read current settings, meter, automatic evolution, playback state and counterpoint audit.',inputSchema:{type:'object',properties:{},additionalProperties:false},annotations:{readOnlyHint:true},execute:async()=>read()},{name:'configure_fugue',title:'Configure the fugue',description:'Set seed, voices, meter, expressive timing and automatic evolution, without restarting playback. Changes apply to unheard music ahead of the visible score.',inputSchema:{type:'object',properties,additionalProperties:false},annotations:{readOnlyHint:false},execute:configure}];
 for(const tool of tools)try{Promise.resolve(modelContext.registerTool(tool)).catch(()=>{});}catch{}
}
