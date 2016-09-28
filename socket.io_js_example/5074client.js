var io = require('socket.io-client')("http://localhost:5074",{
    "transports":['websocket']
});
var socket = io;
socket.on('connect', function () {
        console.log('clent now  emit data...');
        socket.emit('my other event', { my: 'data'  });
});

socket.on('news', function (data) {
        console.log('clent recv server data:',data);
});
