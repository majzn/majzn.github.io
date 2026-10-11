const CACHE='continuum-samples-v1';
self.addEventListener('install',()=>self.skipWaiting());
self.addEventListener('activate',e=>e.waitUntil(self.clients.claim()));
self.addEventListener('fetch',event=>{const u=new URL(event.request.url);if(u.origin==='https://gleitz.github.io'&&u.pathname.startsWith('/midi-js-soundfonts/'))event.respondWith((async()=>{const cache=await caches.open(CACHE),saved=await cache.match(event.request);if(saved)return saved;const response=await fetch(event.request);if(response.ok)await cache.put(event.request,response.clone());return response;})());});
