const { log, error } = require('node:console');
const net = require('node:net'); 
const PacketReader = require('packet-reader')
const Transform = require('node:stream').Transform; 
const chalk = require('chalk'); 

function addLengthPadding(chunk) {
  const lengthHeader = Buffer.alloc(4);
  lengthHeader.writeUint32BE(chunk.length);
  return Buffer.concat([lengthHeader, chunk]);
}

var server = net.createServer(socket => {
	log('client connected from: ' + socket.remoteAddress + ':' + socket.remotePort);

  const clientReader = new PacketReader();
  const clientTransform = new Transform({
    transform(chunk, encoding, callback) {
      clientReader.addChunk(chunk)
      /** @type {Buffer} */
      let packet;
      while (packet = clientReader.read()) {
        log(chalk.cyan('client packet: ' + packet.toString('hex').replace(/(..)/g, '$1 ')));
        this.push(addLengthPadding(packet));
      }
      callback();
    }
  });

  const serverReader = new PacketReader();
  const serverTransform = new Transform({
    transform(chunk, encoding, callback) {
      serverReader.addChunk(chunk)
      /** @type {Buffer} */
      let packet;
      while (packet = serverReader.read()) {
        log(chalk.magenta('server packet: ' + packet.toString('hex').replace(/(..)/g, '$1 ')));
        this.push(addLengthPadding(packet));
      }
      callback();
    }
  });

  const client = net.connect(11727, '127.0.0.1', () => {
    socket.pipe(clientTransform).pipe(client);
    client.pipe(serverTransform).pipe(socket);
  })

  client.on('error', err => {
    socket.destroy();
    error(err);
  });

  socket.on('error', err => {
    client.destroy();
    error(err);
  });
});

server.on('error', error);

const PORT = process.env.PORT || 11728;
server.listen(PORT, '127.0.0.1', () => console.log(`server listening on port ${PORT}`));