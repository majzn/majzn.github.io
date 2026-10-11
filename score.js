
'use strict';
class LivingScore {
 constructor(viewport){this.viewport=viewport;this.content=document.getElementById('scoreContent');this.cache=new Map();this.records=[];this.current=null;this.cycle=null;this.height=436;this.currentBar='';this.dirty=true;this.lastTick=0;this.zoom=1;this.playing=false;this.pixelsPerSecond=135;this.horizontalDensity=1;this.staffColors=null;this.lastLayoutX=NaN;}
 updateSize(){this.content.style.height=this.height+'px';this.viewport.style.setProperty('--score-height',Math.round(this.height*this.zoom)+'px');}
 setPlaying(value){this.playing=!!value;this.draw(this.lastTick);}
 setDensity(value){const n=Math.max(.4,Math.min(2.8,Math.round(Number(value)*100)/100));if(n===this.horizontalDensity)return;this.horizontalDensity=n;this.pixelsPerSecond=135*n;const cycles=this.records.map(r=>r.cycle),currentIndex=this.current?.cycle.index,old=this.records.slice();this.records=[];let base=0;for(const cycle of cycles){const record=this.layout(cycle,base);this.records.push(record);base+=record.total;}this.current=this.records.find(r=>r.cycle.index===currentIndex)||this.records.at(-1)||null;this.cache.forEach(item=>item.div.remove());this.cache.clear();this.dirty=true;this.draw(this.lastTick);}
 setStaffColors(colors){this.staffColors=colors?colors.slice():null;for(const item of this.cache.values())item.div.remove();this.cache.clear();this.dirty=true;this.draw(this.lastTick);}
 setZoom(value){const next=Math.min(4,Math.max(.4,Math.round(value*100)/100));if(next===this.zoom)return;this.zoom=next;this.updateSize();this.dirty=true;this.draw(this.lastTick);}
 key(record,bar){return record.cycle.index+':'+bar;}
 signatureInfo(c,bar,previousRecord=null){
  const measure=c.measures[bar],section=c.sections[measure.section],key=measure.key||section.key;
  const previous=bar>0?c.measures[bar-1]:previousRecord?.cycle.measures.at(-1),previousSection=previous&&(bar>0?c.sections[previous.section]:previousRecord.cycle.sections[previous.section]);
  const oldKey=previous&&(previous.key||previousSection.key),opening=bar===0;
  const keyChange=opening||!!oldKey&&oldKey.name!==key.name;
  const meterChange=opening||!!previous&&previous.meter.id!==measure.meter.id;
  const clefChange=opening||!!previousRecord&&bar===0&&previousRecord.cycle.roles.some((role,v)=>role.clef!==c.roles[v]?.clef);
  return{key,oldKey,opening,keyChange,meterChange,clefChange,meter:measure.meter};
 }
 signatureWidth(c,bar,previousRecord=null){
  const info=this.signatureInfo(c,bar,previousRecord);
  if(!info.keyChange&&!info.meterChange&&!info.clefChange)return 0;
  // VexFlow uses variable-width glyphs and extra naturals when a key changes.
  // Measure the *real* modifier layout in an unattached stave and reserve its
  // complete width before the first rhythmic event, rather than guessing pixels.
  const accidentals=name=>{const count={C:0,G:1,D:2,A:3,E:4,B:5,'F#':6,'C#':7,F:1,Bb:2,Eb:3,Ab:4,Db:5,Gb:6,Cb:7,Am:0,Em:1,Bm:2,'F#m':3,'C#m':4,'G#m':5,'D#m':6,'A#m':7,Dm:1,Gm:2,Cm:3,Fm:4,Bbm:5,Ebm:6,Abm:7};return count[name]??0;};
  const keyComplexity=info.keyChange?15+15*accidentals(info.key.vf)+(!info.opening?9*accidentals(info.oldKey?.vf):0):0;
  const reserved=28+(info.clefChange?55:0)+keyComplexity+(info.meterChange?55:0);
  if(typeof VexFlow!=='undefined'){
   const V=VexFlow.Flow;let furthest=0;
   for(const role of c.roles){
    const stave=new V.Stave(0,0,400);
    stave.setBegBarType(info.opening&&c.index===0?V.Barline.type.SINGLE:V.Barline.type.NONE);
    if(info.clefChange)stave.addClef(role.clef);
    if(info.keyChange)stave.addKeySignature(info.key.vf,info.oldKey&&!info.opening?info.oldKey.vf:undefined);
    if(info.meterChange)stave.addTimeSignature(info.meter.id);
    furthest=Math.max(furthest,stave.getNoteStartX());
   }
   return Math.max(Math.ceil(furthest+32),reserved);
  }
  // Headless structural tests can run without loading the large engraving library.
  return reserved;
 }
 layout(c,base){
  const previous=this.records?.find(r=>r.cycle.index===c.index-1);
  const headers=c.measures.map((m,i)=>this.signatureWidth(c,i,previous)),musicWidths=c.measures.map(m=>(c.timing[m.end]-c.timing[m.t])*this.pixelsPerSecond);
  const widths=musicWidths.map((w,i)=>w+headers[i]),offsets=[0];
  for(const w of widths)offsets.push(offsets.at(-1)+w);
  // The last barline of a measure and the left stave edge of the next one
  // share a coordinate. No duplicate vertical strokes appear at boundaries.
  const initialInset=previous?previous.insets.at(-1):16;
  const insets=musicWidths.map((w,i)=>i===0?initialInset:Math.min(16,musicWidths[i-1]*.22,w*.22));
  insets.push(Math.min(16,musicWidths.at(-1)*.22));
  return{cycle:c,base,headers,musicWidths,widths,offsets,insets,total:offsets.at(-1)};
 }
 setCycle(c,continuous=false){
  if(!continuous){this.cache.clear();this.content.replaceChildren();this.records=[];}
  let record=this.records.find(r=>r.cycle.index===c.index);
  if(!record){const previous=this.current,base=continuous&&previous?previous.base+previous.total:0;record=this.layout(c,base);this.records.push(record);}
  if(this.height!==36+c.roles.length*100){for(const item of this.cache.values())item.div.remove();this.cache.clear();}this.current=record;this.cycle=c;this.currentBar='';this.height=36+c.roles.length*100;this.updateSize();
  this.records=this.records.filter(r=>r.cycle.index>=c.index-1);
  for(const [key,item]of this.cache)if(!this.records.includes(item.record)){item.div.remove();this.cache.delete(key);}
  // Keep CSS coordinates small without changing the relative on-screen position.
  if(record.base>100000){const shift=this.records[0].base;for(const r of this.records)r.base-=shift;for(const item of this.cache.values()){item.start-=shift;item.end-=shift;item.div.style.left=item.start+'px';}}
  this.dirty=true;this.draw(0);
 }
 prepareNext(c){if(!this.current)return;const existing=this.records.find(r=>r.cycle.index===c.index);if(existing){for(const [key,item]of this.cache)if(item.record===existing){item.div.remove();this.cache.delete(key);}this.records=this.records.filter(r=>r!==existing);}this.records.push(this.layout(c,this.current.base+this.current.total));this.dirty=true;this.draw(this.lastTick);}
 replaceFuture(c,bar){const old=this.current,record=this.layout(c,old.base);this.records=this.records.filter(r=>r.cycle.index<=c.index);this.records[this.records.indexOf(old)]=record;for(const [key,item]of this.cache){if(item.record.cycle.index>c.index||item.record===old&&item.bar>=bar){item.div.remove();this.cache.delete(key);}else if(item.record===old)item.record=record;}this.current=record;this.cycle=c;this.dirty=true;this.draw(this.lastTick);}
 nextMeasure(record,bar){if(bar+1<record.cycle.bars)return{record,bar:bar+1};const next=this.records.find(r=>r.cycle.index===record.cycle.index+1);return next?{record:next,bar:0}:null;}
 item(record,bar){return this.cache.get(this.key(record,bar))||this.renderBar(bar,record);}
 spell(m,key){const letters=['c','d','e','f','g','a','b'],naturals=[0,2,4,5,7,9,11];let degree=Fugue.nearestDegree(m,key);if(key.minor&&((m-key.pc)%12+12)%12===11)degree=Fugue.nearestDegree(m-1,key);const diatonic=key.letter+degree,letter=((diatonic%7)+7)%7;let oct=Math.floor(m/12)-1;let base=12*(oct+1)+naturals[letter];while(m-base>6){base+=12;oct++;}while(m-base< -6){base-=12;oct--;}const diff=m-base;return{key:letters[letter]+(diff===1?'#':diff===-1?'b':diff===2?'##':diff===-2?'bb':'')+'/'+oct,acc:diff===1?'#':diff===-1?'b':diff===2?'##':diff===-2?'bb':'n',id:letters[letter]+oct};}
 segments(v,bar,cycle=this.cycle){const {t:start,end}=cycle.measures[bar],es=cycle.events[v].filter(e=>e.t<end&&e.t+e.dur>start),out=[];let t=start;for(const e of es){if(e.t>t)this.split(out,t,e.t,null);this.split(out,Math.max(start,e.t),Math.min(end,e.t+e.dur),e);t=Math.min(end,e.t+e.dur);}if(t<end)this.split(out,t,end,null);return out;}
 split(out,start,end,event){let t=start;while(t<end){let size=[16,12,8,6,4,3,2,1].find(d=>d<=end-t)||1;out.push({t,dur:size,event,tieIn:event&&t>event.t,tieOut:event&&t+size<event.t+event.dur});t+=size;}}
 renderBar(bar,record=this.current){
  const V=Vex.Flow,c=record.cycle,W=record.widths[bar],measure=c.measures[bar],div=document.createElement('div');
  div.className='score-bar';div.style.left=(record.base+record.offsets[bar])+'px';div.style.width=W+'px';this.content.append(div);
  const renderer=new V.Renderer(div,V.Renderer.Backends.SVG);
  const info=this.signatureInfo(c,bar,this.records.find(r=>r.cycle.index===c.index-1));
  const {key,oldKey,opening,keyChange,meterChange,clefChange}=info;
  const header=record.headers[bar],staveX=-record.insets[bar],staveWidth=W+record.insets[bar]-record.insets[bar+1];
  renderer.resize(Math.max(40,W+2),this.height);
  const ctx=renderer.getContext();ctx.setFillStyle('#292923');ctx.setStrokeStyle('#292923');
  const staves=[],voices=[],notesByVoice=[],segmentsByVoice=[];
  c.roles.forEach((role,v)=>{
   const stave=new V.Stave(staveX,v*100+10,staveWidth),staffColor=this.staffColors?.[v]||null;
   // VexFlow adds a beginning and ending barline to *each* standalone stave.
   // Hide the beginning on subsequent bars and let the preceding measure own
   // the boundary. Leave the initial clef/key/meter on the very first bar.
   stave.setBegBarType(opening&&c.index===0?V.Barline.type.SINGLE:V.Barline.type.NONE);
   stave.setEndBarType(V.Barline.type.SINGLE);
   if(clefChange)stave.addClef(role.clef);
   if(keyChange)stave.addKeySignature(key.vf,oldKey&&!opening?oldKey.vf:undefined);
   if(meterChange)stave.addTimeSignature(measure.meter.id);
   if(staffColor)ctx.setStrokeStyle(staffColor);stave.setContext(ctx).draw();staves.push(stave);
   const segments=this.segments(v,bar,c),accidentals=new Map(),notes=segments.map(seg=>{
    const e=seg.event,rest=!e,dotted=[12,6,3].includes(seg.dur),dur=({16:'w',12:'h',8:'h',6:'q',4:'q',3:'8',2:'8',1:'16'})[seg.dur]+(rest?'r':'');
    const spelled=rest?null:this.spell(e.pitch,e.key);
    const note=new V.StaveNote({clef:role.clef,keys:[rest?(role.clef==='bass'?'d/3':'b/4'):spelled.key],duration:dur,dots:dotted?1:0,auto_stem:true});
    if(dotted)V.Dot.buildAndAttach([note],{all:true});
    if(!rest){const defaultAcc=this.keyAccidental(spelled.id[0],key);if((accidentals.get(spelled.id)??defaultAcc)!==spelled.acc){note.addModifier(new V.Accidental(spelled.acc),0);accidentals.set(spelled.id,spelled.acc);}const color=this.staffColors?.[v]||'#34372f';note.setStyle({fillStyle:color,strokeStyle:color});}
    else note.setStyle({fillStyle:'#aaa294',strokeStyle:'#aaa294'});
    return note;
   });
   const voice=new V.Voice({num_beats:measure.meter.num,beat_value:measure.meter.den}).addTickables(notes);
   voices.push(voice);notesByVoice.push(notes);segmentsByVoice.push(segments);
  });
  const startX=Math.max(...staves.map(s=>s.getNoteStartX()));staves.forEach(s=>s.setNoteStartX(startX));
  const formatter=new V.Formatter();voices.forEach(v=>formatter.joinVoices([v]));
  // The formatter calculates note geometry and accidentals, but its rhythmic spacing
  // would vary scrolling speed. Put the finished note contexts at their audio timestamps.
  formatter.format(voices,Math.max(230,W+120));
  notesByVoice.forEach((notes,v)=>notes.forEach(note=>note.setStave(staves[v])));
  const first=notesByVoice[0][0],shift=first.getAbsoluteX()-first.getTickContext().getX();
  voices.forEach((voice,v)=>notesByVoice[v].forEach((note,i)=>{
   const seg=segmentsByVoice[v][i],sec=c.timing[seg.t]-c.timing[measure.t];
   note.getTickContext().setX(header+sec*this.pixelsPerSecond-shift);
  }));
  const activities=[];
  voices.forEach((voice,v)=>{
   const notes=notesByVoice[v],segments=segmentsByVoice[v];
   if(this.staffColors?.[v])ctx.setStrokeStyle(this.staffColors[v]);const beams=V.Beam.generateBeams(notes,{groups:measure.meter.groups.map(g=>new V.Fraction(g,measure.meter.den)),beam_rests:false});
   const svg=div.querySelector('svg'),staveNoteCount=svg.querySelectorAll('.vf-stavenote').length;
   voice.draw(ctx,staves[v]);
   const inkNotes=Array.from(svg.querySelectorAll('.vf-stavenote')).slice(staveNoteCount);
   for(let i=0;i<notes.length;i++){
    const seg=segments[i];
    if(seg.event){
     // Highlight the actual engraved notehead: no extra SVG rings, paths or glyphs.
     const head=inkNotes[i]?.querySelector('.vf-notehead');
     if(head){
      const sounding=true;
      if(sounding){
       head.classList.add('playback-notehead');
       head.style.setProperty('--note-lit','#b96345');
      }
      activities.push({head,start:seg.event.t===seg.t?seg.event.at:c.timing[seg.t],end:seg.event.until,sounding,active:false});
     }
    }
    if(seg.tieIn){const prior=i>0&&segments[i-1].event===seg.event?notes[i-1]:null;
     new V.StaveTie({first_note:prior,last_note:notes[i],first_indices:[0],last_indices:[0]}).setContext(ctx).draw();}
    if(seg.tieOut&&i===notes.length-1)new V.StaveTie({first_note:notes[i],last_note:null,first_indices:[0],last_indices:[0]}).setContext(ctx).draw();
   }
   beams.forEach(b=>b.setContext(ctx).draw());
  });
  const item={div,record,bar,start:record.base+record.offsets[bar],end:record.base+record.offsets[bar+1],activities};
  this.cache.set(this.key(record,bar),item);return item;
 }
 keyAccidental(letter,key){const sharps=['f','c','g','d','a','e','b'],flats=['b','e','a','d','g','c','f'];const signatures={C:0,G:1,D:2,A:3,E:4,B:5,'F#':6,'C#':7,F:-1,Bb:-2,Eb:-3,Ab:-4,Db:-5,Gb:-6,Cb:-7,Am:0,Em:1,Bm:2,'F#m':3,'C#m':4,'G#m':5,'D#m':6,'A#m':7,Dm:-1,Gm:-2,Cm:-3,Fm:-4,Bbm:-5,Ebm:-6,Abm:-7};const count=signatures[key.vf]||0;return count>0&&sharps.slice(0,count).includes(letter)?'#':count<0&&flats.slice(0,-count).includes(letter)?'b':'n';}

