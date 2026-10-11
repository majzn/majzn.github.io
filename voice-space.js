'use strict';
(function(root){
 const clamp=(x,a,b)=>Math.max(a,Math.min(b,x));
 const clean=p=>({x:clamp(Number.isFinite(p?.x)?p.x:.5,-1.5,2.5),y:clamp(Number.isFinite(p?.y)?p.y:.5,-1.5,2.5)});
 const heightBounds=available=>({min:175,max:Math.min(650,Math.max(175,Math.floor(available)-122))});
 const viewClean=p=>({zoom:clamp(Number.isFinite(p?.zoom)?p.zoom:1,.3,4),panX:clamp(Number.isFinite(p?.panX)?p.panX:0,-2800,2800),panY:clamp(Number.isFinite(p?.panY)?p.panY:0,-2000,2000)});
 function hash(s){let h=2166136261;for(const c of String(s)){h^=c.charCodeAt(0);h=Math.imul(h,16777619);}return h>>>0;}
 function rng(seed){let s=hash(seed);return()=>{s=(Math.imul(1664525,s)+1013904223)>>>0;return s/4294967296;};}
 function initial(i,n){const a=-Math.PI/2+2*Math.PI*i/Math.max(n,2),radius=.34;return clean({x:.5+Math.cos(a)*radius,y:.5+Math.sin(a)*radius});}
 // The microphone is unity at its exact location and zero at its acoustic horizon.
 function params(p){const v=clean(p),dx=v.x-.5,dy=v.y-.5,distance=clamp(Math.hypot(dx,dy)/.78,0,1);return {pan:clamp(dx/.7,-1,1),gain:Math.pow(1-distance,1.15),distance};}
 function project(p,view){const v=viewClean(view),c=clean(p);return{x:(c.x-.5)*850*v.zoom+v.panX,y:(c.y-.5)*480*v.zoom+v.panY};}
 function unproject(dx,dy,view){const v=viewClean(view);return clean({x:.5+(dx-v.panX)/(850*v.zoom),y:.5+(dy-v.panY)/(480*v.zoom)});}
 function fromPointer(rect,x,y,view){return unproject(x-rect.left-rect.width/2,y-rect.top-rect.height/2,view);}
 function formation(type,n,seed='counterpoint'){
  const random=rng(seed+':'+type+':'+n),result=[];
  for(let i=0;i<n;i++){
   const t=n===1?.5:i/(n-1),a=2*Math.PI*i/n,ring=.32,noise=()=>random()-.5;
   let x=.5,y=.5;
   if(type==='circle'){x+=ring*Math.cos(a-Math.PI/2);y+=ring*Math.sin(a-Math.PI/2);}
   else if(type==='arc'){x+=(t-.5)*.85;y+=.1-.32*Math.sin(Math.PI*t);}
   else if(type==='hyperbola'){const u=(t-.5)*2;x+=u*.48;y+=(i%2?1:-1)*(.12+.25/(1+7*u*u));}
   else if(type==='gaussian'){const radius=Math.sqrt(-2*Math.log(Math.max(1e-7,random())))*.19,theta=random()*Math.PI*2;x+=radius*Math.cos(theta);y+=radius*Math.sin(theta);}
   else if(type==='orchestra'){const tier=i<Math.ceil(n*.5)?0:1,local=tier?i-Math.ceil(n*.5):i,span=tier?n-Math.ceil(n*.5):Math.ceil(n*.5),theta=Math.PI*(.12+.76*(local+.5)/Math.max(1,span));x+=Math.cos(theta)*(tier?.43:.26);y+=Math.sin(theta)*(tier?.35:.21)-.18;}
   else if(type==='spiral'){const radius=.10+i*.055;x+=radius*Math.cos(a*1.45);y+=radius*Math.sin(a*1.45);}
   else if(type==='line'){x+=(t-.5)*.95;y+=.09*Math.sin(i*1.7);}
   else if(type==='clusters'){const group=i%3,angle=group*2*Math.PI/3,radius=.28;x+=radius*Math.cos(angle)+(noise()*.15);y+=radius*Math.sin(angle)+(noise()*.15);}
   result.push(clean({x,y}));
  }
  return result;
 }
 class VoiceSpace{
  constructor(storage=null){this.storage=storage;this.positions=[];this.view=viewClean({});this.height=260;try{const state=JSON.parse(storage?.getItem('continuum-voice-space-v1')||'null');if(Array.isArray(state?.positions))this.positions=state.positions.slice(0,8).map(p=>p?clean(p):null);if(state?.view)this.view=viewClean(state.view);if(Number.isFinite(state?.height))this.height=clamp(state.height,175,650);}catch{}}
  get(v,n){return this.positions[v]||initial(v,n);}
  set(v,p,persist=true){this.positions[v]=clean(p);if(persist)this.save();return this.positions[v];}
  reset(v){this.positions[v]=null;this.save();}
  remove(v){this.positions.splice(v,1);this.save();}
  insert(v){this.positions.splice(v,0,null);this.save();}
  setView(view,persist=true){this.view=viewClean(view);if(persist)this.save();}
  setHeight(h,persist=true){this.height=clamp(h,175,650);if(persist)this.save();}
  save(){try{this.storage?.setItem('continuum-voice-space-v1',JSON.stringify({version:2,positions:this.positions,view:this.view,height:this.height}));}catch{}}
 }
 root.VoiceSpace=VoiceSpace;root.VoiceSpaceMath={initial,clean,params,fromPointer,project,unproject,formation,viewClean,heightBounds};
 if(typeof module==='object'&&module.exports)module.exports={VoiceSpace,initial,clean,params,fromPointer,project,unproject,formation,viewClean,heightBounds};
})(typeof globalThis!=='undefined'?globalThis:window);
