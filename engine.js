/* Continuum 1.1. Pure deterministic composition: no wall clock, browser, or audio state. */
(function(root){
'use strict';
const mod=(a,b)=>((a%b)+b)%b;
function hash(s){let h=2166136261;for(const c of String(s)){h^=c.charCodeAt(0);h=Math.imul(h,16777619);}return h>>>0;}
function random(seed){let a=hash(seed);return()=>{a+=0x6D2B79F5;let t=a;t=Math.imul(t^(t>>>15),t|1);t^=t+Math.imul(t^(t>>>7),t|61);return((t^(t>>>14))>>>0)/4294967296;};}
const MINOR=[0,2,3,5,7,8,10],MAJOR=[0,2,4,5,7,9,11];
const KEYS=[{pc:2,minor:true,name:'D minor',vf:'Dm',letter:1},{pc:7,minor:true,name:'G minor',vf:'Gm',letter:4},{pc:9,minor:true,name:'A minor',vf:'Am',letter:5},{pc:0,minor:true,name:'C minor',vf:'Cm',letter:0},{pc:4,minor:true,name:'E minor',vf:'Em',letter:2},{pc:5,minor:false,name:'F major',vf:'F',letter:3},{pc:0,minor:false,name:'C major',vf:'C',letter:0},{pc:7,minor:false,name:'G major',vf:'G',letter:4},{pc:2,minor:false,name:'D major',vf:'D',letter:1},{pc:10,minor:false,name:'B♭ major',vf:'Bb',letter:6},{pc:3,minor:false,name:'E♭ major',vf:'Eb',letter:2},{pc:5,minor:true,name:'F minor',vf:'Fm',letter:3},{pc:11,minor:true,name:'B minor',vf:'Bm',letter:6}];
KEYS.push({pc:9,minor:false,name:'A major',vf:'A',letter:5},{pc:6,minor:false,name:'F♯ major',vf:'F#',letter:3},{pc:11,minor:false,name:'B major',vf:'B',letter:6},{pc:4,minor:false,name:'E major',vf:'E',letter:2},{pc:8,minor:false,name:'A♭ major',vf:'Ab',letter:5});
const MODES={major:[0,2,4,5,7,9,11],minor:[0,2,3,5,7,8,10],dorian:[0,2,3,5,7,9,10],phrygian:[0,1,3,5,7,8,10],lydian:[0,2,4,6,7,9,11],mixolydian:[0,2,4,5,7,9,10],aeolian:[0,2,3,5,7,8,10],locrian:[0,1,3,5,6,8,10],harmonic_minor:[0,2,3,5,7,8,11],melodic_minor:[0,2,3,5,7,9,11]};
const ROOTS=['C','C♯','D','E♭','E','F','F♯','G','A♭','A','B♭','B'];
function makeKey(pc,mode='major'){pc=mod(pc,12);const intervals=MODES[mode]||MAJOR,relative={major:0,minor:3,aeolian:3,dorian:10,phrygian:8,lydian:7,mixolydian:5,locrian:1,harmonic_minor:3,melodic_minor:3}[mode]||0,signature=['C','Db','D','Eb','E','F','F#','G','Ab','A','Bb','B'][mod(pc+relative,12)],letter=[0,0,1,2,2,3,3,4,5,5,6,6][pc];return{pc,minor:intervals[2]===3,mode,intervals,name:ROOTS[pc]+' '+mode.replace('_',' '),vf:signature,letter};}
function keyAt(pc,minor,mode){return makeKey(pc,mode||(minor?'minor':'major'));}
function pitch(deg,key){const scale=key.intervals||(key.minor?MINOR:MAJOR);return 60+key.pc+12*Math.floor(deg/7)+scale[mod(deg,7)];}
function nearestDegree(midi,key){let best=0,d=Infinity;for(let i=-28;i<28;i++){const e=Math.abs(pitch(i,key)-midi);if(e<d){d=e;best=i;}}return best;}
function consonant(a,b,lowest){const d=mod(Math.abs(a-b),12);return [0,3,4,7,8,9].includes(d)||(d===5&&Math.min(a,b)>lowest);}
function perfect(a,b){return [0,7].includes(mod(Math.abs(a-b),12));}
function parallel(a,b,pa,pb){return pa!=null&&pb!=null&&a!==pa&&b!==pb&&Math.sign(a-pa)===Math.sign(b-pb)&&perfect(a,b)&&mod(Math.abs(a-b),12)===mod(Math.abs(pa-pb),12);}
function makeMeter(num,den){num=clamp(Math.round(+num)||4,1,16);den=[2,4,8,16].includes(+den)?+den:4;let groups=[];if(den<=4||num<=3)groups=Array(num).fill(1);else if(num%3===0)groups=Array(num/3).fill(3);else{let left=num;while(left>0){const group=left===3?3:Math.min(2,left);groups.push(group);left-=group;}}return{id:num+'/'+den,num,den,groups,barTicks:num*16/den,groupTicks:groups.map(x=>x*16/den)};}
const clamp=(v,a,b)=>Math.max(a,Math.min(b,v));
const METERS=[['3/4',3,4],['4/4',4,4],['5/4',5,4],['6/8',6,8],['7/8',7,8],['9/8',9,8],['12/8',12,8]].map(([,n,d])=>makeMeter(n,d));
function validateTheme(theme){if(theme==null)return null;if(!theme||!Array.isArray(theme.notes)||theme.notes.length<4||theme.notes.length>12)throw new Error('Use 4–12 notes in a theme.');const notes=theme.notes.map(x=>{if(!Number.isInteger(x.d)||Math.abs(x.d)>21||![-1,0,1].includes(x.alter||0)||![1,2,3,4,6,8,12,16].includes(x.dur))throw new Error('Invalid theme note or rhythm.');return x.rest?{d:0,alter:0,dur:x.dur,rest:true}:{d:x.d,alter:x.alter||0,dur:x.dur};});const key=makeKey(theme.pc??0,theme.mode in MODES?theme.mode:'minor'),ps=notes.filter(x=>!x.rest).map(x=>pitch(x.d,key)+x.alter);if(ps.length<3)throw new Error('A theme needs at least three sounded notes.');if(Math.max(...ps)-Math.min(...ps)>16)throw new Error('Keep the theme within a major tenth (16 semitones).');return{pc:key.pc,mode:key.mode,notes};}
function settings(s={}){const number=(v,f,a,b)=>clamp(Number.isFinite(+v)?+v:f,a,b),mode=k=>['fixed','drift','cycle','section','bar','bars','range'].includes(s[k])?s[k]:'fixed',meter=String(s.meter||'4/4').split('/');const result={seed:String(s.seed??'counterpoint'),voices:Math.round(number(s.voices,4,2,8)),density:number(s.density,.56,.15,1),adventure:number(s.adventure,.24,0,1),themeCount:Math.round(number(s.themeCount,1,1,3)),thematicInterplay:number(s.thematicInterplay,.28,0,1),callResponse:number(s.callResponse,.24,0,1),development:number(s.development,.28,0,1),counterpointStyle:['strict','balanced','free'].includes(s.counterpointStyle)?s.counterpointStyle:'balanced',formMode:['canonical','guided','free'].includes(s.formMode)?s.formMode:'guided',mood:number(s.mood,0,-1,1),openingMode:['classic','varied','prologue'].includes(s.openingMode)?s.openingMode:'classic',openingTexture:['auto','solo','octaves','chordal','countertheme','contrapuntal','pedal'].includes(s.openingTexture)?s.openingTexture:'solo',openingVoices:['auto','2','3','4','all'].includes(String(s.openingVoices))?String(s.openingVoices):'auto',tempo:number(s.tempo,84,48,144),meter:makeMeter(meter[0],meter[1]).id,meterMode:mode('meterMode'),keyMode:['form','fixed','cycle','section','bar','bars','range'].includes(s.keyMode)?s.keyMode:'form',tonic:s.tonic==='auto'||s.tonic==null?'auto':Math.round(number(s.tonic,0,0,11)),scale:s.scale in MODES?s.scale:'auto',microtiming:['off','light','expressive'].includes(s.microtiming)?s.microtiming:'light',tempoMode:mode('tempoMode'),densityMode:mode('densityMode'),adventureMode:mode('adventureMode'),variation:['wander','pendulum','sequence','contrast'].includes(s.variation)?s.variation:'wander',theme:validateTheme(s.theme),secondaryThemes:Array.isArray(s.secondaryThemes)?s.secondaryThemes.slice(0,2).map(x=>x?validateTheme(x):null):[]};
 for(const p of ['meter','key','tempo','density','adventure']){result[p+'Every']=Math.round(number(s[p+'Every'],4,1,64));result[p+'Min']=Math.round(number(s[p+'Min'],p==='meter'?3:2,1,64));result[p+'Max']=Math.max(result[p+'Min'],Math.round(number(s[p+'Max'],p==='meter'?12:8,1,64)));result[p+'Depth']=number(s[p+'Depth'],{meter:1,key:1,tempo:7,density:9,adventure:12}[p],0,p==='tempo'?40:p==='key'||p==='meter'?1:50);}
 // Meter choice bounds and interval bounds are independent.
 result.meterLow=Math.round(number(s.meterLow,3,1,16));result.meterHigh=Math.max(result.meterLow,Math.round(number(s.meterHigh,12,1,16)));return result;}
function epoch(s,param,bar,section=0,index=0){const mode=s[param+'Mode'];if(mode==='bar')return bar;if(mode==='bars')return Math.floor(bar/s[param+'Every']);if(mode==='range'){let at=0,k=0;const r=random(s.seed+':interval:'+param+':'+index);while(at<=bar){at+=s[param+'Min']+Math.floor(r()*(s[param+'Max']-s[param+'Min']+1));if(at<=bar)k++;}return k;}if(mode==='section')return section;if(mode==='cycle')return index;return 0;}
function variation(s,param,k,index=0){if(!k)return 0;if(s.variation==='pendulum')return Math.sin(k*Math.PI/2);if(s.variation==='sequence')return ((k%7)-3)/3;if(s.variation==='contrast')return k%2?1:-1;return random(s.seed+':variation:'+param+':'+index+':'+k)()*2-1;}
function automated(s,param,position,bar=0,section=0){const mode=s[param+'Mode'],base=s[param],amplitude=s[param+'Depth']/(param==='tempo'?1:100);let delta=0;if(mode==='drift'){const phase=hash(s.seed+':drift:'+param)/4294967296*Math.PI*2;delta=.7*Math.sin(position*Math.PI*2/3+phase)+.3*Math.sin(position*Math.PI*2/7+phase*.5);}else if(mode!=='fixed')delta=variation(s,param,epoch(s,param,bar,section,Math.floor(position)),mode==='cycle'?0:Math.floor(position));return clamp(base+amplitude*delta,param==='tempo'?48:param==='density'?.15:0,param==='tempo'?144:1);}
function metric(sec,t){const measure=sec.measures?.find(m=>t>=m.t&&t<m.end)||sec.measures?.at(-1),meter=measure?.meter||sec.meter,local=mod(t-(measure?.t??sec.start),meter.barTicks);let start=0;for(const length of meter.groupTicks){if(local<start+length)return{local,groupStart:start,within:local-start,length,strong:local===start};start+=length;}return{local,groupStart:0,within:0,length:1,strong:true};}
function adaptSubject(subject,meter){const target=Math.round(meter.barTicks*2),total=subject.reduce((sum,x)=>sum+x.dur,0);if(target<subject.length)throw new Error('Theme does not fit the phrase.');if(target===total)return subject.map(x=>({...x}));const quantum=target>=subject.length*2&&target%2===0?2:1;let used=0,source=0;return subject.map((x,i)=>{source+=x.dur;const remaining=subject.length-i-1,end=i===subject.length-1?target:clamp(Math.round(source/total*target/quantum)*quantum,used+quantum,target-remaining*quantum),dur=end-used;used=end;return{...x,dur};});}
function measureAt(c,t){let lo=0,hi=c.measures.length-1;while(lo<hi){const mid=Math.ceil((lo+hi)/2);if(c.measures[mid].t<=t)lo=mid;else hi=mid-1;}return c.measures[lo];}
function roles(n){const names=n===4?['Soprano','Alto','Tenor','Bass']:Array.from({length:n},(_,i)=>i===0?'Soprano':i===n-1?'Bass':`Voice ${i+1}`);return names.map((name,i)=>{const center=Math.round(77-i*32/(n-1));return{name,center,low:Math.max(36,center-10),high:Math.min(91,center+11),clef:center<59?'bass':'treble'};});}
// Rule-based subject grammar. The seed selects a rhetorical archetype; no random
// draws, entropy, or weighted dice participate in choosing a generated theme.
// Each cell occupies half a 4/4 measure (eight sixteenth-note ticks). A phrase
// is antecedent/answer/development/cadence, not an unstructured note stream.
const SUBJECT_RHYTHMS=[
 [[4,4],[2,1,1,4],[2,2,-2,2],[4,4]],
 [[4,2,2],[3,1,4],[-4,4],[2,2,4]],
 [[2,2,4],[2,2,4],[4,1,1,2],[4,4]],
 [[4,4],[2,2,4],[-2,2,4],[4,4]],
 [[2,1,1,4],[4,4],[4,1,1,2],[4,4]],
 [[6,2],[-2,2,4],[2,4,2],[4,4]],
 [[4,4],[2,2,4],[1,1,2,4],[-4,4]],
 [[2,2,4],[2,-2,4],[2,1,1,4],[4,4]],
 [[4,4],[3,1,4],[3,1,4],[4,4]],
 [[2,4,2],[-4,4],[2,2,4],[2,2,4]],
 [[4,-2,2],[2,1,1,4],[4,4],[4,4]],
 [[2,2,2,2],[4,4],[-4,4],[4,4]],
 [[4,4],[4,2,2],[4,2,2],[2,2,4]],
 [[4,4],[-2,2,4],[3,1,4],[2,2,4]],
 [[4,2,2],[4,4],[2,1,1,4],[4,4]],
 [[2,2,4],[3,1,4],[4,4],[2,2,4]]
];
// Scale-degree contours: approach, apex, answer, and tonic close.
// The cadential degrees (last -1,0 or 1,0) are deliberate leading/neighbor motion.
const SUBJECT_CONTOURS=[
 [0,1,2,4,3,2,1,2,3,2,1,0],
 [0,2,4,3,2,1,2,3,1,0,-1,0],
 [0,0,0,1,2,0,2,3,2,1,-1,0],
 [0,4,3,2,1,2,3,2,1,2,1,0],
 [0,1,2,1,2,3,4,3,2,1,-1,0],
 [0,2,1,0,2,4,3,2,3,1,-1,0],
 [0,3,2,1,0,1,2,4,3,2,1,0],
 [0,1,0,2,1,0,3,2,1,0,-1,0],
 [0,4,4,3,2,1,2,2,1,0,-1,0],
 [0,2,3,4,2,3,2,1,2,1,-1,0],
 [0,1,2,0,1,2,2,3,4,3,1,0],
 [0,0,1,2,3,2,1,2,3,2,1,0]
];
function subjectFor(seed,family){
 // Integer rotations give a reproducible compositional vocabulary instead of
 // invoking the random generator. The family drives thematic evolution.
 const basis=hash(String(seed)+':rhetoric'),rhythmId=mod(basis+family*7,SUBJECT_RHYTHMS.length);
 const contourId=mod(Math.floor(basis/53)+family*5,SUBJECT_CONTOURS.length);
 const rhythm=SUBJECT_RHYTHMS[rhythmId].flatMap(cell=>cell.map(x=>({dur:Math.abs(x),rest:x<0})));
 const sounding=rhythm.filter(x=>!x.rest),contour=SUBJECT_CONTOURS[contourId];
 let noteIndex=0;
 return rhythm.map(x=>{
  if(x.rest)return{d:0,alter:0,dur:x.dur,rest:true};
  // Equal-interval contour resampling keeps melodic logic when rests or varied
  // subdivisions change the number of sounded notes. Preserve tonic cadence.
  const i=noteIndex++,position=i*(contour.length-1)/(sounding.length-1);
  const degree=contour[Math.round(position)];
  return{d:degree,dur:x.dur};
 });
}
function plan(input,index){const s=settings(input),r=random(s.seed+':plan:'+index),original=KEYS[hash(s.seed)%10],influence=random(s.seed+':mood:'+index)(),autoMode=original.minor?'minor':'major',mode=s.theme?.mode||(s.scale==='auto'&&s.mood!==0&&influence<Math.abs(s.mood)?(s.mood<0?'minor':'major'):(s.scale==='auto'?autoMode:s.scale)),baseRoot=s.theme?.pc??(s.tonic==='auto'?original.pc:s.tonic),root=baseRoot+(s.keyMode==='cycle'?[0,7,5,2,10,3][index%6]:0),home=makeKey(root,mode),family=s.openingMode==='classic'?Math.floor(index/4):index,subject=s.theme?.notes||subjectFor(s.seed,family),themes=[subject];
 for(let k=1;k<s.themeCount;k++){let candidate=s.secondaryThemes[k-1]?.notes||subjectFor(s.seed+':independent:'+k,family+9*k);if(candidate.every((note,i)=>note.d===subject[i]?.d&&note.dur===subject[i]?.dur))candidate=candidate.map((note,i)=>i>0&&i<10?{...note,d:Math.max(-2,Math.min(5,note.d+(i%2?1:-1)))}:note);themes.push(candidate);} 
 const dom=keyAt(root+7,home.minor,mode),rel=keyAt(root+(home.minor?3:9),!home.minor),sub=keyAt(root+5,home.minor,mode),remote=keyAt(root+(home.minor?8:4),false);let at=0;const sections=[],measures=[],[num,den]=s.meter.split('/').map(Number),baseMeter=makeMeter(num,den);
 const meterFor=(bar,ordinal,kind)=>{if(s.meterMode==='cycle')return index===0?baseMeter:makeMeter(s.meterLow+mod(index-1+hash(s.seed+':meter'),s.meterHigh-s.meterLow+1),den);if(['return','coda'].includes(kind)||s.meterMode==='fixed')return baseMeter;const k=epoch(s,'meter',bar,ordinal,index);if(k===0)return baseMeter;const count=s.meterHigh-s.meterLow+1,n=s.meterLow+mod(Math.round((variation(s,'meter',k,s.meterMode==='cycle'?0:index)+1)/2*(count-1)),count);return makeMeter(n,den);};
 const keyFor=(bar,ordinal,kind,fallback)=>{if(['return','coda'].includes(kind)||bar===0)return home;if(['fixed','cycle'].includes(s.keyMode))return home;if(s.keyMode==='form')return fallback;const k=epoch(s,'key',bar,ordinal,index);if(k===0)return home;const steps=[0,7,5,home.minor?3:9,2,10];return makeKey(root+steps[mod(Math.round((variation(s,'key',k,s.keyMode==='cycle'?0:index)+1)*2.5),steps.length)],mode);};
 const add=(name,kind,bars,key,strictness)=>{const ordinal=sections.length,firstBar=measures.length,minimum=s.meterMode==='fixed'?baseMeter.barTicks:Math.min(baseMeter.barTicks,s.meterLow*16/den),phraseBars=Math.max(2,Math.ceil(subject.length*2/minimum/2)*2),stretch=phraseBars/2;bars=Math.round(bars*stretch);const effectiveStrictness=s.counterpointStyle==='strict'?Math.max(.985,strictness):s.counterpointStyle==='balanced'?Math.max(.955,strictness):strictness;
 const q={name,kind,start:at,end:at,bars,firstBar,key,strictness:effectiveStrictness,meter:baseMeter,phraseBars,measures:[],techniques:[]};for(let b=0;b<bars;b++){const meter=meterFor(firstBar+b,ordinal,kind),activeKey=keyFor(firstBar+b,ordinal,kind,key),m={index:measures.length,t:at,end:at+meter.barTicks,meter,key:activeKey,section:ordinal};measures.push(m);q.measures.push(m);at=m.end;}q.end=at;q.meter=q.measures[0].meter;q.key=q.measures[0].key;sections.push(q);};
 if(s.openingMode==='prologue'&&index>0)add('Prelude','episode',2+2*(index%2),home,.88);
 add('Exposition','exposition',s.voices*2+2,home,.98);
 let middle=[['episode',dom],['middle',automated(s,'adventure',index+.25)>.68?remote:rel],['inversion',sub],['augmentation',rel],['diminution',dom],['stretto',home],['pedal',home]];
 // Separate strict contrapuntal rules from the chosen architectural form.
  // Canonical retains a stable classical sequence; guided varies within a
  // preparation / development / intensification trajectory.
  if(s.formMode==='guided'){
   for(const [a,b] of [[0,1],[2,3],[4,5]])if(r()<.5)[middle[a],middle[b]]=[middle[b],middle[a]];
  }else if(s.formMode==='free'){
   for(let i=middle.length-1;i>0;i--){const j=Math.floor(r()*(i+1));[middle[i],middle[j]]=[middle[j],middle[i]];}
   if(index%3===2)middle.splice(2,0,['episode',sub]);
  }
 middle.forEach(([kind,key],i)=>add('Development '+(i+1),kind,kind==='pedal'?4:kind==='diminution'?4:kind==='episode'?4+2*Math.floor(r()*3):8+(r()>.7?2:0),key,kind==='episode'?.88-s.adventure*.15:.94));
 add('Return','return',6+2*Math.floor(r()*3),home,.99);add('Tonic close','coda',4,home,1);
 return{home,sections,measures,length:at,subject,themes,family,r};
}
function compose(input,index=0){const s=settings(input),p=plan(s,index),r=random(s.seed+':notes:'+index+':'+s.voices+':'+s.adventure),rs=roles(s.voices),n=s.voices,T=p.length;
 const fixed=Array.from({length:n},()=>Array(T).fill(null)),events=Array.from({length:n},()=>[]),entries=[];
 const secAt=Array(T);for(const sec of p.sections)for(const m of sec.measures){const local={...sec,key:m.key,meter:m.meter,measure:m};for(let t=m.t;t<m.end;t++)secAt[t]=local;}
 const barAt=(sec,b)=>b>=sec.bars?sec.end:sec.measures[Math.max(0,Math.floor(b))].t+Math.round((b%1)*sec.measures[Math.floor(b)].meter.barTicks);
 const localBar=(sec,t)=>{const m=measureAt(p,t);return m.index-sec.firstBar+(t-m.t)/m.meter.barTicks;};
 const order=[Math.min(1,n-1),0,...Array.from({length:n-2},(_,i)=>i+2)];
 // Vary the leading role from the very first generated cycle. The classic setting
 // retains its familiar traditional entry order for backwards compatibility.
 if(s.openingMode!=='classic'){
  const introRand=random(s.seed+':openingOrder:'+index);
  for(let i=order.length-1;i>0;i--){const j=Math.floor(introRand()*(i+1));[order[i],order[j]]=[order[j],order[i]];}
  if(index>0&&index%2===0){const first=order.shift();order.push(first);}
 }
 const opening={lead:order[0],order:order.slice(),texture:'solo',partners:[],themeIds:[]};
 function makeEntry(v,t,sec,type='subject',factor=1,shift=0,themeId=0){const firstKey=secAt[t]?.key||sec.key;let degree0=nearestDegree(rs[v].center-3,firstKey);degree0-=mod(degree0,7);const inversion=type==='inversion',startBar=localBar(sec,t),requiredBars=sec.phraseBars*factor;if(startBar+requiredBars>sec.bars+1e-8)return [];const end=barAt(sec,startBar+requiredBars),target=end-t;const motif=p.themes[themeId]||p.subject;if(target<motif.length)return [];const reversed=type==='retrograde'||type==='retrograde-inversion';let seq=adaptSubject(reversed?[...motif].reverse():motif,{barTicks:target/2}).map(x=>({...x,d:inversion||type==='retrograde-inversion'?-x.d:x.d,alter:(x.alter||0)*(inversion||type==='retrograde-inversion'?-1:1)}));if(type==='answer')seq=seq.map((x,i)=>({...x,d:x.d+4-(i===1&&x.d===4?1:0)}));let pos=t;const raw=seq.map(x=>{const key=secAt[pos]?.key||firstKey,m=x.rest?null:pitch(degree0+x.d+shift,key)+x.alter;pos+=x.dur;return m;});const sounded=raw.filter(x=>x!==null);if(!sounded.length)return [];let offset=0;while(Math.max(...sounded)+offset>rs[v].high)offset-=12;while(Math.min(...sounded)+offset<rs[v].low)offset+=12;pos=t;return seq.map((x,i)=>{let midi=raw[i]===null?null:raw[i]+offset;if(midi!==null){while(midi>rs[v].high)midi-=12;while(midi<rs[v].low)midi+=12;}const q={t:pos,dur:x.dur,pitch:midi,rest:!!x.rest,voice:v,kind:type,key:secAt[pos]?.key||firstKey,degree:degree0+x.d+shift,themeId};pos+=x.dur;return q;}).filter(x=>x.t+x.dur<=sec.end);}
 function canPlace(v,seq){if(!seq.length||seq.some(x=>fixed[v].slice(x.t,x.t+x.dur).some(Boolean)))return false;let conflicts=0,comparisons=0;for(const e of seq){for(let k=e.t;k<e.t+e.dur;k+=2){for(let u=0;u<n;u++){if(u===v||!fixed[u][k])continue;const f=fixed[u][k];if(e.rest||f.rest)continue;const low=Math.min(e.pitch,f.pitch);if(k%4===0){comparisons++;if(!consonant(e.pitch,f.pitch,low)||u<v&&f.pitch<e.pitch||u>v&&f.pitch>e.pitch)conflicts++;}}}}return !comparisons||conflicts/comparisons<(s.counterpointStyle==='strict'?.06:s.counterpointStyle==='balanced'?.16:.24);}
 function putEntry(v,t,sec,type='subject',factor=1,shift=0,options={}){const themeId=options.themeId||0,seq=makeEntry(v,t,sec,type,factor,shift,themeId);if(!seq.length||seq.some(x=>fixed[v].slice(x.t,x.t+x.dur).some(Boolean)))return false;if((options.checkHarmony||(s.counterpointStyle==='strict'&&sec.kind!=='exposition'&&type!=='stretto'))&&!canPlace(v,seq))return false;for(const e of seq)for(let k=e.t;k<e.t+e.dur;k++)fixed[v][k]=e;entries.push({voice:v,t,end:seq.at(-1)?.t+seq.at(-1)?.dur||t,type,key:sec.key.name,themeId,relation:options.relation||null,pair:options.pair??null});return true;}
 function placeDialogue(sec,serial){const P=sec.phraseBars;if(sec.bars<2*P||p.themes.length<2||s.callResponse<=0)return false;const t1=barAt(sec,0),t2=barAt(sec,P),themeId=1+serial%(p.themes.length-1),pair=sec.firstBar+serial;
   for(let a=0;a<n;a++)for(let b=0;b<n;b++){if(a===b)continue;const ca=makeEntry(a,t1,sec,'subject',1,0,serial%2?themeId:0),cb=makeEntry(b,t2,sec,serial%3===0?'retrograde':'answer',1,0,themeId);if(!canPlace(a,ca)||!canPlace(b,cb))continue;const ok=putEntry(a,t1,sec,'subject',1,0,{themeId:serial%2?themeId:0,checkHarmony:true,relation:'call',pair});if(ok&&putEntry(b,t2,sec,serial%3===0?'retrograde':'answer',1,0,{themeId,checkHarmony:true,relation:'response',pair}))return true;if(ok){for(const e of ca)for(let k=e.t;k<e.t+e.dur;k++)if(fixed[a][k]===e)fixed[a][k]=null;entries.pop();}}
   return false;
 }
 let dialogues=0;for(const sec of p.sections){const st=sec.start,P=sec.phraseBars,at=b=>barAt(sec,b);
  if(sec.kind==='exposition'){
   order.forEach((v,j)=>putEntry(v,at(j*P),sec,j%2?'answer':'subject'));
   // Build complete supporting phrases instead of the previous single-note placeholder.
   // All opening lines end at the first scheduled solo exposition entry, so the
   // later subject statements retain their exact onset and duration.
   const textureOptions=n>=5?['octaves','chordal','countertheme','contrapuntal','pedal','solo']:
    ['solo','octaves','chordal','countertheme','contrapuntal'];
   const texture=s.openingTexture==='auto'?textureOptions[Math.floor(random(s.seed+':texture:'+index)()*textureOptions.length)]:s.openingTexture;
   opening.texture=texture;opening.lines=[];opening.requestedVoices=s.openingVoices;
   const phraseEnd=at(P),leadVoice=opening.lead,leadNotes=[];
   for(let t=st;t<phraseEnd;t++){const e=fixed[leadVoice][t];if(e&&!e.rest&&e.t===t)leadNotes.push(e);}
   const atLead=t=>fixed[leadVoice][Math.min(phraseEnd-1,Math.max(st,t))];
   const harmonyAt=(voice,t,dur,target,lastPitch,themeKind)=>{
    const key=secAt[t].key,other=Array.from({length:n},(_,u)=>u).filter(u=>u!==voice);
    const possible=[];
    for(let midi=rs[voice].low;midi<=rs[voice].high;midi++){
     const scale=nearestDegree(midi,key);if(pitch(scale,key)!==midi)continue;
     if(lastPitch!==null&&Math.abs(midi-lastPitch)>9)continue;
     let conflict=0,crossing=0,collisions=0;
     for(let tick=t;tick<t+dur;tick+=2){for(const u of other){const sounding=fixed[u][tick];if(!sounding||sounding.rest)continue;
      if(!consonant(midi,sounding.pitch,Math.min(midi,sounding.pitch)))conflict+=u===leadVoice?3:1;
      if((voice<u&&midi<sounding.pitch)||(voice>u&&midi>sounding.pitch))crossing++;
      if(midi===sounding.pitch)collisions++;
     }}
     const score=conflict*3+crossing*3+collisions*.45+Math.abs(midi-target)*.23+Math.abs(midi-rs[voice].center)*.08+
       (lastPitch===null?0:Math.abs(midi-lastPitch)*.19+(midi===lastPitch?1.1:0));
     possible.push({midi,score});
    }
    if(!possible.length)return null;
    possible.sort((a,b)=>a.score-b.score||a.midi-b.midi);
    return possible[0].midi;
   };
   const setOpeningLine=(voice,seq,kind,themeId=-1)=>{
    if(seq.length<2||seq[0].t!==st||seq.at(-1).t+seq.at(-1).dur<phraseEnd)return false;
    if(seq.some(e=>!e.rest&&(e.pitch<rs[voice].low||e.pitch>rs[voice].high)||e.dur<1||e.t<st||e.t+e.dur>phraseEnd||fixed[voice].slice(e.t,e.t+e.dur).some(Boolean)))return false;
    let reservedUntil=st;for(const e of seq){if(e.t>reservedUntil){const rest={t:reservedUntil,dur:e.t-reservedUntil,voice,rest:true,kind:'openingRest'};for(let k=rest.t;k<e.t;k++)fixed[voice][k]=rest;}for(let tick=e.t;tick<e.t+e.dur;tick++)fixed[voice][tick]=e;reservedUntil=e.t+e.dur;}
    opening.partners.push(voice);opening.themeIds.push(themeId);
    opening.lines.push({voice,kind,themeId,notes:seq.filter(e=>!e.rest).length,start:st,end:phraseEnd});
    if(themeId>0)entries.push({voice,t:st,end:phraseEnd,type:'subject',key:sec.key.name,themeId,relation:'opening-countertheme',pair:null});
    return true;
   };
   if(leadNotes.length&&texture!=='solo'&&n>2){
    const mixRand=random(s.seed+':opening-orchestration:'+index+':'+texture);
    const octaveFit=v=>[-36,-24,-12,12,24,36].some(delta=>(v<leadVoice?delta>0:delta<0)&&leadNotes.every(e=>e.pitch+delta>=rs[v].low&&e.pitch+delta<=rs[v].high));
    const candidates=order.slice(1).sort((a,b)=>{
     if(texture==='octaves'){const av=octaveFit(a),bv=octaveFit(b);if(av!==bv)return av?-1:1;}
     const da=Math.abs(rs[a].center-rs[leadVoice].center),db=Math.abs(rs[b].center-rs[leadVoice].center);
     return da-db;
    });
    const autoCount=n>=7?2+Math.floor(mixRand()*Math.min(4,n-2)):n>=5?2+Math.floor(mixRand()*2):1;
    const count=s.openingVoices==='auto'?autoCount:s.openingVoices==='all'?n-1:Math.max(1,Math.min(n-1,+s.openingVoices-1));
    const number=Math.max(1,Math.min(n-1,count));
    const voices=[];
    if(texture==='pedal'&&leadVoice!==n-1)voices.push(n-1);
    for(const v of candidates)if(voices.length<number&&!voices.includes(v))voices.push(v);
    for(let j=0;j<voices.length;j++){
     const voice=voices[j],rand=random(s.seed+':openingline:'+index+':'+voice+':'+texture),pattern=j===0?[8,4,4,8,4,4]:j===1?[4,4,6,2,4,8,4]:[6,2,4,4,4,2,6,4];
     const shouldDouble=texture==='octaves';
     if(shouldDouble){
      const deltas=[-36,-24,-12,12,24,36,0].filter(delta=>
       (voice<leadVoice?delta>0:voice>leadVoice?delta<0:true)&&
       leadNotes.every(e=>e.pitch+delta>=rs[voice].low&&e.pitch+delta<=rs[voice].high));
      if(deltas.length){deltas.sort((a,b)=>Math.abs(rs[voice].center-(leadNotes[0].pitch+a))-Math.abs(rs[voice].center-(leadNotes[0].pitch+b)));
       const delta=deltas[0],seq=leadNotes.map(e=>({...e,voice,pitch:e.pitch+delta,kind:'openingOctave',velocity:.52,openingPartner:leadVoice}));
       if(setOpeningLine(voice,seq,'octaves'))continue;
      }
     }
     if(texture==='octaves')continue;
     if(texture==='countertheme'&&p.themes.length>1){
      const thematic=makeEntry(voice,st,sec,'subject',1,0,1),seq=[];let previous=null;
      for(const e of thematic){if(e.rest){seq.push(e);continue;}const midi=harmonyAt(voice,e.t,e.dur,e.pitch,previous,'countertheme');if(midi==null){seq.length=0;break;}
       seq.push({...e,pitch:midi,kind:'openingCountertheme',velocity:.59,openingPartner:leadVoice});previous=midi;}
      if(setOpeningLine(voice,seq,'countertheme',1))continue;
     }
     // Contrapuntal support alternates sustained tones and melodic motion,
     // and moving 3rds/6ths. It is independently rhythmic and range-aware.
     const line=[];let pos=st,previous=null,step=0;
     while(pos<phraseEnd&&step<256){
      const nextMeasure=secAt[pos]?.measure?.end||phraseEnd;
      const units=texture==='pedal'&&voice===n-1?
        [Math.max(4,nextMeasure-pos),Math.max(4,nextMeasure-pos)]:
        texture==='contrapuntal'?[4,2,6,4,8,2,6]:pattern;
      const base=units[(step+Math.floor(rand()*2))%units.length],dur=Math.max(1,Math.min(base,phraseEnd-pos,nextMeasure-pos));
      const active=atLead(pos),leadPitch=active?.pitch??rs[leadVoice].center;
      const key=secAt[pos].key,scaleDirection=voice<leadVoice?1:-1;
      const want=texture==='pedal'&&voice===n-1?
        pitch(nearestDegree(rs[voice].center,key)-mod(nearestDegree(rs[voice].center,key),7),key):
        leadPitch+(scaleDirection*(j===0?4:7))+(s.counterpointStyle==='free'?Math.round(Math.sin((step+j)*1.3)*2):0);
      const target=previous===null?want:previous*.55+want*.45;
      const midi=harmonyAt(voice,pos,dur,target,previous,texture);
      if(midi===null){line.length=0;break;}
      line.push({voice,t:pos,dur,pitch:midi,kind:texture==='pedal'&&voice===n-1?'openingPedal':'openingCounterline',key,velocity:.46+Math.min(.13,rand()*.13),openingPartner:leadVoice});
      previous=midi;pos+=dur;step++;
     }
     if(setOpeningLine(voice,line,texture==='octaves'?'octave-response':texture))continue;
    }
    // A texture with no viable accompaniment should not masquerade as polyphony.
    if(!opening.partners.length)opening.texture='solo';
   }
   if(p.themes.length>1&&s.thematicInterplay>.2){for(let j=1;j<order.length;j++){const v=order[j-1];if(putEntry(v,at(j*P),sec,j%2?'retrograde':'subject',1,0,{themeId:1+(j-1)%(p.themes.length-1),checkHarmony:true,relation:'countertheme'}))break;}}}
  if(['middle','inversion','return','augmentation','diminution'].includes(sec.kind)){const stride=sec.kind==='augmentation'?P*2:sec.kind==='diminution'?P/2:P;for(let b=0,j=0;b+stride<=sec.bars;b+=stride,j++){const v=order[(j+index+(sec.kind==='return'?2:0))%n],main=sec.kind==='return'?'subject':sec.kind==='middle'?(j%2?'answer':'subject'):sec.kind;const type=j>0&&sec.kind!=='return'&&r()<.35?['subject','inversion','diminution'][Math.floor(r()*3)]:main;putEntry(v,at(b),sec,type,type==='augmentation'?2:type==='diminution'?.5:1);}}
  if(sec.kind==='stretto'){for(let j=0;j<Math.min(n,4);j++){const v=order[(j+index)%n];let best=null;for(const delay of [P/2,P*3/8,P/4])for(const sh of [0,2,4]){const t=at(j*delay),seq=makeEntry(v,t,sec,'stretto',1,sh);let cost=0;for(const e of seq)if(!e.rest)for(let k=e.t;k<e.t+e.dur;k++)for(let u=0;u<n;u++)if(u!==v&&fixed[u][k]&&!fixed[u][k].rest){const b=fixed[u][k].pitch;cost+=consonant(e.pitch,b,Math.min(e.pitch,b))?0:4;cost+=v<u&&e.pitch<b||v>u&&e.pitch>b?4:0;}if(!best||cost<best.cost)best={cost,t,sh};}putEntry(v,best.t,sec,'stretto',1,best.sh);}putEntry(order[(index+1)%n],at(P*2.5),sec,'inversion');}
  if(sec.kind==='pedal'){const v=n-1;for(const m of sec.measures){const d=nearestDegree(rs[v].center,m.key),degree=d-mod(d,7)+4,e={voice:v,t:m.t,dur:m.end-m.t,pitch:pitch(degree,m.key),kind:'pedal',key:m.key};for(let k=e.t;k<e.t+e.dur;k++)fixed[v][k]=e;}putEntry(0,at(P/2),sec,'subject');}
  if(sec.kind==='coda'){for(let b=Math.floor(sec.bars*.75);b<sec.bars;b++){const m=sec.measures[b];for(const v of [n-1,...(b===sec.bars-1?[0]:[])]){const midi=m.key.pc+12*Math.round((rs[v].center-m.key.pc)/12),e={voice:v,t:m.t,dur:m.end-m.t,pitch:midi,kind:'cadence',key:m.key};for(let k=e.t;k<e.t+e.dur;k++)fixed[v][k]=e;}}}
  // Dialogues have priority over discretionary statements: a complete call finishes
  // before the answering subject begins in a different register.
  if(['episode','middle'].includes(sec.kind)&&p.themes.length>1&&s.callResponse>0&&(dialogues===0||r()<s.callResponse*.85)&&placeDialogue(sec,sec.firstBar))dialogues++;
  if(!['exposition','return','coda'].includes(sec.kind))for(let j=0;j<2;j++){
    const v=order[(index+j+2)%n],b=Math.floor(r()*Math.max(1,sec.bars-P)),variants=['inversion','diminution','subject','retrograde','retrograde-inversion'],type=variants[Math.floor(r()*(3+2*s.development))],secondary=p.themes.length>1&&r()<s.thematicInterplay?1+Math.floor(r()*(p.themes.length-1)):0;
    putEntry(v,at(b),sec,type,type==='diminution'?.5:1,0,{themeId:secondary,checkHarmony:secondary>0,relation:secondary>0?'interweave':null});
  }
  sec.techniques=[...new Set(entries.filter(e=>e.t>=sec.start&&e.t<sec.end).map(e=>e.type))];
 }
 const last=Array(n).fill(null),prevMel=Array(n).fill(null),held=Array(n).fill(null),until=Array(n).fill(0),lastAttack=Array(n).fill(-100);
 const totals={supportAttacks:0,rests:0,thematicAttacks:0,passing:0};
 function chordAt(t,sec){const local=t-sec.start,B=sec.meter.barTicks,bar=Math.floor(localBar(sec,t)),half=Math.floor(localBar(sec,t)*2);let rootDegree;
  if(sec.kind==='coda')rootDegree=[3,1,4,0][Math.min(3,Math.floor(bar/sec.bars*4))];
  else if(t>=sec.end-B)rootDegree=0;
  else if(t>=sec.end-B*2)rootDegree=4;
  else if(sec.kind==='pedal')rootDegree=4;
  else if(sec.kind==='episode')rootDegree=[5,1,4,0,3,6,2,5][half%8];
  else rootDegree=[0,4,0,3,1,4,0,4][half%8];
  const thematic=fixed.map(a=>a[t]).filter(e=>e&&!e.rest);if(thematic.length&&sec.kind!=='pedal'){let best=rootDegree,score=-1e9;for(const d of [rootDegree,0,1,3,4,5]){const pcs=[d,d+2,d+4].map(k=>mod(pitch(k,sec.key),12));const val=thematic.reduce((a,e)=>a+(pcs.includes(mod(e.pitch,12))?3:0),0)+(d===rootDegree?1:0);if(val>score){best=d;score=val;}}rootDegree=best;}
  return{root:rootDegree,pcs:[rootDegree,rootDegree+2,rootDegree+4].map(d=>mod(pitch(d,sec.key)+(sec.key.mode==='minor'&&rootDegree===4&&mod(d,7)===6?1:0),12))};
 }
 // Single pass with a bounded beam at every onset; all iteration and tie-breaking is stable.
 for(let t=0;t<T;t++){
  const sec=secAt[t],local=t-sec.start,B=sec.meter.barTicks,met=metric(sec,t),density=automated(s,'density',index+t/T,sec.measure.index,sec.measure.section),chord=chordAt(t,sec),prior=held.map(e=>e?.pitch??null);let onsets=[];
  for(let v=0;v<n;v++){
   if(until[v]>t)continue;
   const f=fixed[v][t];if(f){held[v]=f.rest?null:f;until[v]=f.t+f.dur;if(f.t===t){if(!f.rest){events[v].push({...f,velocity:.79});totals.thematicAttacks++;}else totals.rests++;}continue;}
   held[v]=null;
   if(sec.kind==='exposition'&&t<barAt(sec,order.indexOf(v)*sec.phraseBars)){until[v]=barAt(sec,order.indexOf(v)*sec.phraseBars);continue;}
   // Density governs accompaniment independently of the fixed thematic statements.
   // Stable local randomness makes high/low density meaningfully comparable for one seed.
   const activeDensity=automated(s,'density',index+t/T,sec.measure.index,sec.measure.section);
   if(v!==n-1&&!['coda','return','pedal'].includes(sec.kind)&&random(s.seed+':density:'+index+':'+v+':'+t)() < .015+(1-activeDensity)*.26){
     until[v]=Math.min(sec.end,t+Math.max(2,met.length));totals.rests++;continue;
   }
   // Supporting voices breathe in phrases, with less continuous activity in larger ensembles.
   const phrase=Math.floor(localBar(sec,t)/sec.phraseBars),activeLimit=Math.max(2,Math.min(n,Math.round(2+density*(n-2))));
   if(n>4&&v!==n-1&&mod(v+phrase,n)>=activeLimit&&!['coda','return'].includes(sec.kind)){until[v]=Math.min(sec.end,t+4);continue;}
   let dur=v===n-1?met.length*2:met.length;
   if(sec.kind==='coda')dur=localBar(sec,t)>=sec.bars*.75?B:Math.min(8,B);
   else if(sec.kind==='episode')dur=4;
   else if(r()<.17)dur=8;
   dur=Math.min(dur,B-met.local,sec.end-t);for(let k=t+1;k<t+dur;k++)if(fixed[v][k]){dur=k-t;break;}
   if(dur<1)dur=1;
   const entry=entries.find(e=>e.voice===v&&e.end<=t&&e.end+B*2>t);const counter=!!entry&&!['coda','pedal'].includes(sec.kind);
   const cs=[4,3,2,4,3,2,1,2,3,1,2,0,1,2,1,0];
   let target=rs[v].center;
   if(counter)target=pitch(nearestDegree(rs[v].center,sec.key)-mod(nearestDegree(rs[v].center,sec.key),7)+cs[Math.floor((t-entry.end)/2)%16],sec.key);
   else if(sec.kind==='episode')target=pitch(nearestDegree(rs[v].center,sec.key)+p.subject[Math.floor(local/2)%5].d-Math.floor(local/B)%4,sec.key);
   const candidates=[];
   for(let m=rs[v].low;m<=rs[v].high;m++){
    const inChord=chord.pcs.includes(mod(m,12));
    if(!inChord)continue;
    if(last[v]!=null&&Math.abs(m-last[v])>9)continue;
    if(last[v]!=null&&Math.abs(m-last[v])===6)continue;
    if(v===n-1&&sec.kind==='coda'&&mod(m,12)!==chord.pcs[0])continue;
    if(v===0&&sec.kind==='coda'&&localBar(sec,t)>=sec.bars*.75&&mod(m,12)!==sec.key.pc)continue;
    let cost=Math.abs(m-target)*.1+Math.abs(m-rs[v].center)*.08+(last[v]==null?0:Math.abs(m-last[v])*.13);
    if(m===last[v])cost+=1.1;
    if(last[v]!=null&&m!==last[v])for(let u=0;u<n;u++){if(u===v||last[u]==null||prevMel[u]==null)continue;const direction=Math.sign(m-last[v]);if(direction&&Math.sign(last[u]-prevMel[u])===-direction)cost-=.12*s.development;}
    if(last[v]!=null&&prevMel[v]!=null&&Math.abs(last[v]-prevMel[v])>4&&Math.sign(m-last[v])===Math.sign(last[v]-prevMel[v]))cost+=2.6;
    if(v===n-1&&mod(m,12)!==chord.pcs[0])cost+=1.4;
    if(v!==n-1&&m===last[v]&&t-lastAttack[v]<8)cost+=.8;
    cost+=r()*.23;
    candidates.push({m,cost,passing:false});
   }
   candidates.sort((a,b)=>a.cost-b.cost);candidates.push({m:null,cost:7,passing:false});onsets.push({v,dur,counter,candidates:candidates.slice(0,10).concat(candidates.length>10?[candidates.at(-1)]:[])});
  }
  // Bass first establishes the consonance reference. Thematic notes are immutable.
  onsets.sort((a,b)=>b.v-a.v);let beam=[{pitches:held.map(e=>e?.pitch??null),choices:[],cost:0}];
  for(const onset of onsets){const next=[];for(const b of beam)for(const c of onset.candidates){const m=c.m;let penalty=0,ok=true;
    if(m!=null){const low=Math.min(m,...b.pitches.filter(x=>x!=null));for(let u=0;u<n;u++){const x=b.pitches[u];if(u===onset.v||x==null)continue;
      if((u<onset.v&&x<m)||(u>onset.v&&x>m)){ok=false;break;}
      if(!consonant(m,x,low)&&!c.passing){ok=false;break;}
      if(parallel(m,x,prior[onset.v],prior[u])){ok=false;break;}
      if(m===x)penalty+=2.2;
      if(Math.abs(onset.v-u)===1&&Math.abs(m-x)>16)penalty+=1;
    }
    // Anticipate the next fixed thematic attacks during a held support note.
    if(ok&&!c.passing)for(let k=t+1;k<t+onset.dur;k++)for(let u=0;u<n;u++){const f=fixed[u][k];if(u!==onset.v&&f&&!f.rest&&f.t===k&&!consonant(m,f.pitch,Math.min(low,f.pitch))){ok=false;break;}}
    }
    if(!ok)continue;const ps=b.pitches.slice();ps[onset.v]=m;next.push({pitches:ps,choices:[...b.choices,{...onset,...c}],cost:b.cost+c.cost+penalty});
   }next.sort((a,b)=>a.cost-b.cost);beam=next.slice(0,18);}
  const chosen=beam[0];if(chosen)for(const c of chosen.choices){until[c.v]=t+c.dur;if(c.m==null){totals.rests++;continue;}const e={voice:c.v,t,dur:c.dur,pitch:c.m,key:sec.key,kind:c.counter?'countersubject':c.passing?'passing':'free',velocity:.54+(met.strong?.045:0)+(c.v===n-1?.025:0)};held[c.v]=e;events[c.v].push(e);totals.supportAttacks++;}
  for(let v=0;v<n;v++)if(held[v]&&held[v].t===t){prevMel[v]=last[v];last[v]=held[v].pitch;lastAttack[v]=t;}
 }
 // Rubato is structural: a small phrase arch, sectional energy, and cadential breadth.
 const timing=new Float64Array(T+1),tempos=new Float32Array(T);
 const micro={off:0,light:.008,expressive:.018}[s.microtiming];
 for(let t=0;t<T;t++){const sec=secAt[t],B=sec.meter.barTicks,phase=mod(t-sec.start,B*4)/(B*4);let factor=1+.026*Math.sin(2*Math.PI*phase);if(sec.kind==='stretto'||sec.kind==='diminution')factor+=.045;if(sec.kind==='augmentation')factor-=.035;const left=sec.end-t;if(left<B/2)factor*=1-.105*(1-left/(B/2));if(sec.kind==='coda'&&left<B*1.5)factor*=1-.20*(1-left/(B*1.5));tempos[t]=automated(s,'tempo',index+t/T,sec.measure.index,sec.measure.section)*factor;timing[t+1]=timing[t]+15/tempos[t];}
 // Gentle agogic delay inside each metric group; primary beats remain anchored.
 for(let t=1;t<T;t++){const sec=secAt[t],m=metric(sec,t),phrase=mod(t-sec.start,sec.meter.barTicks*4)/(sec.meter.barTicks*4);timing[t]+=micro*Math.sin(Math.PI*m.within/m.length)*(.7+.3*Math.sin(Math.PI*phrase));}
 for(let v=0;v<n;v++)for(const e of events[v]){const m=metric(secAt[e.t],e.t);e.microOffset=m.strong?0:micro*.2*(n-1-v)/Math.max(1,n-1);e.at=timing[e.t]+e.microOffset;e.until=Math.min(timing[T],timing[e.t+e.dur]+e.microOffset);}
 const result={version:3,index,settings:s,controlSegments:[{bar:0,settings:s}],home:p.home,roles:rs,sections:p.sections,measures:p.measures,subject:p.subject,themes:p.themes,family:p.family,opening,length:T,bars:p.measures.length,events,entries,timing:Array.from(timing),tempos:Array.from(tempos),duration:timing[T],totals};result.audit=audit(result);return result;
}
function audit(c){let dissonances=0,parallels=0,crossings=0,strongPairs=0,previous=null;const slots=c.events.map(es=>{const a=Array(c.length).fill(null);for(const e of es)for(let t=e.t;t<e.t+e.dur;t++)a[t]=e;return a;});for(const measure of c.measures){let offset=0;for(const group of measure.meter.groupTicks){const t=measure.t+offset;offset+=group;const ps=slots.map(a=>a[t]?.pitch??null),low=Math.min(...ps.filter(x=>x!=null));for(let v=0;v<ps.length;v++)for(let u=v+1;u<ps.length;u++)if(ps[v]!=null&&ps[u]!=null){strongPairs++;if(!consonant(ps[v],ps[u],low))dissonances++;if(ps[v]<ps[u])crossings++;if(previous&&parallel(ps[v],ps[u],previous[v],previous[u]))parallels++;}previous=ps;}}return{strongPairs,dissonances,parallels,crossings,consonance:strongPairs?1-dissonances/strongPairs:1};}
// Replace only unheard measures; retain their exact historical timings and notation.
function spliceFuture(old,fresh,bar){if(old.roles.length!==fresh.roles.length||bar>=old.bars||bar>=fresh.bars)throw new Error('A compatible future bar is required.');const a=old.measures[bar].t,b=fresh.measures[bar].t,dt=a-b,seconds=old.timing[a]-fresh.timing[b],length=fresh.length+dt;
 const measures=[...old.measures.slice(0,bar).map(m=>({...m})),...fresh.measures.slice(bar).map(m=>({...m,t:m.t+dt,end:m.end+dt}))],sections=[];
 const addSections=(source,lo,hi,shift)=>{for(const sec of source.sections){const start=Math.max(lo,sec.start),end=Math.min(hi,sec.end);if(end<=start)continue;const local=measures.filter(m=>m.t>=start+shift&&m.t<end+shift);for(const m of local)m.section=sections.length;sections.push({...sec,start:start+shift,end:end+shift,firstBar:local[0].index,bars:local.length,measures:local});}};
 addSections(old,0,a,0);addSections(fresh,b,fresh.length,dt);
 const events=old.events.map((es,v)=>[...es.filter(e=>e.t<a).map(e=>({...e,dur:Math.min(e.dur,a-e.t),until:Math.min(e.until,old.timing[a])})),...fresh.events[v].filter(e=>e.t+e.dur>b).map(e=>({...e,t:Math.max(b,e.t)+dt,dur:e.t+e.dur-Math.max(b,e.t),at:Math.max(fresh.timing[b],e.at)+seconds,until:e.until+seconds}))]);
 const timing=[...old.timing.slice(0,a),...fresh.timing.slice(b).map(t=>t+seconds)],tempos=[...old.tempos.slice(0,a),...fresh.tempos.slice(b)],entries=[...old.entries.filter(e=>e.t<a).map(e=>({...e,end:Math.min(e.end,a)})),...fresh.entries.filter(e=>e.end>b).map(e=>({...e,t:Math.max(e.t,b)+dt,end:e.end+dt}))];
 const result={...fresh,opening:old.opening,controlSegments:[...(old.controlSegments||[{bar:0,settings:old.settings}]).filter(x=>x.bar<bar),{bar,settings:fresh.settings}],measures,sections,events,entries,timing,tempos,length,duration:timing[length],mixed:true};result.audit=audit(result);return result;
}
const api={spliceFuture,compose,plan,roles,settings,makeMeter,makeKey,MODES,ROOTS,validateTheme,epoch,hash,random,pitch,nearestDegree,consonant,parallel,audit,KEYS,METERS,automated,metric,adaptSubject,measureAt};if(typeof module!=='undefined')module.exports=api;root.Fugue=api;
})(typeof globalThis!=='undefined'?globalThis:this);
