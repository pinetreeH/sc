var io = require('socket.io-client')("http://localhost:8080",{
    "transports":['websocket']
});
//socket.on('news', function (data) {
//        console.log(data);
//        socket.emit('my other event', { my: 'data'  });
      
//});

io.on('connection',function(socket){
    socket.join('room_yy');
    socket.emit("hhh");
}

