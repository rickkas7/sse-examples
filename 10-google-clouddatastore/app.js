
const config = require('./config');

const Datastore = require('@google-cloud/datastore');
const datastore = Datastore({
	projectId: config.get('GCLOUD_PROJECT')
});

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
	// Each database row has a unique key, and it's generated here
    var key = datastore.key(config.get('DATA_STORE_ENTITY_KIND'));

    // The Particle event data is assumed to be JSON. All of the fields will be stored in the cloud
    // datastore automatically. If you are using the test firmware from 01-simple-data-printer, this will
    // only be the counter value, but you can add other and they automatically get stored in the database!
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
   
    datastore.save({
        key: key,
        data: obj
    }, function(err) {
		if(err) {
			console.log('There was an error storing the event', err);
		}
		console.log('stored in datastore', obj);
    });

};