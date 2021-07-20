// import modules
const net = require('net');
const http = require('http');

// required constants
const BAD_REQUEST = '0';
const EMPTY_RESPONSE = '1';
const RESET_RESPONSE = '2';

// holds device ids that have to be reset
const resetStatus = [];


// logs the device status to the frontend server
async function logStatus(deviceId, status) {
  console.log(deviceId, status);
  return new Promise((resolve, reject) => {
    http.get(`http://legend-fair-syrup.glitch.me/api/log?secret=abcdefgh&id=${deviceId}&status=${status}`, {
      headers: {
        'User-Agent': 'nodejs'
      }
    }, res => {
      resolve();
    });
  });
}


// processes requests to server
function processData(data) {
  data = data.trim().split(':');
  
  switch ( data[0] ) {
    // request by server to reset device
    case 'COM':
      resetStatus.push(data[1]);
      return EMPTY_RESPONSE;
      break;
      
    // request by device for status log
    case 'STA':
      logStatus(data[1], data[2]);
      
      if ( resetStatus.indexOf(data[1]) !== -1 ) {
        resetStatus.splice( resetStatus.indexOf( data[1] ), 1 );
        return RESET_RESPONSE;
      } else {
        return EMPTY_RESPONSE;
      }
      break;
  }
  return BAD_REQUEST;
}

const server = net.createServer({
  allowHalfOpen: true
}, socket => {
  socket.setEncoding('utf8');
  socket.setTimeout(100000);
  
  socket.on('data', data => {
    let output = processData(data);
    socket.write(output + "\n", () => socket.end());
  });
  
  socket.on('timeout', () => socket.end());
  
  socket.on('end', () => console.log('end'));
});

server.listen(4000);
