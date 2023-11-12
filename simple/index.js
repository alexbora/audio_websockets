const http = require('http');

const reqListener = function (req, res) {
  let counter = 0;

  res.writeHead(200, {
    'Content-Type': 'text/event-stream',
    'Connection': 'keep-alive',
    'Cache-Control': 'no-cache',
    'Access-Control-Allow-Origin': '*'
  });

  setInterval(function() {
    res.write(`data sent ok: ${counter}\n\n`);
    counter++;
  }, 2000);

}

const server = http.createServer(reqListener);
server.listen(3000);
