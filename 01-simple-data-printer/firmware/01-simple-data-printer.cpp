#include "Particle.h"

const unsigned long PUBLISH_PERIOD_MS = 10000;
unsigned long lastPublish = 0;
int counter = 0;

void setup() {
	Serial.begin(9600);
}

void loop() {
	if (millis() - lastPublish >= PUBLISH_PERIOD_MS) {
		lastPublish = millis();

		// Publish an event every 10 seconds. The event is JSON formatted and has a
		// value "counter" that increments each time.
		char data[256];
		snprintf(data, sizeof(data), "{\"counter\":%d}", counter++);

		Particle.publish("sse-examples-01", data, PRIVATE);
	}
}
