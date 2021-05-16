function sendPost(){
    var xhr = new XMLHttpRequest();
    //send request to server, change 127.0.0.1:12345/ to url of the server
    let url = "127.0.0.1:12345"
    console.log("POST");
    xhr.open("POST", url);

    //set header and value
    xhr.setRequestHeader('Content-Type', 'text/txt');

    console.log(xhr.send("test text"));
}

function badReq(){
    var xhr = new XMLHttpRequest();
    //send request to server, change 127.0.0.1:12345/ to url of the server
    let url = "127.0.0.1:12345"
    console.log("get");
    xhr.open("GET", url, true);

    //set header and value
    xhr.setRequestHeader('Content-Type', 'text/txt');

    xhr.send(JSON.stringify("test text"));
}