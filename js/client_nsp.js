var chat = require('socket.io-client')('http://localhost:8080/chat',
                                       { "transports":['websocket']

});
var news = require('socket.io-client')('http://localhost:8080/news',{
        "transports":['websocket']

});
      
    chat.on('connect', function () {
            chat.emit('hi!');
              
    });
  
news.on('news', function () {
        news.emit('woot');
          
});
