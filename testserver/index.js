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


const ws = require('nodejs-websocket');
 

var server = ws.createServer(function (conn) {
    
    console.log("New connection")

    conn.on("binary", function (inStream) {

        var data = Buffer.alloc(0)

        inStream.on("readable", function () {

            console.log("begin read");

            var newData = inStream.read()
            if (newData)
                data = Buffer.concat([data, newData], data.length+newData.length)
        })

        inStream.on("end", function () {
            console.log("Received " + data.length + " bytes of binary data")
            conn.sendBinary(data);
        })
    })
    conn.on("close", function (code, reason) {
        console.log("Connection closed")
    })

}).listen(80)