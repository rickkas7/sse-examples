#include "Particle.h"

#include "SparkJson.h"

void handleButtonClick(system_event_t event, int param);
void handleEvent(const char *event, const char *data);

StaticJsonBuffer<256> jsonBuffer;
char buttonClicked = false;

void setup() {
	Serial.begin(9600);

	Particle.subscribe("sse-examples-12-change", handleEvent, MY_DEVICES);
	System.on(button_click, handleButtonClick);
}

void loop() {
	if (buttonClicked) {
		buttonClicked = false;

		char data[256];
		snprintf(data, sizeof(data), "{\"value\":%d}", rand());

		Particle.publish("sse-examples-12-set", data, PRIVATE);

		Serial.printlnf("sending: %s", data);
	}
}


void handleButtonClick(system_event_t event, int param) {
	buttonClicked = true;
}

void handleEvent(const char *event, const char *data) {
	char *mutableData = strdup(data);

	JsonObject& root = jsonBuffer.parseObject(mutableData);

	free(mutableData);

	if (root.success()) {
		const char *coreid = root["coreid"];
		int value = root["value"];

		Serial.printlnf("value=%d coreid=%s", value, coreid);
	}
}
