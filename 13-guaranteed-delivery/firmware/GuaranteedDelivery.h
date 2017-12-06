#ifndef __GUARANTEEDDELIVERY_H
#define __GUARANTEEDDELIVERY_H

#include "Particle.h"
#include "JsonParserGeneratorRK.h"

class GuaranteedDelivery {
public:
	GuaranteedDelivery(char *retainedBuf, size_t retainedBufLen, const char * const indexKeyName, const char * const eventName);
	virtual ~GuaranteedDelivery();

	void setup();
	void loop();

	JsonWriter &getJsonWriter();
	bool publishJson();

	bool publish(const char *data);

	void checkPublish();

protected:
	void removeOldest();
	void removeById(int id);

	int functionHandler(String command);

	char *retainedBuf;
	size_t retainedBufLen;
	const char * const idKeyName;
	const char * const eventName;

	unsigned long lastCheck;
	unsigned long nextCheckPeriod;
	size_t publishOffset;
};

#endif /* __GUARANTEEDDELIVERY_H */
