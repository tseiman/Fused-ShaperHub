/*
 * 
 * This is a NodeJS bases server to mimic OCtave API
 *
 *
 */

const http = require('http');
const fs = require('fs');

const server = http.createServer(function (req, res) {
  var url = req.url;
  url = url.replace("//","/");
  console.log("Reqest.URL: " + url);
  fs.readFile("./remoteFsModel.json", function (err, data) {
    var parsedData = JSON.parse(data);
    if (err || parsedData[url] === undefined) {
      console.error("Cant find URL:", url);
      res.writeHead(404);
      const body = [];
      console.log(JSON.stringify(body));
      res.end(JSON.stringify(body)); // JSON.stringify(err)
      return;
    }




    res.writeHead(200, { 'Content-Type': 'application/json' });

    res.end(JSON.stringify(parsedData[url]));
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

