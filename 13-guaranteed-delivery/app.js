
const config = require('./config');

const Datastore = require('@google-cloud/datastore');
const datastore = Datastore({
	projectId: config.get('GCLOUD_PROJECT')
});

var Particle = require('particle-api-js');
var particle = new Particle();

// console.log("deviceFilter=" + config.get('DEVICE_FILTER'));
// console.log("authToken=" + config.get('AUTH_TOKEN'));

particle.getEventStream({ deviceId:config.get('DEVICE_FILTER'), auth:config.get('AUTH_TOKEN'), name:'ssetest13' }).then(
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
    var key = datastore.key(config.get('DATA_STORE_ENTITY_KIND'));

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
       
    datastore.save({
        key: key,
        data: obj
    }, function(err) {
		if (err) {
			console.log('There was an error storing the event', err);
			return;
		}
		
		var arg = data['ts'].toString();

		console.log('stored in datastore ts=' + arg, obj);
	
		
		particle.callFunction({deviceId:event.coreid, name:'deliveryConf', argument:arg, auth:config.get('AUTH_TOKEN')}).then(
				function(data) {
					console.log("function called successfully");
				},
				function(err) {
					console.log("function called failed", err);					
				}
				);
    });

};