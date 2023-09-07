/*******************************************************************************
 * 2023. remi.leveque(α)pm.me
 * As this code is a derivative work from
 * https://github.com/eclipse/paho.mqtt.c/blob/master/src/samples/MQTTClient_publish_async.c
 * it is made available under the same license, which is:
 * 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v2.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    https://www.eclipse.org/legal/epl-2.0/
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 *******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <MQTTClient.h>

#include <unistd.h>

#include "paho.h"

#define QOS         1
#define TIMEOUT     10000L // 10'000 ms

char topic [100] = "default_topic";

MQTTClient                client;
MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
MQTTClient_message        pubmsg    = MQTTClient_message_initializer;
MQTTClient_deliveryToken  deliveredtoken;


void delivered(void *context, MQTTClient_deliveryToken dt)
{
	printf("Message with token value %d delivery confirmed\n", dt);
	deliveredtoken = dt;
}

int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
	// Should not be called as this is a pure publisher program.
	printf("Message arrived\n");
	printf("     topic: %s\n", topicName);
	printf("   message: %.*s\n", message->payloadlen, (char*)message->payload);
	MQTTClient_freeMessage(&message);
	MQTTClient_free(topicName);
	return 1;
}

void connlost(void *context, char *cause)
{
	printf("\nConnection lost\n");
	printf("     cause: %s\n", cause);

	// Let's try to reconnect
	while (1) {
	if ( connectMQTTClient() == MQTTCLIENT_SUCCESS )
		break;
	else
		usleep(5000000L); // Wait 5s
    }
}

int connectMQTTClient() {
	int rc;
	if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
		printf("Failed to connect, return code %d\n", rc);  
	return rc;
}

int initMQTTClient(const char* addr, const char* id, const char* user, const char* key) {

	sprintf(topic, "iot/dev/%s/data", id);

	int rc;

	if ((rc = MQTTClient_create(&client, addr, id,
	  MQTTCLIENT_PERSISTENCE_NONE, NULL)) != MQTTCLIENT_SUCCESS)
	{
		printf("Failed to create client, return code %d\n", rc);
		return EXIT_FAILURE;
	}

	if ((rc = MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered)) != MQTTCLIENT_SUCCESS)
	{
		printf("Failed to set callbacks, return code %d\n", rc);
		goto destroy_exit;
	}

	conn_opts.keepAliveInterval = 20;
	conn_opts.cleansession      = 1;
	conn_opts.username          = user;
	conn_opts.password          = key;

	if ((rc = connectMQTTClient() != MQTTCLIENT_SUCCESS) )
		goto destroy_exit;

	pubmsg.qos = QOS;
	pubmsg.retained = 1;  //New mqtt client subscribers will immediattly get the current count.

	return rc;

destroy_exit:
	MQTTClient_destroy(&client);
	return EXIT_FAILURE;
}

int sendToMQTT(char* const myPayload) {

	pubmsg.payload    = myPayload;
	pubmsg.payloadlen = (int)strlen(myPayload);
	deliveredtoken    = 0;

	MQTTClient_deliveryToken token;

	int rc;

	if ((rc = MQTTClient_publishMessage(client, topic, &pubmsg, &token)) != MQTTCLIENT_SUCCESS)
	{
		if (rc == MQTTCLIENT_DISCONNECTED)
			printf("Failed to publish message, MQTTCLIENT_DISCONNECTED \n");
		else
			printf("Failed to publish message, return code %d\n", rc);
        
		return EXIT_FAILURE;
	}

	printf("Waiting for publication of %s\n"
	       "on topic %s\n",
	       (char*)pubmsg.payload, topic);
	while (deliveredtoken != token)
	{
		usleep(10000L);
	}

	printf("\n");
	return EXIT_SUCCESS;
}