
// Put all onload AJAX calls here, and event listeners
$(document).ready(function() {

    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/getConnection',   //The server endpoint we are connecting to
        success: function (data) {

            $('#queryPanel').hide();
            $("#store").hide();

            document.getElementById("host").value = data.host;
            document.getElementById("user").value = data.username;
            document.getElementById("password").value = data.password;
            document.getElementById("database").value = data.database;

            if (data.username == "usernameGoesHere") {
                $('#myModal').modal('show');
            }
        },

        fail: function(error) {
            // Non-200 return, do something with error
            console.log(error); 
        }
    });

    // On page-load AJAX Example
    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/getFile',   //The server endpoint we are connecting to
        success: function (data) {
            /*  Do something with returned object
                Note that what we get is an object, not a string, 
                so we do not need to parse it on the server.
                JavaScript really does handle JSONs seamlessly
            */
            for (var i in data) {
                for (var j in data[i]) {

                    if (!data[i][j].endsWith(".ged")) {
                        var status = document.getElementById("statusPanel");
                        var p = document.createElement("p");
                        var node = document.createTextNode(data[i][j] + " could not be parsed");
                        p.appendChild(node);
                        status.appendChild(p);
                        continue;
                    }

                    $.ajax({
                        type: 'get',            //Request type
                        dataType: 'json',       //Data type - we will use JSON for almost everything 
                        url: '/parseFile',
                        data: {"filename": data[i][j]},
                        success: function (data) {
                            var table = document.getElementById("fileLog");
                            if (table.rows[1].cells.item(0).innerHTML == "No Files") {
                                table.deleteRow(1);
                            }
                            var row = table.insertRow(-1);
                            var cell1 = row.insertCell(0);
                            cell1.innerHTML = "<a href = /uploads/" + data.filename + ">" + data.filename + "</a>";
                            var cell2 = row.insertCell(1);
                            cell2.innerHTML = data.source;
                            var cell3 = row.insertCell(2);
                            cell3.innerHTML = data.gedcVersion;
                            var cell4 = row.insertCell(3);
                            cell4.innerHTML = data.encoding;
                            var cell5 = row.insertCell(4);
                            cell5.innerHTML = data.subName;
                            var cell6 = row.insertCell(5);
                            cell6.innerHTML = data.subAddress;
                            var cell7 = row.insertCell(6);
                            cell7.innerHTML = data.numIndividuals;
                            var cell8 = row.insertCell(7);
                            cell8.innerHTML = data.numFamilies;

                            var x = document.getElementById("fnameInd");
                            var option = document.createElement("option");
                            option.text = data.filename;
                            x.add(option);

                            x = document.getElementById("fnameDescendant");
                            option = document.createElement("option");
                            option.text = data.filename;
                            x.add(option);

                            x = document.getElementById("fnameAncestor");
                            option = document.createElement("option");
                            option.text = data.filename;
                            x.add(option);

                            x = document.getElementById("fnameView");
                            option = document.createElement("option");
                            option.text = data.filename;
                            x.add(option);

                            x = document.getElementById("fnameQuery");
                            option = document.createElement("option");
                            option.text = data.filename;
                            x.add(option);

                            var status = document.getElementById("statusPanel");
                            var p = document.createElement("p");
                            var node = document.createTextNode(data.filename + " was parsed");
                            p.appendChild(node);
                            status.appendChild(p);
                            $("#store").show();
                        },

                        fail: function(error) {
                            console.log(error);
                        }
                    });
                }
            }

            if (i == null) {
                var row = table.insertRow(1);
                var cell1 = row.insertCell(0);
                cell1.innerHTML = "No Files";
            }

            //We write the object to the console to show that the request was successful
            console.log(data); 
        },
        fail: function(error) {
            // Non-200 return, do something with error
            console.log(error); 
        }
    });

    $("#fnameView").on("change", function() {
        var files = document.getElementById("fnameView");
        var file = files.options[files.selectedIndex].value;
        var status = document.getElementById("statusPanel");
        var p = document.createElement("p");
        var node = document.createTextNode(file + " was chosen to be displayed in GEDCOM View Panel");
        p.appendChild(node);
        status.appendChild(p);
        $.ajax({
            type: 'get',            //Request type
            dataType: 'json',       //Data type - we will use JSON for almost everything 
            url: '/view',
            data: {"filename": file}, //The server endpoint we are connecting to
            success: function (data) {
                var table = document.getElementById("view");
                var x = table.rows.length;
                for (j = x - 1; j > 0; --j) {
                    table.deleteRow(j);
                }
                for (var i in data) {
                    var row = table.insertRow(-1);
                    var cell1 = row.insertCell(0);
                    cell1.innerHTML = data[i].givenName;
                    var cell2 = row.insertCell(1);
                    cell2.innerHTML = data[i].surname;
                    var cell3 = row.insertCell(2);
                    cell3.innerHTML = data[i].sex;
                    var cell4 = row.insertCell(3);
                    cell4.innerHTML = data[i].familyMembers;
                }
                if (i == null) {
                    var row = table.insertRow(1);
                    var cell1 = row.insertCell(0);
                    cell1.innerHTML = "No Individuals";
                }
            },

            fail: function(error) {
                // Non-200 return, do something with error
                console.log(error); 
            }
        });

    });

    $("#clear").click(function() {
        $("#statusPanel").empty();
    });

    $("#descendantsSubmit").click(function(e) {
        $("#descendantsTree").empty();
        e.preventDefault();
        var files = document.getElementById("fnameDescendant");
        var filename = files.options[files.selectedIndex].value;
        var gname = document.getElementById("gnameDescendant").value;
        var surname = document.getElementById("surnameDescendant").value;
        var maxGen = document.getElementById("maxGenDescendant").value;
        $.ajax({
            type: 'get',            //Request type
            dataType: 'json',       //Data type - we will use JSON for almost everything 
            url: '/getDescendants',   //The server endpoint we are connecting to
            data: {"filename": filename,
                    "givenName": gname,
                    "surname": surname,
                    "maxGen": maxGen},
            success: function (data) {
                var table = document.getElementById("descendantsTree");
                var title = document.createElement("h2");
                var text = document.createTextNode("Descendant Tree");
                title.append(text);
                table.append(title);
                for (var i in data) {
                    var tree = document.createElement("p");
                    var generation = "| ";
                    for (var j in data[i]) {
                        generation = generation.concat(data[i][j].givenName + " " + data[i][j].surname);
                        generation = generation.concat(" | ");
                        if ((Number(j) + 1) == data[i].length) {
                            var node = document.createTextNode(generation)
                            tree.appendChild(node);
                            table.appendChild(tree);
                        }
                    }
                }
                if (i == null) {
                    var tree = document.createElement("p");
                    var node = document.createTextNode("No Descendants");
                    tree.appendChild(node);
                    table.appendChild(tree);
                }

                var status = document.getElementById("statusPanel");
                var p = document.createElement("p");
                var node = document.createTextNode( data[i][j].givenName + " " + data[i][j].surname + " descendants displayed");
                p.appendChild(node);
                status.appendChild(p);
            },

            fail: function(error) {
                // Non-200 return, do something with error
                console.log(error); 
            }
        });
    });

    $("#ancestorsSubmit").click(function(e) {
        e.preventDefault();
        $("#descendantsTree").empty();
        
        var files = document.getElementById("fnameAncestor");
        var filename = files.options[files.selectedIndex].value;
        var gname = document.getElementById("gnameAncestor").value;
        var surname = document.getElementById("surnameAncestor").value;
        var maxGen = document.getElementById("maxGenAncestor").value;
        $.ajax({
            type: 'get',            //Request type
            dataType: 'json',       //Data type - we will use JSON for almost everything 
            url: '/getAncestors',   //The server endpoint we are connecting to
            data: {"filename": filename,
                    "givenName": gname,
                    "surname": surname,
                    "maxGen": maxGen},
            success: function (data) {
                var table = document.getElementById("descendantsTree");
                var title = document.createElement("h2");
                var text = document.createTextNode("Ancestor Tree");
                title.append(text);
                table.append(title);
                for (var i in data) {
                    var tree = document.createElement("p");
                    var generation = "| ";
                    for (var j in data[i]) {
                        generation = generation.concat(data[i][j].givenName + " " + data[i][j].surname);
                        generation = generation.concat(" | ");
                        if ((Number(j) + 1) == data[i].length) {
                            var node = document.createTextNode(generation)
                            tree.appendChild(node);
                            table.appendChild(tree);
                        }
                    }
                }
                if (i == null) {
                    var tree = document.createElement("p");
                    var node = document.createTextNode("No Ancestors");
                    tree.appendChild(node);
                    table.appendChild(tree);
                }

                var status = document.getElementById("statusPanel");
                var p = document.createElement("p");
                var node = document.createTextNode( data[i][j].givenName + " " + data[i][j].surname + " ancestors displayed");
                p.appendChild(node);
                status.appendChild(p);
            },

            fail: function(error) {
                // Non-200 return, do something with error
                console.log(error);
            }
        });
    });

    $("#modalSubmit").click(function(e) {
        e.preventDefault();
        var host = document.getElementById("host").value;
        var username = document.getElementById("user").value;
        var password = document.getElementById("password").value;
        var database = document.getElementById("database").value;
        $.ajax({
            type: 'get',            //Request type
            dataType: 'json',       //Data type - we will use JSON for almost everything 
            url: '/connect',   //The server endpoint we are connecting to
            data: {"host": host,
                    "username": username,
                    "password": password,
                    "database": database},
            success: function (data) {
                $('#myModal').modal('hide');
            },
            
            fail: function(error) {
                // Non-200 return, do something with error
                console.log(error);
            }
        });
    });

    $("#store").click(function(e) {
        e.preventDefault();
        $.ajax({
            type: 'get',            //Request type
            dataType: 'json',       //Data type - we will use JSON for almost everything 
            url: '/clear',   //The server endpoint we are connecting to
            success: function (data) {
            },
            
            fail: function(error) {
                // Non-200 return, do something with error
                console.log(error);
            }
        });
        var files = document.getElementById("fnameView");
        for (i = 0; i < files.options.length; ++i) {
            var file = files.options[i].value;
            $.ajax({
                type: 'get',            //Request type
                dataType: 'json',       //Data type - we will use JSON for almost everything 
                url: '/parseFile',
                data: {"filename": file},
                success: function (data) {
                    $.ajax({
                        type: 'get',            //Request type
                        dataType: 'json',       //Data type - we will use JSON for almost everything 
                        url: '/fileTable',   //The server endpoint we are connecting to
                        data: {"filename": data.filename,
                                "source": data.source,
                                "gedcVersion": data.gedcVersion,
                                "encoding": data.encoding,
                                "subName": data.subName,
                                "subAddress": data.subAddress,
                                "numIndividuals": data.numIndividuals,
                                "numFamilies": data.numFamilies},
                        success: function (data) {
                            var status = document.getElementById("statusPanel");
                            var p = document.createElement("p");
                            var node = document.createTextNode(data.filename + " stored in database");
                            p.appendChild(node);
                            status.appendChild(p);

                            $.ajax({
                                type: 'get',            //Request type
                                dataType: 'json',       //Data type - we will use JSON for almost everything 
                                url: '/view',
                                data: {"filename": data.filename,
                                        "file_id": data.file_id},
                                success: function (data) {
                                    for (var k in data) {
                                        $.ajax({
                                            type: 'get',            //Request type
                                            dataType: 'json',       //Data type - we will use JSON for almost everything 
                                            url: '/indTable',   //The server endpoint we are connecting to
                                            data: {"filename": data[k].file_id,
                                                    "givenName": data[k].givenName,
                                                    "surname": data[k].surname,
                                                    "sex": data[k].sex,
                                                    "numFamily": data[k].familyMembers},
                                            success: function (data) {
                                                console.log("success");
                                            },
                                
                                            fail: function(error) {
                                                // Non-200 return, do something with error
                                                console.log(error);
                                            }
                                        });
                                    }
                                },

                                fail: function(error) {
                                    // Non-200 return, do something with error
                                    console.log(error);
                                }
                            });
                        },
            
                        fail: function(error) {
                            // Non-200 return, do something with error
                            console.log(error);
                        }
                    });
                },

                fail: function(error) {
                    // Non-200 return, do something with error
                    console.log(error);
                }
            });
        }
    });

    $("#clearData").click(function(e) {
        e.preventDefault();
        $.ajax({
            type: 'get',            //Request type
            dataType: 'json',       //Data type - we will use JSON for almost everything 
            url: '/clear',   //The server endpoint we are connecting to
            success: function (data) {
                var status = document.getElementById("statusPanel");
                var p = document.createElement("p");
                var node = document.createTextNode("Database cleared");
                p.appendChild(node);
                status.appendChild(p);
            },
            
            fail: function(error) {
                // Non-200 return, do something with error
                console.log(error);
            }
        });
    });

    $("#displayStatus").click(function(e) {
        e.preventDefault();
        $.ajax({
            type: 'get',            //Request type
            dataType: 'json',       //Data type - we will use JSON for almost everything 
            url: '/displayStatus',   //The server endpoint we are connecting to
            success: function (data) {
                console.log(data);
            },
            
            fail: function(error) {
                // Non-200 return, do something with error
                console.log(error);
            }
        });
    });

    $("#execute").click(function(e) {
        e.preventDefault();
        $('#queryPanel').show();
    });

     $("#hide").click(function(e) {
        e.preventDefault();
        $('#queryPanel').hide();
    });

    $("#query1").click(function(e) {
        e.preventDefault();
        $.ajax({
            type: 'get',            //Request type
            dataType: 'json',       //Data type - we will use JSON for almost everything 
            url: '/query1',   //The server endpoint we are connecting to
            success: function (data) {

                var heading = document.getElementById("queryHeading");
                heading.innerHTML = "Individuals Sorted By Last Name";
                var table = document.getElementById("queryTable");
                var x = table.rows.length;
                for (j = x - 1; j > 0; --j) {
                    table.deleteRow(j);
                }
                table.deleteTHead();
                var header = table.createTHead();
                var row = header.insertRow(0);
                var cell1 = row.insertCell(0);
                cell1.innerHTML = "ID";
                var cell2 = row.insertCell(1);
                cell2.innerHTML = "Given Name"
                var cell3 = row.insertCell(2);
                cell3.innerHTML = "Surname"
                var cell4 = row.insertCell(3);
                cell4.innerHTML = "Sex"
                var cell5 = row.insertCell(4);
                cell5.innerHTML = "Family Size"
                var cell6 = row.insertCell(5);
                cell6.innerHTML = "Source File ID"

                for (var i in data) {
                    
                    var row = table.insertRow(-1);
                    var cell1 = row.insertCell(0);
                    cell1.innerHTML = data[i].ind_id;
                    var cell2 = row.insertCell(1);
                    cell2.innerHTML = data[i].given_name;
                    var cell3 = row.insertCell(2);
                    cell3.innerHTML = data[i].surname;
                    var cell4 = row.insertCell(3);
                    cell4.innerHTML = data[i].sex;
                    var cell5 = row.insertCell(4);
                    cell5.innerHTML = data[i].fam_size;
                    var cell6 = row.insertCell(5);
                    cell6.innerHTML = data[i].source_file;
                }
                var status = document.getElementById("statusPanel");
                var p = document.createElement("p");
                var node = document.createTextNode("Query 1 Table displayed");
                p.appendChild(node);
                status.appendChild(p);
                

            },
            
            fail: function(error) {
                // Non-200 return, do something with error
                console.log(error);
            }
        });
    });

    $("#query2").click(function(e) {
        e.preventDefault();
        var files = document.getElementById("fnameQuery");
        var file = files.options[files.selectedIndex].value;
        $.ajax({
            type: 'get',            //Request type
            dataType: 'json',       //Data type - we will use JSON for almost everything 
            url: '/query2',
            data: {"filename": file},  //The server endpoint we are connecting to
            success: function (data) {

                var heading = document.getElementById("queryHeading");
                heading.innerHTML = "Individuals in Current File";
                var table = document.getElementById("queryTable");
                var x = table.rows.length;
                for (j = x - 1; j > 0; --j) {
                    table.deleteRow(j);
                }
                table.deleteTHead();
                var header = table.createTHead();
                var row = header.insertRow(0);
                var cell1 = row.insertCell(0);
                cell1.innerHTML = "ID";
                var cell2 = row.insertCell(1);
                cell2.innerHTML = "Given Name"
                var cell3 = row.insertCell(2);
                cell3.innerHTML = "Surname"
                var cell4 = row.insertCell(3);
                cell4.innerHTML = "Sex"
                var cell5 = row.insertCell(4);
                cell5.innerHTML = "Family Size"
                var cell6 = row.insertCell(5);
                cell6.innerHTML = "Source File ID"

                for (var i in data) {
                    
                    var row = table.insertRow(-1);
                    var cell1 = row.insertCell(0);
                    cell1.innerHTML = data[i].ind_id;
                    var cell2 = row.insertCell(1);
                    cell2.innerHTML = data[i].given_name;
                    var cell3 = row.insertCell(2);
                    cell3.innerHTML = data[i].surname;
                    var cell4 = row.insertCell(3);
                    cell4.innerHTML = data[i].sex;
                    var cell5 = row.insertCell(4);
                    cell5.innerHTML = data[i].fam_size;
                    var cell6 = row.insertCell(5);
                    cell6.innerHTML = data[i].source_file;
                }

                var status = document.getElementById("statusPanel");
                var p = document.createElement("p");
                var node = document.createTextNode("Query 2 Table displayed");
                p.appendChild(node);
                status.appendChild(p);
            },
            
            fail: function(error) {
                // Non-200 return, do something with error
                console.log(error);
            }
        });
    });

    $("#query3").click(function(e) {
        e.preventDefault();
        $.ajax({
            type: 'get',            //Request type
            dataType: 'json',       //Data type - we will use JSON for almost everything 
            url: '/query3',   //The server endpoint we are connecting to
            success: function (data) {

                var heading = document.getElementById("queryHeading");
                heading.innerHTML = "Files Encoded in ASCII";
                var table = document.getElementById("queryTable");
                var x = table.rows.length;
                for (j = x - 1; j > 0; --j) {
                    table.deleteRow(j);
                }
                table.deleteTHead();
                var header = table.createTHead();
                var row = header.insertRow(0);
                var cell1 = row.insertCell(0);
                cell1.innerHTML = "ID";
                var cell2 = row.insertCell(1);
                cell2.innerHTML = "File Name"
                var cell3 = row.insertCell(2);
                cell3.innerHTML = "Source"
                var cell4 = row.insertCell(3);
                cell4.innerHTML = "Version"
                var cell5 = row.insertCell(4);
                cell5.innerHTML = "Encoding"
                var cell6 = row.insertCell(5);
                cell6.innerHTML = "Submitter Name"
                var cell7 = row.insertCell(6);
                cell7.innerHTML = "Submitter Address"
                var cell8 = row.insertCell(7);
                cell8.innerHTML = "Number of Individuals"
                var cell9 = row.insertCell(8);
                cell9.innerHTML = "Number of Families"

                for (var i in data) {
                    var row = table.insertRow(-1);
                    var cell1 = row.insertCell(0);
                    cell1.innerHTML = data[i].file_id;
                    var cell2 = row.insertCell(1);
                    cell2.innerHTML = data[i].file_Name;
                    var cell3 = row.insertCell(2);
                    cell3.innerHTML = data[i].source
                    var cell4 = row.insertCell(3);
                    cell4.innerHTML = data[i].version
                    var cell5 = row.insertCell(3);
                    cell5.innerHTML = data[i].encoding;
                    var cell6 = row.insertCell(4);
                    cell6.innerHTML = data[i].sub_name;
                    var cell7 = row.insertCell(5);
                    cell7.innerHTML = data[i].sub_addr;
                    var cell8 = row.insertCell(6);
                    cell8.innerHTML = data[i].num_individuals;
                    var cell9 = row.insertCell(7);
                    cell9.innerHTML = data[i].num_families;
                }

                var status = document.getElementById("statusPanel");
                var p = document.createElement("p");
                var node = document.createTextNode("Query 3 Table displayed");
                p.appendChild(node);
                status.appendChild(p);
            },
            
            fail: function(error) {
                // Non-200 return, do something with error
                console.log(error);
            }
        });
    });

    $("#query4").click(function(e) {
        e.preventDefault();
        $.ajax({
            type: 'get',            //Request type
            dataType: 'json',       //Data type - we will use JSON for almost everything 
            url: '/query4',   //The server endpoint we are connecting to
            success: function (data) {
                var heading = document.getElementById("queryHeading");
                heading.innerHTML = "Individuals With Familes of Size 5 or Greater";
                var table = document.getElementById("queryTable");
                var x = table.rows.length;
                for (j = x - 1; j > 0; --j) {
                    table.deleteRow(j);
                }
                table.deleteTHead();
                var header = table.createTHead();
                var row = header.insertRow(0);
                var cell1 = row.insertCell(0);
                cell1.innerHTML = "ID";
                var cell2 = row.insertCell(1);
                cell2.innerHTML = "Given Name"
                var cell3 = row.insertCell(2);
                cell3.innerHTML = "Surname"
                var cell4 = row.insertCell(3);
                cell4.innerHTML = "Sex"
                var cell5 = row.insertCell(4);
                cell5.innerHTML = "Family Size"
                var cell6 = row.insertCell(5);
                cell6.innerHTML = "Source File ID"

                for (var i in data) {
                    
                    var row = table.insertRow(-1);
                    var cell1 = row.insertCell(0);
                    cell1.innerHTML = data[i].ind_id;
                    var cell2 = row.insertCell(1);
                    cell2.innerHTML = data[i].given_name;
                    var cell3 = row.insertCell(2);
                    cell3.innerHTML = data[i].surname;
                    var cell4 = row.insertCell(3);
                    cell4.innerHTML = data[i].sex;
                    var cell5 = row.insertCell(4);
                    cell5.innerHTML = data[i].fam_size;
                    var cell6 = row.insertCell(5);
                    cell6.innerHTML = data[i].source_file;
                }

                var status = document.getElementById("statusPanel");
                var p = document.createElement("p");
                var node = document.createTextNode("Query 4 Table displayed");
                p.appendChild(node);
                status.appendChild(p);
            },
            
            fail: function(error) {
                // Non-200 return, do something with error
                console.log(error);
            }
        });
    });

    $("#query5").click(function(e) {
        e.preventDefault();
        $.ajax({
            type: 'get',            //Request type
            dataType: 'json',       //Data type - we will use JSON for almost everything 
            url: '/query5',   //The server endpoint we are connecting to
            success: function (data) {
                var heading = document.getElementById("queryHeading");
                heading.innerHTML = "All Female Individuals";
                var table = document.getElementById("queryTable");
                var x = table.rows.length;
                for (j = x - 1; j > 0; --j) {
                    table.deleteRow(j);
                }
                table.deleteTHead();
                var header = table.createTHead();
                var row = header.insertRow(0);
                var cell1 = row.insertCell(0);
                cell1.innerHTML = "ID";
                var cell2 = row.insertCell(1);
                cell2.innerHTML = "Given Name"
                var cell3 = row.insertCell(2);
                cell3.innerHTML = "Surname"
                var cell4 = row.insertCell(3);
                cell4.innerHTML = "Sex"
                var cell5 = row.insertCell(4);
                cell5.innerHTML = "Family Size"
                var cell6 = row.insertCell(5);
                cell6.innerHTML = "Source File ID"

                for (var i in data) {
                    
                    var row = table.insertRow(-1);
                    var cell1 = row.insertCell(0);
                    cell1.innerHTML = data[i].ind_id;
                    var cell2 = row.insertCell(1);
                    cell2.innerHTML = data[i].given_name;
                    var cell3 = row.insertCell(2);
                    cell3.innerHTML = data[i].surname;
                    var cell4 = row.insertCell(3);
                    cell4.innerHTML = data[i].sex;
                    var cell5 = row.insertCell(4);
                    cell5.innerHTML = data[i].fam_size;
                    var cell6 = row.insertCell(5);
                    cell6.innerHTML = data[i].source_file;
                }
                var status = document.getElementById("statusPanel");
                var p = document.createElement("p");
                var node = document.createTextNode("Query 5 Table displayed");
                p.appendChild(node);
                status.appendChild(p);
            },
            
            fail: function(error) {
                // Non-200 return, do something with error
                console.log(error);
            }
        });
    });

    $("#help").click(function(e) {
        e.preventDefault();
        $.ajax({
            type: 'get',            //Request type
            dataType: 'json',       //Data type - we will use JSON for almost everything 
            url: '/helpInd',   //The server endpoint we are connecting to
            success: function (data) {

                var heading = document.getElementById("queryHeading");
                heading.innerHTML = "Table Descriptions";
                var table = document.getElementById("queryTable");
                var x = table.rows.length;
                for (j = x - 1; j > 0; --j) {
                    table.deleteRow(j);
                }
                table.deleteTHead();
                var header = table.createTHead();
                var row = header.insertRow(0);
                var cell1 = row.insertCell(0);
                cell1.innerHTML = "Field";
                var cell2 = row.insertCell(1);
                cell2.innerHTML = "Type"
                var cell3 = row.insertCell(2);
                cell3.innerHTML = "Null"
                var cell4 = row.insertCell(3);
                cell4.innerHTML = "Key"
                var cell5 = row.insertCell(4);
                cell5.innerHTML = "Default"
                var cell6 = row.insertCell(5);
                cell6.innerHTML = "Extra"

                var row1 = table.insertRow(-1);
                var cell = row1.insertCell(0);
                cell.innerHTML = "Individual Table";

                for (var i in data) {
                    
                    var row = table.insertRow(-1);
                    var cell1 = row.insertCell(0);
                    cell1.innerHTML = data[i].Field;
                    var cell2 = row.insertCell(1);
                    cell2.innerHTML = data[i].Type;
                    var cell3 = row.insertCell(2);
                    cell3.innerHTML = data[i].Null;
                    var cell4 = row.insertCell(3);
                    cell4.innerHTML = data[i].Key;
                    var cell5 = row.insertCell(4);
                    cell5.innerHTML = data[i].Default;
                    var cell6 = row.insertCell(5);
                    cell6.innerHTML = data[i].Extra;
                }
            },
            
            fail: function(error) {
                // Non-200 return, do something with error
                console.log(error);
            }
        });

        $.ajax({
            type: 'get',            //Request type
            dataType: 'json',       //Data type - we will use JSON for almost everything 
            url: '/helpFile',   //The server endpoint we are connecting to
            success: function (data) {

                var table = document.getElementById("queryTable");
                var row1 = table.insertRow(-1);
                var cell = row1.insertCell(0);
                cell.innerHTML = "File Table";

                for (var i in data) {
                    
                    var row = table.insertRow(-1);
                    var cell1 = row.insertCell(0);
                    cell1.innerHTML = data[i].Field;
                    var cell2 = row.insertCell(1);
                    cell2.innerHTML = data[i].Type;
                    var cell3 = row.insertCell(2);
                    cell3.innerHTML = data[i].Null;
                    var cell4 = row.insertCell(3);
                    cell4.innerHTML = data[i].Key;
                    var cell5 = row.insertCell(4);
                    cell5.innerHTML = data[i].Default;
                    var cell6 = row.insertCell(5);
                    cell6.innerHTML = data[i].Extra;
                }

                var status = document.getElementById("statusPanel");
                var p = document.createElement("p");
                var node = document.createTextNode("Help Table displayed");
                p.appendChild(node);
                status.appendChild(p);
            },
            
            fail: function(error) {
                // Non-200 return, do something with error
                console.log(error);
            }
        });
    });

    $("#searchSubmit").click(function(e) {
        e.preventDefault();
        var search = document.getElementById("searchInput").value;
        $.ajax({
            type: 'get',            //Request type
            dataType: 'json',       //Data type - we will use JSON for almost everything 
            url: '/find',
            data: {"search": search},  //The server endpoint we are connecting to
            success: function (data) {

                var heading = document.getElementById("queryHeading");
                heading.innerHTML = "User SQL Statement";
                var table = document.getElementById("queryTable");
                var x = table.rows.length;
                for (j = x - 1; j > 0; --j) {
                    table.deleteRow(j);
                }
                table.deleteTHead();

                if (data.error != null) {
                    var header = table.createTHead();
                    var row = header.insertRow(0);
                    var cell = row.insertCell(0);
                    cell.innerHTML = data.error;
                }

                else {
                    if (data[0].table == "INDIVIDUAL") {
                        
                        var header = table.createTHead();
                        var row = header.insertRow(0);
                        var cell1 = row.insertCell(0);
                        cell1.innerHTML = "ID";
                        var cell2 = row.insertCell(1);
                        cell2.innerHTML = "Given Name"
                        var cell3 = row.insertCell(2);
                        cell3.innerHTML = "Surname"
                        var cell4 = row.insertCell(3);
                        cell4.innerHTML = "Sex"
                        var cell5 = row.insertCell(4);
                        cell5.innerHTML = "Family Size"
                        var cell6 = row.insertCell(5);
                        cell6.innerHTML = "Source File ID"

                        for (var i in data) {
                            
                            var row = table.insertRow(-1);
                            var cell1 = row.insertCell(0);
                            cell1.innerHTML = data[i].ind_id;
                            var cell2 = row.insertCell(1);
                            cell2.innerHTML = data[i].given_name;
                            var cell3 = row.insertCell(2);
                            cell3.innerHTML = data[i].surname;
                            var cell4 = row.insertCell(3);
                            cell4.innerHTML = data[i].sex;
                            var cell5 = row.insertCell(4);
                            cell5.innerHTML = data[i].fam_size;
                            var cell6 = row.insertCell(5);
                            cell6.innerHTML = data[i].source_file;
                        }
                    }

                    else {
                        var header = table.createTHead();
                        var row = header.insertRow(0);
                        var cell1 = row.insertCell(0);
                        cell1.innerHTML = "ID";
                        var cell2 = row.insertCell(1);
                        cell2.innerHTML = "File Name"
                        var cell3 = row.insertCell(2);
                        cell3.innerHTML = "Source"
                        var cell4 = row.insertCell(3);
                        cell4.innerHTML = "Version"
                        var cell5 = row.insertCell(4);
                        cell5.innerHTML = "Encoding"
                        var cell6 = row.insertCell(5);
                        cell6.innerHTML = "Submitter Name"
                        var cell7 = row.insertCell(6);
                        cell7.innerHTML = "Submitter Address"
                        var cell8 = row.insertCell(7);
                        cell8.innerHTML = "Number of Individuals"
                        var cell9 = row.insertCell(8);
                        cell9.innerHTML = "Number of Families"

                        for (var i in data) {
                            var row = table.insertRow(-1);
                            var cell1 = row.insertCell(0);
                            cell1.innerHTML = data[i].file_id;
                            var cell2 = row.insertCell(1);
                            cell2.innerHTML = data[i].file_Name;
                            var cell3 = row.insertCell(2);
                            cell3.innerHTML = data[i].source
                            var cell4 = row.insertCell(3);
                            cell4.innerHTML = data[i].version
                            var cell5 = row.insertCell(3);
                            cell5.innerHTML = data[i].encoding;
                            var cell6 = row.insertCell(4);
                            cell6.innerHTML = data[i].sub_name;
                            var cell7 = row.insertCell(5);
                            cell7.innerHTML = data[i].sub_addr;
                            var cell8 = row.insertCell(6);
                            cell8.innerHTML = data[i].num_individuals;
                            var cell9 = row.insertCell(7);
                            cell9.innerHTML = data[i].num_families;
                        }
                    }
                }
                var status = document.getElementById("statusPanel");
                var p = document.createElement("p");
                var node = document.createTextNode("Table displayed");
                p.appendChild(node);
                status.appendChild(p);
            },
            
            fail: function(error) {
                // Non-200 return, do something with error
                console.log(error);
            }
        });
    });   

    // Event listener form replacement example, building a Single-Page-App, no redirects if possible
    $('#someform').submit(function(e){
        e.preventDefault();
        $.ajax({});
    });
});