 timeAt(c,t){const i=Math.max(0,Math.min(c.length-1,Math.floor(t))),fraction=Math.max(0,Math.min(1,t-i));return c.timing[i]+(c.timing[i+1]-c.timing[i])*fraction;}
 xAt(t){const r=this.current,c=r.cycle,m=Fugue.measureAt(c,t),bar=m.index;
  return r.base+r.offsets[bar]+r.headers[bar]+(this.timeAt(c,t)-c.timing[m.t])*this.pixelsPerSecond;
 }
 draw(t){if(!this.current)return;this.lastTick=t;const record=this.current,bar=Fugue.measureAt(record.cycle,t).index,key=this.key(record,bar),x=this.xAt(t),zoom=this.zoom,anchor=this.viewport.clientWidth*.5;
  const left=x-(anchor+500)/zoom,right=x+(this.viewport.clientWidth-anchor+700)/zoom,seconds=this.timeAt(record.cycle,t);
  if(this.currentBar!==key||this.dirty||!Number.isFinite(this.lastLayoutX)||Math.abs(x-this.lastLayoutX)>180/zoom){for(const r of this.records){for(let b=0;b<r.cycle.bars;b++)if(r.base+r.offsets[b+1]>=left&&r.base+r.offsets[b]<=right)this.item(r,b);}for(const [k,item]of this.cache)if(item.end<left||item.start>right){item.div.remove();this.cache.delete(k);}this.currentBar=key;this.lastLayoutX=x;this.dirty=false;}
  this.content.style.transform=`translate3d(${anchor-x*zoom}px,0,0) scale(${zoom})`;
  for(const item of this.cache.values()){
   const activeRecord=item.record.cycle.index===record.cycle.index;
   for(const note of item.activities){
    const active=this.playing&&activeRecord&&note.sounding&&seconds>=note.start&&seconds<note.end;
    if(note.active!==active){note.head.classList.toggle('is-sounding',active);note.active=active;}
   }
  }
 }
}
window.LivingScore=LivingScore;
