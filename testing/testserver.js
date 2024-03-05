/*
 * TS, 2020-2024
 * This is a NodeJS bases server to mimic Cloud API for testing without 
 * calling for each simple test the real service online
 *
 */

const http = require('http');
const fs = require('fs');

const filesystemModel = "./remoteFsModel.json";

class Context {

  parsedModelData = null;

  constructor() {

    console.log("Opening file: " + filesystemModel);

    fs.readFile(filesystemModel, function (err, data) {
      this.parsedModelData = JSON.parse(data);
      if(err) {
        console.err("Cant open File structure file", filesystemModel , err);
        process.exit(1);
      }
      this.parsedModelData = JSON.parse(data);

    }.bind(this));
  }



  getFolders(url,callback) {

    var context = {
      "returncode": 200,
      "mimetype": "application/json"
    };
    
    if(this.parsedModelData[url] === undefined) {
      console.error("Cant find URL:", url);
      context.returncode = 404;
      context.data = "[]";
      callback(context);
      return;
    }

    context.data = JSON.stringify(this.parsedModelData[url]);

    callback(context);
  }
  
  getBlob(url,callback) {
    var filename = "./" + url + ".svg";
    var context = {
      "returncode": 200,
      "mimetype": "image/svg+xml"
    };

    fs.readFile(filename, function (err, data) {

      if (err) {
        console.error("Cant find URL:", url, err, parsedData[url]);
        context.returncode = 404;
        context.data = "";
        callback(context);
      }
      
      context.data = data;
      callback(context);
      return;    

    });
  }


}



var context = new Context();


const server = http.createServer(function (req, res) {
  var url = req.url;
  var result = null;

  
  
  var callback = function(context) {
    res.writeHead(context.returncode, { 'Content-Type': context.mimetype });
    res.end(context.data);
  }


  url = url.replace("//", "/");
  console.log("Reqest.URL: " + url);

  if (url.match(/^\/blobs\/.*/)) {
    context.getBlob(url, callback);
  } else {
    context.getFolders(url, callback);
  }

});

const port = 3000;
const host = '127.0.0.1';
server.listen(port, host);
console.log(`Listening at http://${host}:${port}`);

