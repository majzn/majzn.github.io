'use strict';
(function(root){
 const hash=s=>{let h=2166136261;for(const c of String(s)){h^=c.charCodeAt(0);h=Math.imul(h,16777619);}return h>>>0;};
 const hue=n=>(n%360+360)%360;
 function palette(seed,scheme='analogous',count=4){const base=hash(seed)%360;const offsets={analogous:[-48,-22,8,34,57,83],complementary:[0,175,24,202,47,224],triadic:[0,120,240,24,145,265],split:[0,145,215,20,165,238],pastel:[0,32,74,128,178,260],earth:[0,27,62,145,198,220]}[scheme]||[0,30,90,160];const colors=[];
  for(let i=0;i<count;i++){const a=offsets[i%offsets.length]+Math.floor(i/offsets.length)*13,h=hue(base+a),s=scheme==='pastel'?33:scheme==='earth'?29:47,l=scheme==='pastel'?42:scheme==='earth'?38:36;colors.push(`hsl(${h} ${s}% ${l}%)`);}return colors;
 }
 root.StaffColorTheory={palette,hash};if(typeof module!=='undefined'&&module.exports)module.exports={palette,hash};
})(typeof globalThis!=='undefined'?globalThis:window);
