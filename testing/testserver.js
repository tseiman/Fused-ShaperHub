/*
 * TS, 2020-2024
 * This is a NodeJS bases server to mimic Cloud API for testing without 
 * calling for each simple test the real service online
 *
 */

const http = require('http');
const fs = require('fs');

const server = http.createServer(function (req, res) {
  var url = req.url;
  var file = {
    "type": "json",
    "mimetype": "application/json",
    "name": "./remoteFsModel.json"
  }



  url = url.replace("//","/");
  console.log("Reqest.URL: " + url);

  if(url.match(/^\/blobs\/.*/)) {
    file = {
      "type": "svg",
      "mimetype": "image/svg+xml",
      "name": "./" + url + ".svg"
    }
  } 

  console.log("Opening file: " + file.name);
  fs.readFile(file.name, function (err, data) {
    var parsedData = {};
    if(file.type === 'json') { 
      parsedData = JSON.parse(data); 
      data = JSON.stringify(parsedData[url]);
    } else {
      parsedData[url] = 1; // evil hack to sattisfy sanity check below
    }

    if (err || parsedData[url] === undefined) {
      console.error("Cant find URL:", url,err,parsedData[url]);
      res.writeHead(404);
      const body = [];
      console.log(JSON.stringify(body));
      res.end(JSON.stringify(body)); // JSON.stringify(err)
      return;
    }

    
    res.writeHead(200, { 'Content-Type': file.mimetype });

    res.end(data);
  });

 

});

const port = 3000;
const host = '127.0.0.1';
server.listen(port, host);
console.log(`Listening at http://${host}:${port}`);

