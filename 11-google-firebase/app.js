
const config = require('./config');
const path = require('path');

var admin = require("firebase-admin");

// Note that you must generate a service account .json file and store it in the directory with config.json. You
// also must specify the filename in FIREBASE_CERT_FILE in config.json.
var serviceAccount = require(path.join(__dirname, config.get('FIREBASE_CERT_FILE')));

// Make sure you set FIREBASE_DATABASE in config.json. Note: It's different than the Google cloud project name!
admin.initializeApp({
	credential: admin.credential.cert(serviceAccount),
	databaseURL: "https://" + config.get('FIREBASE_DATABASE') + ".firebaseio.com"
});

var db = admin.database();
var dbRef = db.ref(config.get('FIREBASE_PARENT'));

var Particle = require('particle-api-js');
var particle = new Particle();

// console.log("deviceFilter=" + config.get('DEVICE_FILTER'));
// console.log("authToken=" + config.get('AUTH_TOKEN'));

// "sse-examples-01" is the Particle event name to filter on. It's a prefix, so event beginning with
// this name will be stored in the database.
particle.getEventStream({ deviceId:config.get('DEVICE_FILTER'), auth:config.get('AUTH_TOKEN'), name:'sse-examples-01' }).then(
		function(stream) {
			stream.on('event', function(event) {
				console.log("event: ", event);
				storeEvent(event);
			});
		},
		function(err) {
			console.log("Failed to getEventStream: ", err);
		});

function storeEvent(event) {

	var dbChild = dbRef.child(config.get('FIREBASE_CHILD'));
	
	var data = JSON.parse(event.data);

    // You can uncomment some of the other things if you want to store them in the database
    var obj = {
    	coreid: event.coreid,
    	published_at: event.published_at
    }

    // Copy the data in message.data, the Particle event data, as top-level 
    // elements in obj. This breaks the data out into separate columns.
    for (var prop in data) {
        if (data.hasOwnProperty(prop)) {
            obj[prop] = data[prop];
        }
    }
   
    dbChild.push().set(obj);
};