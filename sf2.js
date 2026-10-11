'use strict';
/* Local SF2 (uncompressed PCM) reader; zone and sample rendering use Web Audio. */
(function(root){
 const clamp=(n,a,b)=>Math.min(b,Math.max(a,n));
 const seconds=v=>clamp(Math.pow(2,clamp(v,-12000,8000)/1200),.002,20);
 const GEN={instrument:41,sampleID:53,keyRange:43,velRange:44,pan:17,attenuation:48,root:58,coarse:51,fine:52,attack:34,hold:35,decay:36,sustain:37,release:38,loop:54,filter:8};
 const additive=new Set([GEN.pan,GEN.attenuation,GEN.coarse,GEN.fine]);
 function SF2(buffer,name='SoundFont'){
  if(!(buffer instanceof ArrayBuffer))throw Error('Expected an SF2 file.');
  this.data=new DataView(buffer);this.name=name;this.pcm=null;this.samples=[];this.presets=[];this.instruments=[];this.buffers=new WeakMap();this.parse();
 }
 SF2.prototype.readString=function(offset,count){const d=this.data;let s='';for(let i=0;i<count;i++){const b=d.getUint8(offset+i);if(!b)break;s+=String.fromCharCode(b);}return s.trim();};
 SF2.prototype.chunks=function(from,to,fn){const d=this.data;for(let p=from;p+8<=to;){const id=this.readString(p,4),n=d.getUint32(p+4,true),end=p+8+n;if(end>to)throw Error('Corrupted SF2 chunk: '+id);fn(id,p+8,n);p=end+(n&1);}};
 SF2.prototype.parse=function(){const d=this.data;if(d.byteLength<20||this.readString(0,4)!=='RIFF'||this.readString(8,4)!=='sfbk')throw Error('Not a RIFF SoundFont 2 (.sf2) file.');
  const size=d.getUint32(4,true);if(size+8>d.byteLength)throw Error('Incomplete SF2 file.');const tables={};
  this.chunks(12,size+8,(id,p,n)=>{if(id!=='LIST'||n<4)return;const list=this.readString(p,4);if(list==='sdta')this.chunks(p+4,p+n,(k,q,len)=>{if(k==='smpl')this.pcm={offset:q,count:Math.floor(len/2)};});if(list==='pdta')this.chunks(p+4,p+n,(k,q,len)=>tables[k]=[q,len]);});
  if(!this.pcm)throw Error('SF2 contains no 16-bit PCM sample data.');
  const read=(id,stride,fn)=>{const spec=tables[id];if(!spec||spec[1]%stride)throw Error('Missing or invalid SoundFont table: '+id);return Array.from({length:spec[1]/stride},(_,i)=>fn(spec[0]+stride*i));};
  const headers=read('phdr',38,p=>({name:this.readString(p,20),program:d.getUint16(p+20,true),bank:d.getUint16(p+22,true),bag:d.getUint16(p+24,true)}));
  const pbag=read('pbag',4,p=>d.getUint16(p,true));const pgen=read('pgen',4,p=>({op:d.getUint16(p,true),value:d.getInt16(p+2,true),unsigned:d.getUint16(p+2,true)}));
  const inst=read('inst',22,p=>({name:this.readString(p,20),bag:d.getUint16(p+20,true)}));const ibag=read('ibag',4,p=>d.getUint16(p,true));const igen=read('igen',4,p=>({op:d.getUint16(p,true),value:d.getInt16(p+2,true),unsigned:d.getUint16(p+2,true)}));
  this.samples=read('shdr',46,p=>({name:this.readString(p,20),start:d.getUint32(p+20,true),end:d.getUint32(p+24,true),loopStart:d.getUint32(p+28,true),loopEnd:d.getUint32(p+32,true),rate:d.getUint32(p+36,true),pitch:d.getUint8(p+40),correction:d.getInt8(p+41),link:d.getUint16(p+42,true),type:d.getUint16(p+44,true)})).slice(0,-1);
  const zones=(bags,gens,start,end)=>{const out=[];for(let i=start;i<end;i++){const g={};const a=bags[i],b=bags[i+1];if(b===undefined||a===undefined||a>b||b>gens.length)throw Error('Malformed SoundFont zone indexes.');for(let j=a;j<b;j++)g[gens[j].op]=gens[j].unsigned;out.push(g);}return out;};
  this.instruments=inst.slice(0,-1).map((h,i)=>({name:h.name,zones:zones(ibag,igen,h.bag,inst[i+1].bag)}));
  const range=(g,k)=>{const v=g[k];return v===undefined?[0,127]:[v&255,v>>8];};
  const signed=x=>x>32767?x-65536:x;
  const combine=(...gg)=>{const result={};for(const g of gg)if(g)for(const [key,value]of Object.entries(g)){const op=+key;if(additive.has(op))result[op]=(result[op]||0)+signed(value);else if(op===GEN.keyRange||op===GEN.velRange){const old=range(result,op),now=range(g,op);result[op]=clamp(Math.max(old[0],now[0]),0,255)|(clamp(Math.min(old[1],now[1]),0,255)<<8);}else result[op]=value;}return result;};
  this.presets=headers.slice(0,-1).map((h,i)=>{const pzones=zones(pbag,pgen,h.bag,headers[i+1].bag),pglobal=pzones.find(g=>g[GEN.instrument]===undefined)||{};const resolved=[];
   for(const pz of pzones){const index=pz[GEN.instrument];if(index===undefined||index>=this.instruments.length)continue;const ins=this.instruments[index],iglobal=ins.zones.find(g=>g[GEN.sampleID]===undefined)||{};
    for(const iz of ins.zones){const sample=iz[GEN.sampleID];if(sample===undefined||sample>=this.samples.length)continue;const g=combine(pglobal,pz,iglobal,iz);const keys=range(g,GEN.keyRange),vel=range(g,GEN.velRange);if(keys[0]>keys[1]||vel[0]>vel[1])continue;resolved.push({sample,keyLo:keys[0],keyHi:keys[1],velLo:vel[0],velHi:vel[1],pan:clamp(signed(g[GEN.pan]||0)/500,-1,1),attenuation:clamp(signed(g[GEN.attenuation]||0),0,1440),root:g[GEN.root]===undefined?null:g[GEN.root],coarse:signed(g[GEN.coarse]||0),fine:signed(g[GEN.fine]||0),attack:seconds(signed(g[GEN.attack]??-12000)),decay:seconds(signed(g[GEN.decay]??-12000)),sustain:clamp(signed(g[GEN.sustain]||0)/1000,0,1),release:seconds(signed(g[GEN.release]??-7000)),loop:(g[GEN.loop]||0)&3,filter:g[GEN.filter]===undefined?null:clamp(8.176*Math.pow(2,(g[GEN.filter]&65535)/1200),40,18000)});}
   }
   return{name:h.name||'Preset '+h.program,program:h.program,bank:h.bank,zones:resolved};});
  if(!this.presets.some(p=>p.zones.length))throw Error('No playable presets found in this SoundFont.');
 };
 SF2.prototype.getBuffer=function(ctx,index){let cache=this.buffers.get(ctx);if(!cache){cache=new Map();this.buffers.set(ctx,cache);}if(cache.has(index))return cache.get(index);const s=this.samples[index];if(!s||s.end<=s.start||s.end>this.pcm.count||!s.rate)throw Error('Invalid PCM sample bounds.');
  const n=s.end-s.start,buf=ctx.createBuffer(1,n,s.rate),dest=buf.getChannelData(0),offset=this.pcm.offset+s.start*2;for(let i=0;i<n;i++)dest[i]=this.data.getInt16(offset+i*2,true)/32768;cache.set(index,buf);return buf;};
 SF2.prototype.play=function(preset,pitch,velocity,ctx,when,duration,bus,level,active){const patch=this.presets[preset];if(!patch)return;const midiVel=clamp(Math.round(velocity*127),1,127),matching=patch.zones.filter(z=>pitch>=z.keyLo&&pitch<=z.keyHi&&midiVel>=z.velLo&&midiVel<=z.velHi);if(!matching.length)return;
  for(const z of matching.slice(0,12)){let sample;try{sample=this.samples[z.sample];if(!sample)continue;}catch{continue;}
   let source;try{source=ctx.createBufferSource();source.buffer=this.getBuffer(ctx,z.sample);}catch{continue;}
   const root=z.root!==null&&z.root<=127?z.root:sample.pitch,pitchShift=pitch-root+z.coarse+(z.fine+sample.correction)/100;source.playbackRate.value=Math.pow(2,pitchShift/12);
   const attack=clamp(Math.min(z.attack,Math.max(.006,duration*.2)),.003,1.1),release=clamp(Math.min(z.release,Math.max(.04,duration*.32)),.025,1.3),decay=clamp(Math.min(z.decay,Math.max(.02,duration*.3)),.015,1.5),end=when+Math.max(.04,duration),env=ctx.createGain(),pan=ctx.createStereoPanner();pan.pan.value=clamp(z.pan*.65+((sample.type&127)===4?-.8:(sample.type&127)===2?.8:0),-1,1);
   const amp=level*Math.pow(10,-z.attenuation/200)*.78;env.gain.setValueAtTime(0,when);env.gain.linearRampToValueAtTime(amp,when+Math.min(attack,duration*.45));env.gain.setTargetAtTime(amp*Math.pow(10,-z.sustain*3),when+Math.min(attack,duration*.45),Math.max(.012,decay/3));env.gain.setTargetAtTime(0,end,Math.max(.008,release/4));
   const canLoop=(z.loop===1||z.loop===3)&&sample.loopEnd>sample.loopStart+8&&sample.loopEnd<=sample.end;
   if(canLoop){source.loop=true;source.loopStart=(sample.loopStart-sample.start)/sample.rate;source.loopEnd=(sample.loopEnd-sample.start)/sample.rate;}
   source.connect(env);env.connect(pan);pan.connect(bus);const stopAt=end+Math.max(.1,release*2);const entry={end:stopAt+.05,node:{stop:()=>{const t=ctx.currentTime;env.gain.cancelScheduledValues(t);env.gain.setTargetAtTime(0,t,.012);try{source.stop(t+.06);}catch{}}}};
   source.onended=()=>{source.disconnect();env.disconnect();pan.disconnect();active.delete(entry);};try{source.start(when);source.stop(stopAt);active.add(entry);}catch{source.disconnect();env.disconnect();pan.disconnect();}
  }
 };
 root.SF2=SF2;if(typeof module!=='undefined'&&module.exports)module.exports=SF2;
})(typeof globalThis!=='undefined'?globalThis:window);
