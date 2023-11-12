// server.js

const http = require('http');

const server = http.createServer((req, res) => {
  // Set up SSE headers
  res.writeHead(200, {
    'Content-Type': 'text/event-stream',
    'Cache-Control': 'no-cache',
    'Connection': 'keep-alive',
  });

  // Send SSE events at intervals
  const intervalId = setInterval(() => {
    res.write(`data: ${JSON.stringify({ message: 'SSE event' })}\n\n`);
  }, 1000);

  // Close the connection after 10 seconds
  setTimeout(() => {
    clearInterval(intervalId);
    res.end();
  }, 1000*60*60);
});

const PORT = 3000;
server.listen(PORT, () => {
  console.log(`Server running at http://localhost:${PORT}`);
});

