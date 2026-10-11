'use strict';
// Literary, Latin-inspired titles. Deterministic for the exact generated cycle;
// no network, user-data collection, clocks or unseeded Math.random required.
(function(root){
 const light=['Aurorae','Lucis','Solis','Stellarum','Caeli','Veris','Dierum','Albae','Concordiae','Florum','Serenitatis','Aureae'];
 const dark=['Noctis','Umbrae','Vesperi','Somniorum','Silentii','Temporis','Memoriae','Nebulae','Lacrimarum','Tenebrarum','Hiberni','Arcani'];
 const neutral=['Ventorum','Maris','Aetheris','Orbis','Fluminum','Vitae','Cordis','Aeternitatis','Lunae','Montium','Harmoniae','Itineris'];
 const heads=['Cantus','Vox','Lumen','Carmen','Iter','Somnium','Mysterium','Arcanum','Resonantia','Aevum','Ritus','Aether','Chorus','Flamma','Hymnus','Fons','Tempus','Vestigium','Origo','Murmur'];
 const preludes=['Sub Astris','Inter Umbras','De Lumine','In Aeternum','Sub Luna','Ad Caelum','In Silentio','Inter Stellas','De Tempore','Per Noctem','Sub Vespera','Ad Auroram'];
 const codas=['Perennis','Arcanus','Siderum','Aeterna','Infinita','Serenum','Solitudo','Imaginum'];
 function hash(text){let h=2166136261;for(let i=0;i<text.length;i++){h^=text.charCodeAt(i);h=Math.imul(h,16777619);}h^=h>>>16;h=Math.imul(h,2246822507);h^=h>>>13;return h>>>0;}
 function roman(n){n=Math.max(1,Math.min(3999,Math.floor(n)||1));const symbols=[[1000,'M'],[900,'CM'],[500,'D'],[400,'CD'],[100,'C'],[90,'XC'],[50,'L'],[40,'XL'],[10,'X'],[9,'IX'],[5,'V'],[4,'IV'],[1,'I']];let s='';for(const [v,glyph] of symbols)while(n>=v){s+=glyph;n-=v;}return s;}
 function title(cycle){if(!cycle||!Number.isInteger(cycle.index)||cycle.index<0)throw new Error('A generated fugue cycle is required.');
  const seed=String(cycle.settings?.seed||'counterpoint'),subject=cycle.subject||[],sig=subject.map(n=>[n.d,n.dur].join(':')).join(','),traits=[seed,cycle.index,cycle.home?.mode||'',cycle.home?.pc??'',cycle.family??'',sig].join('|');
  const salt=hash(traits),variant=hash(traits+'|form'),word=hash(traits+'|head'),shade=hash(traits+'|shade');
  const darkKey=cycle.home?.minor===true,words=darkKey?[...dark,...neutral]:[...light,...neutral];
  const of=words[shade%words.length],head=heads[word%heads.length],of2=words[hash(traits+'|echo')%words.length];
  let name;
  switch(variant%6){
   case 0:name=`${head} ${of}`;break;
   case 1:name=`${head} ${of} et ${of2}`;break;
   case 2:name=`${head} ${preludes[shade%preludes.length]}`;break;
   case 3:name=`${preludes[word%preludes.length]} · ${head} ${of}`;break;
   case 4:name=`${head} ${of} ${codas[variant%codas.length]}`;break;
   default:name=`${heads[(word+7)%heads.length]} ${of2}`;
  }
  return {name,subtitle:`FUGA ${roman(cycle.index+1)} · ${(cycle.home?.mode||'modus').replace(/_/g,' ').toUpperCase()}`,cycle:cycle.index,seed};
 }
 function opacityAt(seconds,duration){if(!Number.isFinite(seconds)||seconds<0)return 0;const span=Math.min(18,Math.max(10,(Number(duration)||60)*.18)),enter=Math.min(2.5,span*.22),hold=Math.min(4.1,span*.32),ease=x=>{x=Math.max(0,Math.min(1,x));return x*x*(3-2*x);};if(seconds<enter)return ease(seconds/enter);if(seconds<hold)return 1;if(seconds<span)return 1-ease((seconds-hold)/(span-hold));return 0;}
 const api={title,roman,opacityAt};root.CycleTitles=api;if(typeof module!=='undefined'&&module.exports)module.exports=api;
})(typeof globalThis!=='undefined'?globalThis:window);
