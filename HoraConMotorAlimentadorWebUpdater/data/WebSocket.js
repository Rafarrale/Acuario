var rainbowEnable = false;
//var connection = new WebSocket('ws://'+location.hostname+':81/', ['arduino']);
var connection = new WebSocket('ws://'+location.hostname+':81/', ['arduino']);
var cont = 0;

output = document.getElementById("chatbox");

connection.onopen = function () {
    connection.send('Connect ' + new Date());
};
connection.onerror = function (error) {
    console.log('WebSocket Error ', error);
};
connection.onmessage = function (event) {  
   console.log('Server: ', event.data);  
   writeToScreen(event.data);
   muestraTemp(event.data);

    
};
connection.onclose = function(){
    console.log('WebSocket connection closed');
};

function sendRGB() {
    var r = document.getElementById('r').value**2/1023;
    var g = document.getElementById('g').value**2/1023;
    var b = document.getElementById('b').value**2/1023;
    
    var rgb = r << 20 | g << 10 | b;
    var rgbstr = '#'+ rgb.toString(16);    
    console.log('RGB: ' + rgbstr); 
    connection.send(rgbstr);
}

function writeToScreen(message) {
    var str = message;
    var n = str.search("activacion");
    if(n == 5){
        var node = document.createElement("LI");
        var textnode = document.createTextNode(message);
        node.appendChild(textnode);
        document.getElementById("chatbox").appendChild(node);
    }
}

function rainbowEffect(){
    rainbowEnable = ! rainbowEnable;
    if(rainbowEnable){
        connection.send("R");
        document.getElementById('rainbow').style.backgroundColor = '#00878F';
        document.getElementById('r').className = 'disabled';
        document.getElementById('g').className = 'disabled';
        document.getElementById('b').className = 'disabled';
        document.getElementById('r').disabled = true;
        document.getElementById('g').disabled = true;
        document.getElementById('b').disabled = true;
        console.log('R');
    } else {
        connection.send("N");
        document.getElementById('rainbow').style.backgroundColor = '#999';
        document.getElementById('r').className = 'enabled';
        document.getElementById('g').className = 'enabled';
        document.getElementById('b').className = 'enabled';
        document.getElementById('r').disabled = false;
        document.getElementById('g').disabled = false;
        document.getElementById('b').disabled = false;
        sendRGB();
        console.log('N');
    }  
}

function contador(){
	connection.send("T");
}

function muestraTemp(message){
	var str = message;
    var n = str.search("temperatura");
	if(n == 0){
		var res = str.split(" ");
		var newItem = document.createElement("LI");       // Create a <li> node
		var textnode = document.createTextNode(res[1]);  // Create a text node
		newItem.appendChild(textnode);                    // Append the text to <li>

		var list = document.getElementById("chatboxTemp");    // Get the <ul> element to insert a new node
		list.insertBefore(newItem, list.childNodes[0]);  // Insert <li> before the first child of 
	}
}

setInterval('contador()',5000);



