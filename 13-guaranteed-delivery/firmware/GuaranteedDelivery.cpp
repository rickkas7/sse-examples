#include "GuaranteedDelivery.h"

#include "JsonParserGeneratorRK.h"

static const uint32_t RETAINED_BUF_HEADER_MAGIC = 0x801d3cf2;
static const uint16_t RETAINED_BUF_VERSION = 1;
typedef struct {
	uint32_t magic;
	uint16_t version;
	uint16_t length;
} RetainedBufHeader;

static const char * const GUARANTEED_DELIVERY_FUNCTION_NAME = "deliveryConf"; // max 12 characters

// Retry transmission after 30 seconds
static unsigned long PUBLISH_CHECK_PERIOD_MS = 30000;

// Check this often when offline
static unsigned long OFFLINE_PUBLISH_CHECK_PERIOD_MS = 5000;

// Set up a static JSON parser and generator using a single 256 byte buffer, since we never both parse and
// generate at the same time
static char internalBuffer[256];
static JsonParserGeneratorRK::jsmntok_t jsonTokens[20];
static JsonParser jsonParser(internalBuffer, sizeof(internalBuffer), jsonTokens, sizeof(jsonTokens)/sizeof(jsonTokens[0]));
static JsonWriter jsonWriter(internalBuffer, sizeof(internalBuffer));

GuaranteedDelivery::GuaranteedDelivery(char *retainedBuf, size_t retainedBufLen, const char * const idKeyName, const char * const eventName) :
	retainedBuf(retainedBuf), retainedBufLen(retainedBufLen), idKeyName(idKeyName), eventName(eventName),
	lastCheck(0), nextCheckPeriod(OFFLINE_PUBLISH_CHECK_PERIOD_MS), publishOffset(0) {

	RetainedBufHeader *hdr = (RetainedBufHeader *)retainedBuf;
	if (hdr->magic != RETAINED_BUF_HEADER_MAGIC || hdr->version != RETAINED_BUF_VERSION) {
		hdr->magic = RETAINED_BUF_HEADER_MAGIC;
		hdr->version = RETAINED_BUF_VERSION;
		hdr->length = 0;
	}
}

GuaranteedDelivery::~GuaranteedDelivery() {

}

void GuaranteedDelivery::setup() {
	Particle.function(GUARANTEED_DELIVERY_FUNCTION_NAME, &GuaranteedDelivery::functionHandler, this);
}

void GuaranteedDelivery::loop() {
	if (millis() - lastCheck >= nextCheckPeriod) {
		lastCheck = millis();
		checkPublish();
	}
}

JsonWriter &GuaranteedDelivery::getJsonWriter() {
	jsonWriter.clear();
	memset(internalBuffer, 0, sizeof(internalBuffer));

	return jsonWriter;
}

bool GuaranteedDelivery::publishJson() {

	return publish(jsonWriter.getBuffer());
}

bool GuaranteedDelivery::publish(const char *data) {
	RetainedBufHeader *hdr = (RetainedBufHeader *)retainedBuf;
	size_t len = strlen(data);

	// If the buffer is full, remove the oldest
	while ((hdr->length + len + 1) > (retainedBufLen - sizeof(RetainedBufHeader))) {
		removeOldest();
	}

	// Add to end of the buffer
	strcpy(&retainedBuf[sizeof(RetainedBufHeader) + hdr->length], data);
	hdr->length += len + 1;


	// Transmit if we can
	if (Particle.connected()) {
		Serial.printlnf("publishing %s new length=%d", data, hdr->length);
		Particle.publish(eventName, data, PRIVATE);
		lastCheck = millis();
	}
	else {
		Serial.printlnf("queueing publish %s new length=%d", data, hdr->length);
	}
	return true;
}

void GuaranteedDelivery::checkPublish() {
	if (Particle.connected()) {
		RetainedBufHeader *hdr = (RetainedBufHeader *)retainedBuf;
		if (hdr->length > 0) {
			// There are old events to publish
			// We are connected to the cloud - try publishing
			char *data = &retainedBuf[sizeof(RetainedBufHeader) + publishOffset];
			Serial.printlnf("publishing %s", data);
			Particle.publish(eventName, data, PRIVATE);
			publishOffset += strlen(data) + 1;

			if (publishOffset < hdr->length) {
				// We have more to publish. Check again after the 1 second publish window is up.
				nextCheckPeriod = 1010;
			}
			else {
				// At end of the list, check again in 30 seconds
				nextCheckPeriod = PUBLISH_CHECK_PERIOD_MS;
				publishOffset = 0;
			}
		}
	}
	else {
		// Check every 5 seconds when offline
		nextCheckPeriod = OFFLINE_PUBLISH_CHECK_PERIOD_MS;
	}
}

int GuaranteedDelivery::functionHandler(String command) {
	// The guaranteed delivery function is called with the Id to confirm
	int id = atoi(command.c_str());

	Serial.printlnf("functionHandler %d", id);

	removeById(id);

	return 0;
}

void GuaranteedDelivery::removeOldest() {
	RetainedBufHeader *hdr = (RetainedBufHeader *)retainedBuf;
	if (hdr->length > 0) {
		char *start = &retainedBuf[sizeof(RetainedBufHeader)];
		size_t len = strlen(start);

		char *next = &start[len + 1];

		hdr->length -= (len + 1);
		if (hdr->length > 0) {
			memmove(next, start, hdr->length);
		}
		Serial.printlnf("removeOldeset item len=%d new length=%d %s", len, hdr->length, start);
	}
	else {
		Serial.printlnf("removeOldeset nothing to remove");
	}
	// When removing entries from the list, reset the publish offset as it could be invalid now
	publishOffset = 0;
}

void GuaranteedDelivery::removeById(int id) {
	RetainedBufHeader *hdr = (RetainedBufHeader *)retainedBuf;
	if (hdr->length > 0) {
		char *start = &retainedBuf[sizeof(RetainedBufHeader)];

		while(start < &retainedBuf[sizeof(RetainedBufHeader) + hdr->length]) {
			size_t len = strlen(start);

			jsonParser.clear();
			jsonParser.addString(start);
			jsonParser.parse();

			int intValue = -1;
			if (!jsonParser.getOuterValueByKey(idKeyName, intValue) || intValue == id) {
				// Data is unparsable or found so remove it
				Serial.printlnf("removing id=%d %s", intValue, start);

				char *next = &start[len + 1];

				if ((&retainedBuf[sizeof(RetainedBufHeader) + hdr->length] - next) > 0) {
					memmove(start, next, &retainedBuf[sizeof(RetainedBufHeader) + hdr->length] - next);
				}
				hdr->length -= (len + 1);
			}
			else {
				// Check next item
				start += len + 1;
			}
		}
	}
	// When removing entries from the list, reset the publish offset as it could be invalid now
	publishOffset = 0;
}


