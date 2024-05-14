const cacheName = "sub-mare-imperium-derelict-v5";

const appShellFiles = [
    "/",
    "index.html",
    "index.js",
    "index.data",
    "index.wasm",
    "favicon.ico",
    "apple-touch-icon.png",
    "apple-touch-icon-precomposed.png",
    "pwa/apple/iphonex_splash.png",
    "pwa/apple/ipadpro2_splash.png",
    "pwa/apple/iphonexsmax_splash.png",
    "pwa/apple/iphoneplus_splash.png",
    "pwa/apple/iphone6_splash.png",
    "pwa/apple/single-page-icon.png",
    "pwa/apple/ipadpro3_splash.png",
    "pwa/apple/iphonexr_splash.png",
    "pwa/apple/ipadpro1_splash.png",
    "pwa/apple/iphone5_splash.png",
    "pwa/apple/ipad_splash.png",

];

const gamesImages = [];
const contentToCache = appShellFiles.concat(gamesImages);

self.addEventListener("install", (e) => {
    console.log("[Service Worker] Install");
    e.waitUntil(
        (async() => {
            const cache = await caches.open(cacheName);
            console.log("[Service Worker] Caching all: app shell and content");
            await cache.addAll(contentToCache);
        })(),
    );
});

self.addEventListener("fetch", (e) => {
   e.respondWith(
       (async() => {
           const r = await caches.match(e.request);
           console.log(`[Service Worker] Fetching resource: ${e.request.url}`);
           if (r) {
               return r;
           }

           const response = await fetch(e.request);
           const cache = await caches.open(cacheName);
           console.log(`[Service Worker] Caching new resource: ${e.request.url}`);
           cache.put(e.request, response.clone());
           return response;
       })(),
   );
});

self.addEventListener("activate", (e) => {
    e.waitUntil(
        caches.keys().then((keyList) => {
            return Promise.all(
                keyList.map((key) => {
                    if (key === cacheName) {
                        return;
                    }
                    return caches.delete(key);
                }),
            );
        }),
    );
});
