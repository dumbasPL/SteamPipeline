const { log, error } = require('node:console');
const net = require('node:net'); 
const {Transform} = require('node:stream'); 
const util = require('node:util');
const PacketReader = require('packet-reader')
const chalk = require('chalk');

const {values: args} = util.parseArgs({
  options: {
    'host': { type: 'string', short : 'h', default: '127.0.0.1' },
    'port': { type: 'string', short : 'p', default: '11727' },
    'listen-port': { type: 'string', short : 'l', default: '11728' },
  },
  strict: true,
})

function addLengthPadding(chunk) {
  const lengthHeader = Buffer.alloc(4);
  lengthHeader.writeUint32BE(chunk.length);
  return Buffer.concat([lengthHeader, chunk]);
}

var server = net.createServer(socket => {
	log('client connected from: ' + socket.remoteAddress + ':' + socket.remotePort);

  socket.setNoDelay(true);

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

  const client = net.connect(parseInt(args['port']), args['host'], () => {
    client.setNoDelay(true);

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
server.listen(parseInt(args['listen-port']), '0.0.0.0', () => {
  console.log(`server listening on ${server.address().address}:${server.address().port}`);
});