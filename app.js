'use strict'

// C library API
const ffi = require('ffi-napi');

// Express App (Routes)
const express = require("express");
const app     = express();
const path    = require("path");
const fileUpload = require('express-fileupload');

app.use(fileUpload());
app.use(express.static(path.join(__dirname+'/uploads')));

// Minimization
const fs = require('fs');
const JavaScriptObfuscator = require('javascript-obfuscator');

// Important, pass in port as in `npm run dev 1234`, do not change
const portNum = process.argv[2];

// Send HTML at root, do not change
app.get('/',function(req,res){
  res.sendFile(path.join(__dirname+'/public/index.html'));
});

// Send Style, do not change
app.get('/style.css',function(req,res){
  //Feel free to change the contents of style.css to prettify your Web app
  res.sendFile(path.join(__dirname+'/public/style.css'));
});

// Send obfuscated JS, do not change
app.get('/index.js',function(req,res){
  fs.readFile(path.join(__dirname+'/public/index.js'), 'utf8', function(err, contents) {
    const minimizedContents = JavaScriptObfuscator.obfuscate(contents, {compact: true, controlFlowFlattening: true});
    res.contentType('application/javascript');
    res.send(minimizedContents._obfuscatedCode);
  });
});

//Respond to POST requests that upload files to uploads/ directory
app.post('/upload', function(req, res) {
  if(!req.files) {
    return res.status(400).send('No files were uploaded.');
  }
  if (req.files.uploadFile == null) {
    res.redirect('/');
  }
  if (req.files.uploadFile != null) {
    let uploadFile = req.files.uploadFile;
    if (uploadFile.name.split('.').pop() != "svg") {
      return res.status(400).send('Invalid file type was uploaded. Upload Failed. Click back on browser to return to previous screen');
    }
 
  // Use the mv() method to place the file somewhere on your server
    uploadFile.mv('uploads/' + uploadFile.name, function(err) {
      if(err) {
        return res.status(500).send(err);
      }

      res.redirect('/');
    });
  }
});

//Respond to GET requests for files in the uploads/ directory
app.get('/uploads/:name', function(req , res){
  fs.stat('uploads/' + req.params.name, function(err, stat) {
    if(err == null) {
      res.sendFile(path.join(__dirname+'/uploads/' + req.params.name));
    } else {
      console.log('Error in file downloading route: '+err);
      res.send('');
    }
  });
});

//******************** Your code goes here ******************** 

let Lib = ffi.Library('./parser/bin/libsvgparse.so',{
    "createSVGJSON": ["string", ["string", "string"]],
    "getTitle": ["string", ["string", "string"]],
    "getDesc": ["string",["string", "string"]],
    "getPathsJSON": ["string",["string", "string"]],
    "getRectsJSON": ["string",["string", "string"]],
    "getCircsJSON": ["string",["string", "string"]],
    "getGroupsJSON": ["string",["string", "string"]],
    "getRectAtt": ["string", ["string", "string", "int"]],
    "getCircAtt": ["string", ["string", "string", "int"]],
    "getPathAtt": ["string", ["string", "string", "int"]],
    "getGroupAtt": ["string", ["string", "string", "int"]],
    "updateTitleDesc": ["int",["string", "string", "string", "string"]]
});





app.get('/allfiles', function(rec, res) {
  let fs = require('fs');
  let files = fs.readdirSync('uploads/');
  res.send(files);
})

app.get('/getSVGJSON', function(rec, res){
  let str = "uploads/" + rec.query.file;
  let Jstr = Lib.createSVGJSON(str, "parser/svg.xsd");
  res.send(Jstr);

})


//Sample endpoint
app.get('/someendpoint', function(req , res){
  let retStr = req.query.name1 + " " + req.query.name2;
  res.send({
    foo: retStr
  });
});

app.get('/titledesc', function(rec, res){
  let str = "uploads/" + rec.query.file;
  let title = Lib.getTitle(str, "parser/svg.xsd");
  let desc = Lib.getDesc(str, "parser/svg.xsd");
  res.send({
    Title: title,
    Desc: desc
  });

})

app.get('/rects', function(rec, res){
  let str = "uploads/" + rec.query.file;
  let rects = Lib.getRectsJSON(str, "parser/svg.xsd");
  res.send(rects);
});

app.get('/circs', function(rec, res){
  let str = "uploads/" + rec.query.file;
  let circs = Lib.getCircsJSON(str, "parser/svg.xsd");
  res.send(circs);


});

app.get('/paths', function(rec, res){
  let str = "uploads/" + rec.query.file;
  let paths = Lib.getPathsJSON(str, "parser/svg.xsd");
  res.send(paths);
});

app.get('/groups', function(rec, res){
  let str = "uploads/" + rec.query.file;
  let groups = Lib.getGroupsJSON(str, "parser/svg.xsd");
  res.send(groups);
  
});

app.get('/getAtt', function(rec, res){
  let str = "uploads/" + rec.query.file;
  let atts;
  
  if (rec.query.elem == 'r'){
    atts = Lib.getRectAtt(str,"parser/svg.xsd", rec.query.num );
  }
  if (rec.query.elem == 'c'){
    atts = Lib.getCircAtt(str,"parser/svg.xsd", rec.query.num );
  }
  if (rec.query.elem == 'p'){
    atts = Lib.getPathAtt(str,"parser/svg.xsd", rec.query.num );
  }
  if (rec.query.elem == 'g'){
    atts = Lib.getGroupAtt(str,"parser/svg.xsd", rec.query.num );
  }
  res.send(atts);

});

app.get('/updateTitle', function(rec, res){

  let title = rec.query.ttl;
  let desc = rec.query.dsc;
  let file = "uploads/" + rec.query.file;
  let pass = Lib.updateTitleDesc(file, "parser/svg.xsd", title, desc);
  res.send({stat:pass});



});


app.listen(portNum);
console.log('Running app at localhost: ' + portNum);