'use strict'

// C library API
const ffi = require('ffi');

var connection = null;

let sharedLib = ffi.Library("./sharedLib", {
"gedcomToJSON": ["string", ["string"]],
"jsonToNewFile": ["void", ["string", "string"]],
"addIndJSON": ["void", ["string", "string"]],
"parseIndividualsToJSON": ["string", ["string"]],
"getDescendantsJSON": ["string", ["string", "string", "int"]],
"getAncestorsJSON": ["string", ["string", "string", "int"]]
});

var host = 'dursley.socs.uoguelph.ca';
var username ='usernameGoesHere';
var password = 'passwordGoesHere';
var database = 'databaseNameGoesHere';

const mysql = require('mysql');

// Express App (Routes)
const express = require("express");
const app     = express();
const path    = require("path");
const fileUpload = require('express-fileupload');

app.use(fileUpload());

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


  let uploadFile = req.files.uploadFile;

  if (uploadFile == null) {
    res.redirect('/');
  }
  // Use the mv() method to place the file somewhere on your server
  uploadFile.mv('uploads/' + uploadFile.name, function(err) {
    if(err) {
      return res.status(500).send(err);
    }
    res.redirect('/');
  });
});

//Respond to GET requests for files in the uploads/ directory
app.get('/uploads/:name', function(req , res){
  fs.stat('uploads/' + req.params.name, function(err, stat) {
    console.log(err);
    if(err == null) {
      res.sendFile(path.join(__dirname+'/uploads/' + req.params.name));
    } else {
      res.send('');
    }
  });
});

//******************** Your code goes here ******************** 

app.get('/getFile', function(req , res){
  var directory = fs.readdirSync("./uploads/");
  res.send({directory});
});

app.get('/parseFile', function(req , res){

  let filename = "uploads/" + String(req.query.filename);
  let objectString = sharedLib.gedcomToJSON(filename);
  let object = JSON.parse(objectString);
  res.send(object);
});

app.get("/create", function(req, res) {
  if (!req.query.fileName.endsWith(".ged")) {
    res.redirect('/');
  }
  let obj = {"source":req.query.source, "gedcVersion":"5.5", "encoding":req.query.encoding, "subName":req.query.subName, "subAddress":req.query.address};
  let JSONstring = JSON.stringify(obj);
  let filename = "uploads/" + String(req.query.fileName);
  sharedLib.jsonToNewFile(JSONstring, filename);
  res.redirect('/');
});

app.get("/add", function(req, res) {
  let obj = {"givenName":req.query.gname, "surname":req.query.surname};
  let JSONstring = JSON.stringify(obj);
  let filename = "uploads/" + String(req.query.fnameInd);
  sharedLib.addIndJSON(filename, JSONstring);
  res.redirect('/');
});

app.get('/view', function(req , res){

  let filename = "uploads/" + String(req.query.filename);
  let objectString = sharedLib.parseIndividualsToJSON(filename);
  let object = JSON.parse(objectString);
  for (var i in object) {
    object[i].file_id = req.query.file_id;
  }
  res.send(object);
});

app.get('/getDescendants', function(req , res){

  let filename = "uploads/" + String(req.query.filename);
  let ind = {"givenName":req.query.givenName, "surname":req.query.surname};
  let JSONString = JSON.stringify(ind);
  var maxGen = Number(req.query.maxGen);
  let objectString = sharedLib.getDescendantsJSON(filename, JSONString, maxGen);
  let object = JSON.parse(objectString);
  res.send(object);
});

app.get('/getAncestors', function(req , res){

  let filename = "uploads/" + String(req.query.filename);
  let ind = {"givenName":req.query.givenName, "surname":req.query.surname};
  let JSONString = JSON.stringify(ind);
  var maxGen = Number(req.query.maxGen);
  let objectString = sharedLib.getAncestorsJSON(filename, JSONString, maxGen);
  let object = JSON.parse(objectString);
  res.send(object);
});

app.get('/getConnection', function(req , res){
  let object = {"host": host, "username": username, "password": password, "database": database};
  res.send(object);
});

