class ContinuumRecorder extends AudioWorkletProcessor {
 constructor(){super();this.left=new Float32Array(4096);this.right=new Float32Array(4096);this.used=0;this.running=true;this.port.onmessage=e=>{if(e.data==='stop'){this.flush();this.running=false;this.port.postMessage({done:true});}};}
 flush(){if(!this.used)return;const left=this.left.slice(0,this.used),right=this.right.slice(0,this.used);this.port.postMessage({left,right},[left.buffer,right.buffer]);this.used=0;}
 process(inputs){if(!this.running)return false;const input=inputs[0];if(input?.[0])for(let i=0;i<input[0].length;i++){this.left[this.used]=input[0][i];this.right[this.used]=input[1]?.[i]??input[0][i];if(++this.used===4096)this.flush();}return true;}
}
registerProcessor('continuum-recorder',ContinuumRecorder);
