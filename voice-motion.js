'use strict';
(function(root){
 const clamp=(x,a,b)=>Math.max(a,Math.min(b,x));
 const hash=s=>{let h=2166136261;for(const c of String(s)){h=Math.imul(h^c.charCodeAt(0),16777619);}return h>>>0;};
 const unit=s=>hash(s)/4294967296;
 const modes=['ordered','shuffle','random'],periods=['cycle','section','2bars','4bars','8bars'];
 const styles=['natural','legato','detached','staccato'];
 function normalize(input={}){input=input&&typeof input==='object'?input:{};const a=Array.isArray(input.instruments)?input.instruments.filter(x=>typeof x==='string'&&x.length&&x.length<200).slice(0,16):[];return{enabled:!!input.enabled,mode:modes.includes(input.mode)?input.mode:'ordered',period:periods.includes(input.period)?input.period:'section',instruments:[...new Set(a)],style:styles.includes(input.style)?input.style:'natural',humanize:clamp(Number(input.humanize??.18)||0,0,1),dynamics:clamp(Number(input.dynamics??.24)||0,0,1),vibrato:clamp(Number(input.vibrato??.12)||0,0,1)};}
 function epoch(c,e,seq){const period=seq.period;if(period==='cycle')return c.index;const bar=c.measures.find(m=>e.t>=m.t&&e.t<m.end)||c.measures.at(-1);if(!bar)return c.index;if(period==='section')return c.index*10000+bar.section;const n=parseInt(period,10);return c.index*10000+Math.floor(bar.index/n);}
 function instrument(c,e,seq,base){seq=normalize(seq);if(!seq.enabled||!seq.instruments.length)return base;const n=seq.instruments.length,k=epoch(c,e,seq);if(seq.mode==='ordered')return seq.instruments[((k%n)+n)%n];if(seq.mode==='random')return seq.instruments[Math.floor(unit(c.settings.seed+':inst:'+e.voice+':'+k)*n)];let order=Array.from({length:n},(_,i)=>i),round=Math.floor(k/n),r=hash(c.settings.seed+':shuffle:'+e.voice+':'+round);for(let j=n-1;j>0;j--){r=(Math.imul(1664525,r)+1013904223)>>>0;const i=r%(j+1);[order[i],order[j]]=[order[j],order[i]];}return seq.instruments[order[((k%n)+n)%n]];}
 function expression(c,e,seq){seq=normalize(seq);const id=c.settings.seed+':perform:'+c.index+':'+e.voice+':'+e.t+':'+e.pitch;
  const r=unit(id),r2=unit(id+':b'),r3=unit(id+':c');let gate=1,attackScale=1;
  if(seq.style==='legato'){gate=1.05;attackScale=1.15;}
  if(seq.style==='detached'){gate=.79;attackScale=.68;}
  if(seq.style==='staccato'){gate=.52;attackScale=.48;}
  const strength=seq.humanize,phrasing=seq.dynamics;
  const m=c.measures.find(x=>e.t>=x.t&&e.t<x.end),offbeat=m&&e.t>m.t&&(e.t-m.t)%4!==0;
  const offset=strength===0?0:(r-.5)*.022*strength; // fixed and reproducible, at most ±11 ms
  const velocity=clamp(1+(r2-.5)*.25*strength+(offbeat?.11:-.02)*phrasing+.08*Math.sin((e.t||0)*.16+e.voice)*phrasing,.65,1.22);
  const gateWobble=1+(r3-.5)*.12*strength;
  return {offset,velocity,gate:gate*gateWobble,attackScale,vibrato:seq.vibrato};
 }
 root.VoiceMotion={normalize,instrument,expression,epoch,hash};
 if(typeof module!=='undefined'&&module.exports)module.exports=root.VoiceMotion;
})(typeof globalThis!=='undefined'?globalThis:this);
