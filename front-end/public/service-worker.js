importScripts('https://storage.googleapis.com/workbox-cdn/releases/6.1.1/workbox-sw.js');

workbox.routing.registerRoute(
    ({request}) => request.destination === 'icons',
    new workbox.strategies.CacheFirst(),
);