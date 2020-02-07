const http = require('http');
const fs = require('fs');

const server = http.createServer(function(req, res) {

 fs.readFile("./" + req.url, function (err,data) {
    if (err) {
      res.writeHead(404);
      res.end(JSON.stringify(err));
      return;
    }
    res.writeHead(200, {'Content-Type': 'application/json'});

    res.end(data);
  });

/*
  console.dir(request.param);

  if (request.method == 'POST') {
    console.log('POST');
    var body = ''
    request.on('data', function(data) {
      body += data
      console.log('Partial body: ' + body);
    });
    request.on('end', function() {
      console.log('Body: ' + body);
      response.writeHead(200, {'Content-Type': 'application/json'});
      response.end(`
	{ 
	    'status': 'OK'
	}`);
    })
  } else {
    console.log('GET');
    var json = `

    `
    response.writeHead(200, {'Content-Type': 'application/json'})
    response.end(html);
  }
*/


});

const port = 3000;
const host = '127.0.0.1';
server.listen(port, host);
console.log(`Listening at http://${host}:${port}`);

