// モジュールのロード
var http = require('http'),
    io = require('socket.io'),
    zmq = require('zeromq'),
    json = JSON.stringify;

// HTTPサーバーの作成
var server = http.createServer();
server.listen(8080);
var webSocket = io.listen(server);


//-------------------------------------------
// ソケットの作成
//-------------------------------------------
var zmqSocket = zmq.createSocket('req');
zmqSocket.connect('tcp://127.0.0.1:5555');
zmqSocket.on('message', function(data) {
  // Kinect -> node -> ブラウザ
  webSocket.broadcast(data);
});

webSocket.on('connection', function(client) {
  console.info('browser connected');
  client.on('message', function(data) {
    console.info('received from browser:', data);
    // ブラウザ -> node -> Kinect
    zmqSocket.send(data);
  });
  client.on('disconnect', function() {
    console.info('disconnect');
  });
});
