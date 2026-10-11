importScripts('engine.js');
onmessage=e=>{const {id,settings,index}=e.data;try{postMessage({id,cycle:Fugue.compose(settings,index)});}catch(error){postMessage({id,error:error.message});}};