app.get('/connect', function(req , res){
  host = req.query.host;
  username = req.query.username;
  password = req.query.password;
  database = req.query.database;

  connection = mysql.createConnection({
      host     : 'dursley.socs.uoguelph.ca',
      user     : username,
      password : password,
      database : database
  });
  connection.connect();

  connection.query("SELECT * FROM information_schema.tables", function (err, rows, fields) {
    if (err) {
      console.log("Something went wrong. "+err);
      host = 'dursley.socs.uoguelph.ca';
      username ='usernameGoesHere';
      password = 'passwordGoesHere';
      database = 'databaseNameGoesHere';
    }

    else {
      connection.query("create table FILE (file_id int auto_increment, file_Name varchar(60) not null, source varchar(250) not null, version varchar(10) not null, encoding varchar(10) not null, sub_name varchar(62) not null, sub_addr varchar(256), num_individuals int, num_families int, primary key(file_id) )", function (err, rows, fields) {
      if (err) console.log("Something went wrong. "+err);
    });

    connection.query("create table INDIVIDUAL (ind_id int auto_increment, surname varchar(256) not null, given_name varchar(250) not null, sex varchar(1), fam_size int, source_file int, primary key(ind_id), FOREIGN KEY(source_file) REFERENCES FILE(file_id) ON DELETE CASCADE)", function (err, rows, fields) {
      if (err) console.log("Something went wrong. "+err);
    });

    res.send(req.query);
    }
  });

});

app.get('/clear', function(req , res){
  connection.query("delete from FILE", function (err, rows, fields) {
    if (err) {
        console.log("Something went wrong. "+err);
    }
  });

  connection.query("delete from INDIVIDUAL", function (err, rows, fields) {
    if (err) {
        console.log("Something went wrong. "+err);
    }
  });

  res.send(req.query);
});

app.get('/displayStatus', function(req , res){
  
  connection.query("SELECT * from FILE", function (err, rows, fields) {
    if (err) {
        console.log("Something went wrong. "+err);
    }
    var fileCount = rows.length;

    connection.query("SELECT * from INDIVIDUAL", function (err, rows, fields) {
      if (err) {
          console.log("Something went wrong. "+err);
      }
      var indCount = rows.length;
      console.log("Database has " + fileCount + " files and " + indCount + " individuals");
    });
  });
  res.send(req.query);
});

app.get('/fileTable', function(req , res){
  
  var data = req.query;
  var subAddress;
  if (data.subAddress = "") {
    subAddress = null;
  }

  else {
    subAddress = "\'" + data.subAddress + "\'";
  }
  var insert = "INSERT INTO FILE (file_Name, source, version, encoding, sub_name, sub_addr, num_individuals, num_families) VALUES (\'" + data.filename + "\', \'" + data.source + "\', \'" + data.gedcVersion + "\', \'" + data.encoding + "\', \'" + data.subName + "\',"+ subAddress + ", \'" + data.numIndividuals + "\', \'" + data.numFamilies + "\')";
  connection.query(insert, function (err, rows, fields) {
      if (err) {
        console.log("Something went wrong. "+err);
      }

      var select = "SELECT file_id FROM FILE WHERE file_Name = \'" + data.filename + "\'";
      connection.query(select, function (err, rows, fields) {
          if (err) {
            console.log("Something went wrong. "+err);
          }
          data["file_id"] = rows[0].file_id;
          res.send(data);
      });
  });
});

app.get('/indTable', function(req , res){

  var data = req.query;
  var insert = "INSERT INTO INDIVIDUAL (surname, given_name, sex, fam_size, source_file) VALUES (\'" + data.surname + "\', \'" + data.givenName + "\', \'" + data.sex + "\', \'" + data.numFamily + "\', \'" + data.filename + "\')";
  connection.query(insert, function (err, rows, fields) {
    if (err) {
      console.log("Something went wrong. "+err);
    }
  });
  res.send(data);
});

app.get('/query1', function(req , res){

  connection.query("SELECT * FROM INDIVIDUAL ORDER BY surname", function (err, rows, fields) {
    
    if (err) 
        console.log("Something went wrong. "+err);
    else {
        var array = [];
        var i = 0;
        for (let row of rows){
            var object = {};
            object.ind_id = row.ind_id;
            object.surname = row.surname;
            object.given_name = row.given_name;
            object.sex = row.sex;
            object.fam_size = row.fam_size;
            object.source_file = row.source_file;
            array[i] = object;
            ++i;
        }
        res.send(array);
    }

  });
});

app.get('/query2', function(req , res){

  var select = "SELECT file_id FROM FILE WHERE file_Name = \'" + req.query.filename + "\'";
  connection.query(select, function (err, rows, fields) {
      if (err) {
        console.log("Something went wrong. "+err);
      }

      if (rows[0] == null) {
        var array = [];
        res.send(array);
      }

      else {
      
        var file_id = rows[0].file_id;
    
        connection.query("SELECT * FROM INDIVIDUAL WHERE source_file = " + file_id + " ORDER BY surname", function (err, rows, fields) {
          
        if (err) 
            console.log("Something went wrong. "+err);
        else {
            var array = [];
            var i = 0;
            for (let row of rows){
                var object = {};
                object.ind_id = row.ind_id;
                object.surname = row.surname;
                object.given_name = row.given_name;
                object.sex = row.sex;
                object.fam_size = row.fam_size;
                object.source_file = row.source_file;
                array[i] = object;
                ++i;
            }
            res.send(array);
        }

      });
    }
  });

});

