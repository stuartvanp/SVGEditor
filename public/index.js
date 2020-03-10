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
                    console.log(files[i]);
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
                    }
                })
            }
        }
        },
        fail: function(error){
            console.log("Failed to load filenames");
        }
    })

    // Event listener form example , we can use this instead explicitly listening for events
    // No redirects if possible
    $('#someform').submit(function(e){
        $('#blah').html("Form has data: "+$('#entryBox').val());
        e.preventDefault();
        //Pass data to the Ajax call, so it gets passed to the server
        $.ajax({
            //Create an object for connecting to another waypoint
        });
    });




});

function validateUpload(){
    let myfile = document.forms[uploadbttn][uploadFile].value;
        
    
    return false;

}
