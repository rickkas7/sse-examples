#include "Particle.h"

#include "GuaranteedDelivery.h"

STARTUP(System.enableFeature(FEATURE_RETAINED_MEMORY));

SYSTEM_THREAD(ENABLED);

retained char savedData[1024];
GuaranteedDelivery guaranteedDelivery(savedData, sizeof(savedData), "ts", "ssetest13");

unsigned long nextPublish = 0;

void setup() {
	Serial.begin(9600);
	guaranteedDelivery.setup();
}

void loop() {
	guaranteedDelivery.loop();

	unsigned long now = Time.now();

	if (nextPublish == 0) {
		if (Time.isValid()) {
			// Set the next time to publish once we have a valid real-time clock value

			// For this test, we publish once per minute
			int second = Time.second(now);
			if (second > 0) {
				nextPublish = now + (60 - second);
			}
			else {
				nextPublish = now;
			}
		}
	}

	if (nextPublish != 0 && nextPublish <= now) {
		JsonWriter jw = guaranteedDelivery.getJsonWriter();

		{
			JsonWriterAutoObject obj(&jw);

			jw.insertKeyValue("ts", nextPublish);
			jw.insertKeyValue("a", rand());
		}

		guaranteedDelivery.publishJson();

		nextPublish += 60;
	}
}
