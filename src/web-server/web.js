const express = require('express');
const app = express();
const ws = require('ws');
const http = require('http');
const net = require('net');

const server = http.createServer(app);
const wss = new ws.Server({ server });

const devices = {};

const host = "";
const port = 0;

app.use(express.static('public'));

wss.on('connection', ws => {
  ws.on('message', console.log);
});

app.get('/', (req, res) => {
  res.sendFile('src/index.html', {
    root: '.'
  });
});

app.get('/api/log', (req, res) => {
  if ( !req.query['secret'] || req.query['secret'] !== process.env.SECRET ) {
    res.send('Unauthorized');
    return;
  }
  let deviceId = req.query['id'];
  let deviceStatus = req.query['status'];
  
  if ( !deviceId || !deviceStatus || typeof deviceId !== 'string' || typeof deviceStatus !== 'string' ) {
    res.send('Invalid');
  }
  
  devices[deviceId] = deviceStatus;
  
  wss.clients.forEach(client => {
    if (client.readyState === ws.OPEN) {
      let changes = {};
      changes[deviceId] = deviceStatus;
      client.send(JSON.stringify(changes));
    }
  });
  
  res.send('200');
});

app.get('/api/reset', (req, res) => {
  if ( !req.query['id'] ) {
    res.send('Invalid');
    return;
  }
  
  let client = net.createConnection({
    host: host,
    port: port
  }, () => {
    client.write(`COM:${req.query['id']}\n`);
  });
  
  client.on('data', () => client.end())
  res.send('200');
});

app.get('/api', (req, res) => {
  res.send(JSON.stringify(devices));
});

server.listen(process.env.PORT);