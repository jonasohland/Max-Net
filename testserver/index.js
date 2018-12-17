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


const protobuf = require('protobufjs');
const WebSocket = require('ws');
 
const wss = new WebSocket.Server({ port: 8080 });
 
var send_mess;

protobuf.load('../source/projects/shared/proto/generic_max.proto', (err, root) => {

    if(err){
        console.log(err);
        return;
    }

    var generic_max = root.lookupType("generic_max");

    var atom_float_array = root.lookupType("atom_float_array");

    var float_array_ = {
        values: [
            2.54556,
            6.867858
        ]
    };

    var mess = {
        atom: [   
            { 
                int_val: 2,
                type: 0
            }
        ]
    };

    var err = generic_max.verify(mess);

    if(!err){

        var out_mess = generic_max.create(mess);

        console.log(out_mess);

        send_mess = generic_max.encode(out_mess).finish();

        var mess_back = generic_max.decode(send_mess);

        console.log(JSON.stringify(generic_max.toObject(mess_back)));

        // console.log(send_mess);

    } 


});

wss.on('connection', (ws) => {

    ws.on('message', (message) => {
  
        console.log('received: %s', message);

        if(send_mess){

            ws.send(send_mess, (error) => {
                if(error){
                    console.log(error);
                }
            });

            console.log("sended: " + JSON.stringify(send_mess));
        }
  
    });

    ws.on('close', (code, reason) => {
        console.log(`connection closed: ${code} ${reason}`);
    });
 
  
});