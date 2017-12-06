
var config = require('./config.json'); 

var Particle = require('particle-api-js');
var particle = new Particle();

var mysql = require('mysql');

// Make sure you update the required MySQL server settings in config.json!
var con = mysql.createConnection({
	host:config.mysql_host,
	user:config.mysql_user,
	password:config.mysql_password,
	database:config.mysql_database});


con.connect(function(err) {
	if (err) throw err;
	
	console.log("Connected to database");

	// "sse-examples-01" in the Particle event to filter on. It's a prefix, so all events beginning with this
	// name are stored in the database.
	particle.getEventStream({ deviceId:config.deviceFilter, auth:config.authToken, name:'sse-examples-01' }).then(
			function(stream) {
				stream.on('event', function(event) {
					console.log("event: ", event);
					
					// This assumes that the data in the event is valid JSON!
					var data = JSON.parse(event.data);
					
					// The "counter" value in the JSON is stored in the counter column in the SQL database.
					
					// create table example02 (id INT NOT NULL AUTO_INCREMENT, device VARCHAR(34), ts DATETIME, counter INT, UNIQUE INDEX(id));
					var sql = "INSERT INTO example02 (device, ts, counter) VALUES ('" + event.coreid + "', NOW(), " + data.counter + ")";
					  con.query(sql, function (err, result) {
					    if (err) throw err;
					    console.log("inserted: " + sql);
					  });
				});
			},
			function(err) {
				console.log("Failed to getEventStream: ", err);
			});

});


