
const config = require('./config');
const path = require('path');

var admin = require("firebase-admin");

var serviceAccount = require(path.join(__dirname, config.get('FIREBASE_CERT_FILE')));

admin.initializeApp({
	credential: admin.credential.cert(serviceAccount),
	databaseURL: "https://" + config.get('FIREBASE_DATABASE') + ".firebaseio.com"
});

var db = admin.database();
var dbRef = db.ref(config.get('FIREBASE_PARENT'));

const dbChildName = "sharedValue";
const particleValueChangeEventName = "sse-examples-12-change";
const particleValueSetEventName = "sse-examples-12-set";

var Particle = require('particle-api-js');
var particle = new Particle();

//console.log("deviceFilter=" + config.get('DEVICE_FILTER'));
//console.log("authToken=" + config.get('AUTH_TOKEN'));

dbRef.on("child_changed", function(snapshot) {
	var value = snapshot.val();
	console.log("child_changed", value);

	particle.publishEvent({name:particleValueChangeEventName, data:JSON.stringify(value), isPrivate:true, auth:config.get('AUTH_TOKEN')}).then(
			function(data) {
				if (data.body.ok) {
					console.log("event published successfully");
				}
			},
			function(err) {
				console.log("error publishing", err);
			}
	);
});

particle.getEventStream({ deviceId:config.get('DEVICE_FILTER'), auth:config.get('AUTH_TOKEN'), name:particleValueSetEventName }).then(
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

	var dbChild = dbRef.child(dbChildName);

	var data = JSON.parse(event.data);

	// You can uncomment some of the other things if you want to store them in the database
	var obj = {
			coreid: event.coreid
			// published_at: event.published_at
	}

	// Copy the data in message.data, the Particle event data, as top-level 
	// elements in obj. This breaks the data out into separate columns.
	for (var prop in data) {
		if (data.hasOwnProperty(prop)) {
			obj[prop] = data[prop];
		}
	}

	// Use set(), not push().set() since we don't want to create new rows, we want to keep changing the single shared value
	dbChild.set(obj);
};