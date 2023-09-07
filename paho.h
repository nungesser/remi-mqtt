#pragma once

// paho callbacks
void delivered     (void *context, MQTTClient_deliveryToken dt);
int  msgarrvd      (void *context, char *topicName, int topicLen, MQTTClient_message *message);
void connlost      (void *context, char *cause);

// functions
int  connectMQTTClient();
int  initMQTTClient(const char* addr, const char* id, const char* user, const char* key);
int  sendToMQTT    (char* const myPayload);
