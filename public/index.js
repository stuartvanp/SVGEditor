// Put all onload AJAX calls here, and event listeners
$(document).ready(function() {
    // On page-load AJAX Example
    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/someendpoint',   //The server endpoint we are connecting to
        data: {
            name1: "Value 1",
            name2: "Value 2"
        },
        success: function (data) {
            /*  Do something with returned object
                Note that what we get is an object, not a string, 
                so we do not need to parse it on the server.
                JavaScript really does handle JSONs seamlessly
            */
            $('#blah').html("On page load, received string '"+data.foo+"' from server");
            //We write the object to the console to show that the request was successful
            console.log(data); 

        },
        fail: function(error) {
            // Non-200 return, do something with error
            $('#blah').html("On page load, received error from server");
            console.log(error); 
        }
    });

    $.ajax({ 
        type: 'get',
        dataType: 'json',
        url:'myfirst',
        success:function(data) {
   
            console.log(data.foo);
        }
    })

    $.ajax({
        type: 'get',
        url: 'Allfiles',
        success:function(data){
            console.log(data);
            files = data;
            if (data.length > 0) {
                $('#LOG').html('<table border="1" class = "table">' +
                        '<thead id = "LOGROW" class="thead-dark">' +
                    '<TR>' + 
                        '<TH><center>Image <BR> (click to download)</center></TH>' + 
                        '<TH><center>File name<BR> (click to download)</center></TH>' +
                        '<TH><center>File size</center></TH>' +
                        '<TH><center>Number of Rectangles</center></TH> <TH><center>Number of Circles</center></TH>' +
                        '<TH><center>Number of Paths</center></TH>' + 
                        '<TH><center>Number of Groups</center></TH>' +
                    '</TR>' +
                    '</thead>' +
                    '</table>' );
               for (let i = 0; i < data.length; i++){
                    let filename= files[i];
                    $.ajax({
                        type: 'get',
                        url: '/getSVGJSON',
                        data: {file: filename},
                        success: function(data){
                            console.log("LOADING FILE " + files[i]);

                            let svgjson = JSON.parse(data);
                            $('#LOGROW').append ('<TR><TD style="text-align:center"><a href="' + files[i] + '" download> <img src = "uploads/' +
                             files[i] + '" width = 200> </a> </TD>' +
                            '<TD style="text-align:center"> <a href="' + files[i] + '" download>' + files[i] + '</a></TD>' + 
                            '<TD style="text-align:center">'+ svgjson.size +'KB</TD>' +
                            '<TD style="text-align:center">' + svgjson.numRect + '</TD>'+
                            '<TD style="text-align:center">' + svgjson.numCirc + '</TD>'+
                            '<TD style="text-align:center">' + svgjson.numPaths + '</TD>'+
                            '<TD style="text-align:center">' + svgjson.numGroups + '</TD>)</TR>');
                            $("#svgDropDown").append('<option value = "' + files[i] + '">' + files[i] + '</option>');
                            if (i == 0 ) {
                                setTable(files[i]);
                            }

                    }
                })
            }
        }
        },
        fail: function(error){
            console.log("Failed to load filenames");
        }
    });



    // Event listener form example , we can use this instead explicitly listening for events
    // No redirects if possible
    $('#someform').submit(function(e){
        $('#blah').html("Form has data: "+$('#entryBox').val());
        e.preventDefault();
        //Pass data to the Ajax call, so it gets passed to the server
        console.log("PORRRRRK");

        $.ajax({
            //Create an object for connecting to another waypoint
        });
    });

    $('#chooseSVG').change(function(e) {
        let selection = document.getElementById('svgDropDown');
        let filename = selection.value;
        setTable(filename);
        
    });

    $('#TDform').submit(function(e){
        e.preventDefault();
        let title = document.getElementById('titleIn').value;
        let desc = document.getElementById('descIn').value;
        let filename = document.getElementById('titleIn').name;


        if (title.length > 255) {
            console.log("Title too long");
            alert("Title must be 255 characters or less!");
            return;
        }
        if (desc.length > 255) {
            console.log("Desc too long");
            alert("Description must be 255 characters or less");
            return;
        }
        $.ajax({
            type: 'get',
            url: '/updateTitle',
            data: {ttl: title,
            dsc: desc,
            file: filename},
            success: function(data){
                console.log(data.stat);
                if (data.stat == 1) {
                    location.reload();
                    console.log("HELEO")
                }
                else {
                    alert("failed to update title/desc");
                }  
            },
            fail: function(data){
                console.log("FAILED TO UPDATE TITLE/DESC")
            }
        });
    });
    $("#addRectF").submit(function(e){
        e.preventDefault();
        console.log("ATTEMPTING TO ADD RECTANGLE");
        e.preventDefault();

    })
    $("#addCircF").submit(function(e){
        e.preventDefault();
        console.log("ATTEMPTING TO ADD CIRCLE");

    })
    
});

