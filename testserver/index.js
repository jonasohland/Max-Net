/*

var ws = require("nodejs-websocket");
 
// Scream server example: "hi" -> "HI!!!"
var server = ws.createServer(function (conn) {

    console.log("New connection");

    conn.on("text", function (str) {
        console.log("Received "+str);
        conn.sendText(str.toUpperCase()+"!!!");
    });

    conn.on("close", function (code, reason) {
        console.log("Connection closed");
    });

    conn.on('error', (err) => {
        console.log(`Connection ${err}`);

    });

});

server.socket.on('error', (err) => {
    console.log(err);
});

server.listen(80);

*/



const WebSocket = require('ws');
 
const wss = new WebSocket.Server({ port: 8080 });
 
wss.on('connection', (ws) => {
  ws.on('message', (message) => {
    console.log('received: %s', message);
  });

  ws.on('close', (code, reason) => {
    console.log(`connection closed: ${code} ${reason}`);
  });
 
  ws.send('something');
});