
var config = require('./config.json'); 

var Particle = require('particle-api-js');
var particle = new Particle();

//console.log("deviceFilter=" + config.deviceFilter);
//console.log("authToken=" + config.authToken);

// "sse-examples-01" is the event name to trigger on. It's a prefix, so all events beginning with this will
// be printed out!

particle.getEventStream({ deviceId:config.deviceFilter, auth:config.authToken, name:'sse-examples-01' }).then(
		function(stream) {
			stream.on('event', function(event) {
				console.log("event: ", event);
			});
		},
		function(err) {
			console.log("Failed to getEventStream: ", err);
		});