function setTable(filename){
    console.log(filename);
    $("#imageDisplay").html('<TD colspan="5" style="text-align:center"><img src = "'+filename+ '"width = 800></TD>');
        //gets tittle and description
        $.ajax({
        type: 'get',
        url: '/titledesc',
        data: {file: filename},
        success: function(data){                
            $("#title").html(data.Title);
            $("#desc").html(data.Desc);
            $("#titlediv").html('<label for="titleIn">Title: </label>' +
            '<input type = "text" id = "titleIn" name = "'+filename +'" value = "' + data.Title+'">');
            $("#descdiv").html('<label for="descIn">Description: </label>' + 
            "<input type = 'text' id = 'descIn' name = 'descIn' value = '"+ data.Desc+"'>");
            

            },
            fail: function(error){
                console.log("Failed to load title and desc");
            }
        });
        $('#tablebody').html('');
        //gets rectangles
        $.ajax({
            type: 'get',
            url: '/rects',
            data: {file: filename},
            success:function(data) {
                
                let rects = JSON.parse(data);
                for (let i = 0; i < rects.length; i++) {
                    $("#tablebody").append('<TR><TD style="text-align:center" id ="rect' + i +'">RECTANGLE ' + (i + 1) +  
                    '</TD><TD style="text-align:center">Upper left corner: x = ' + rects[i].x + rects[i].units + ', y =' +
                    rects[i].y + rects[i].units + '<BR>Width: ' + rects[i].w + rects[i].units + ' Height: '
                    + rects[i].h + rects[i].units + 
                    '</TD><TD style="text-align:center">Other Attributes: ' + rects[i].numAttr + 
                    '</TD><TD><button onClick = "showAttr(this.id, this.name)" id = "rect' + i + '" name = "' +filename + 
                    '">Show Attributes</button></TD>/TR>');
                }
            }
        });
        //gets circles
        $.ajax({
            type: 'get',
            url: '/circs',
            data: {file: filename},
            success:function(data) {
                let circs = JSON.parse(data);
                for (let i = 0; i < circs.length; i++) {
                    $("#tablebody").append('<TR><TD style="text-align:center"> CIRCLE ' + (i + 1) +  
                    '</TD><TD style="text-align:center">Centre: x = ' + circs[i].cx + circs[i].units + ', y =' +
                    circs[i].cy + circs[i].units + ' Radius: ' + circs[i].r + circs[i].units +
                    '</TD><TD style="text-align:center">Other Attributes: ' + circs[i].numAttr + 
                    '</TD><TD><button onClick = "showAttr(this.id, this.name)" id = "circ' + i  + '" name = "' +filename  + 
                    '">Show Attributes</button></TD</TR>');
                }
            }
        });

        //gets paths
        $.ajax ({
            type: 'get',
            url: '/paths',
            data: {file:filename},
            success: function(data){
                let paths = JSON.parse(data);
                for (let i = 0; i < paths.length; i++) {
                    $("#tablebody").append('<TR><TD style="text-align:center">PATH ' + (i + 1) +  
                    '</TD><TD style="text-align:center">' + paths[i].d + 
                    '</TD><TD style="text-align:center">Other Attributes: ' + paths[i].numAttr + 
                    '</TD><TD><button onClick = "showAttr(this.id, this.name)" id = "path' + i  + '" name = "' +filename  + 
                    '">Show Attributes</button></TD></TR>');
                }
            }
        });
        //gets groups
        $.ajax ({
            type: 'get',
            url: '/groups',
            data: {file:filename},
            success: function(data) {
                let groups =JSON.parse(data);
                for (let i = 0; i < groups.length; i++) {
                    $("#tablebody").append('<TR><TD style="text-align:center">GROUP ' + (i + 1) +  
                    '</TD><TD style="text-align:center">' + groups[i].children + ' Children' + 
                    '</TD><TD style="text-align:center">Other Attributes: ' + groups[i].numAttr + 
                    '</TD><TD><button onClick = "showAttr(this.id, this.name)" id = "group' + i  + '" name = "' +filename  + 
                    '">Show Attributes</button></TD></TR>');
                }
            }
        });

}


function showAttr(id, name){
    let index = id.match(/\d+/g).map(Number)[0];
    let type = id[0];
    $.ajax({
        type: 'get',
        url: '/getAtt',
        data: {file: name,
        elem: type,
        num: index},
        success: function(data){
            attribs = JSON.parse(data);
            $("#otherAtts").html('OTHER ATTRIBUTES<BR>');
            for (let i = 0; i < attribs.length; i++) {
                $("#otherAtts").append(attribs[i].name + ": " + attribs[i].value + "<BR>");
            }
        }
    });
}

function uploadF(uploadFile){
    console.log("YEET");

}

function editTD(title, desc){
    console.log(title, desc);
    if (title.length > 255) {
        console.log("TOO LONG TITLE");
    }

}