app.get('/query3', function(req , res){

  connection.query("SELECT * FROM FILE WHERE encoding = \'ASCII\'", function (err, rows, fields) {
    
    if (err) 
        console.log("Something went wrong. "+err);
    else {
         var array = [];
          var i = 0;
          for (let row of rows){
              var object = {};
              object.file_id = row.file_id;
              object.file_Name = row.file_Name;
              object.source = row.source;
              object.version = row.version;
              object.encoding = row.encoding;
              object.sub_name = row.sub_name;
              object.sub_addr = row.sub_addr;
              object.num_individuals = row.num_individuals;
              object.num_families = row.num_families;
              array[i] = object;
              ++i;
          }
          res.send(array);
    }

  });
});

app.get('/query4', function(req , res){

  connection.query("SELECT * FROM INDIVIDUAL WHERE fam_size >= 5 ORDER BY surname", function (err, rows, fields) {
    
    if (err) 
        console.log("Something went wrong. "+err);
    else {
        var array = [];
        var i = 0;
        for (let row of rows){
            var object = {};
            object.ind_id = row.ind_id;
            object.surname = row.surname;
            object.given_name = row.given_name;
            object.sex = row.sex;
            object.fam_size = row.fam_size;
            object.source_file = row.source_file;
            array[i] = object;
            ++i;
        }
        res.send(array);
    }

  });
  
});

app.get('/query5', function(req , res){

  connection.query("SELECT * FROM INDIVIDUAL WHERE sex = \'F\' ORDER BY surname", function (err, rows, fields) {
    
    if (err) 
        console.log("Something went wrong. "+err);
    else {
          var i = 0;
          var array = [];
          for (let row of rows){
              var object = {};
              object.ind_id = row.ind_id;
              object.surname = row.surname;
              object.given_name = row.given_name;
              object.sex = row.sex;
              object.fam_size = row.fam_size;
              object.source_file = row.source_file;
              array[i] = object;
              ++i;
          }
          res.send(array);
    }

  });
});

app.get('/helpInd', function(req , res){

  connection.query("DESCRIBE INDIVIDUAL", function (err, rows, fields) {
    
    if (err) 
        console.log("Something went wrong. "+err);
    else {
          var array = [];
          var i = 0;
          for (let row of rows){
              var object = {};
              object.Field = row.Field;
              object.Type = row.Type;
              object.Null = row.Null;
              object.Key = row.Key;
              object.Default = row.Default;
              object.Extra = row.Extra;
              array[i] = object;
              ++i;
          }
          res.send(array);
    }

  });
});

app.get('/helpFile', function(req , res){

  connection.query("DESCRIBE FILE", function (err, rows, fields) {
    
    if (err) 
        console.log("Something went wrong. "+err);
    else {
          var array = [];
          var i = 0;
          for (let row of rows){
              var object = {};
              object.Field = row.Field;
              object.Type = row.Type;
              object.Null = row.Null;
              object.Key = row.Key;
              object.Default = row.Default;
              object.Extra = row.Extra;
              array[i] = object;
              ++i;
          }
          res.send(array);
    }

  });
});


app.get('/find', function(req , res){

  var search = req.query.search;
  connection.query(search, function (err, rows, fields) {
    
    if (err) {
        console.log("Something went wrong. "+err);
        object = {};
        object.error = "Invalid SQL statement";
        res.send(object);
    }

    else {
        var array = [];
        var split = search.split(" ");
        var index = split.indexOf("FROM");
        if (split[index + 1] == "FILE") {
          var i = 0;
          for (let row of rows){
              var object = {};
              object.file_id = row.file_id;
              object.file_Name = row.file_Name;
              object.source = row.source;
              object.version = row.version;
              object.encoding = row.encoding;
              object.sub_name = row.sub_name;
              object.sub_addr = row.sub_addr;
              object.num_individuals = row.num_individuals;
              object.num_families = row.num_families;
              object.table = "FILE";
              array[i] = object;
              ++i;
          }
          
        }

        else {
          var i = 0;
          for (let row of rows){
              var object = {};
              object.ind_id = row.ind_id;
              object.surname = row.surname;
              object.given_name = row.given_name;
              object.sex = row.sex;
              object.fam_size = row.fam_size;
              object.source_file = row.source_file;
              object.table = "INDIVIDUAL";
              array[i] = object;
              ++i;
          }
        }
        
        res.send(array);
    }

  });
});

app.listen(portNum);
console.log('Running app at localhost: ' + portNum);