var socket = require('socket.io-client')("http://localhost:8080/yy",{
    "transports":['websocket']
});
socket.on('news', function (data) {
        console.log(data);
        socket.emit('my other event', { my: 'data'  });
      
});
