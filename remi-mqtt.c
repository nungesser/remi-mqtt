#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <MQTTClient.h>

#define DRIVER_FILE "/proc/my_echo"

#define USERNAME    "device-iot"

#define QOS         1
#define TIMEOUT     10000L // 10'000 ms

char topic [100] = "default_device_id";

MQTTClient                client;
MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
MQTTClient_message        pubmsg    = MQTTClient_message_initializer;
MQTTClient_deliveryToken  token;


int initMQTTClient(const char* addr, const char* id, const char* key) {

	sprintf(topic, "iot/dev/%s/data", id);

	int rc;

	if ((rc = MQTTClient_create(&client, addr, id,
	  MQTTCLIENT_PERSISTENCE_NONE, NULL)) != MQTTCLIENT_SUCCESS)
	{
		printf("Failed to create client, return code %d\n", rc);
		return EXIT_FAILURE;
	}

	conn_opts.keepAliveInterval = 200;
	conn_opts.cleansession      = 1;
	conn_opts.username          = USERNAME;
	conn_opts.password          = key;
	if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
	{
		printf("Failed to connect, return code %d\n", rc);
		return EXIT_FAILURE;
	}

	pubmsg.qos = QOS;
	pubmsg.retained = 1;  //New mqtt client subscribers will immediattly get the current count.
        
	return (EXIT_SUCCESS);
}

int sendToMQTT(char* const myPayload) {

	pubmsg.payload    = myPayload;
	pubmsg.payloadlen = (int)strlen(myPayload);

	int rc;

	if ((rc = MQTTClient_publishMessage(client, topic, &pubmsg, &token)) != MQTTCLIENT_SUCCESS)
	{
		if (rc == MQTTCLIENT_DISCONNECTED)
			printf("Failed to publish message, MQTTCLIENT_DISCONNECTED \n");
		else
			printf("Failed to publish message, return code %d\n", rc);
        
		return EXIT_FAILURE;;
	}

	printf("Publication of %s\n"
	       "on topic %s\n",
	       (char*)pubmsg.payload, topic);
	rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
	if (rc == MQTTCLIENT_DISCONNECTED)
		printf("Failed to publish, MQTTCLIENT_DISCONNECTED \n\n");
	//printf("Message with delivery token %d delivered\n\n", token);
    
	printf("\n");
	return EXIT_SUCCESS;
}


int main(int argc, char* argv[]) {

	// Retrieve var env we need:
	const char* const BROKER_URI = getenv ("BROKER_URI");
	if (BROKER_URI == NULL)
		printf("BROKER_URI not in env\n");

	const char* const DEVICE_ID = getenv ("DEVICE_ID");
	if (DEVICE_ID == NULL)
		printf("DEVICE_ID  not in env\n");

	const char* const DEVICE_KEY = getenv ("DEVICE_KEY");
	if (DEVICE_KEY == NULL)
		printf("DEVICE_KEY not in env\n");

	if ( BROKER_URI == NULL || DEVICE_ID == NULL || DEVICE_KEY == NULL)
		exit(EXIT_FAILURE);

	// Init our MQTT client
	if ( initMQTTClient(BROKER_URI, DEVICE_ID, DEVICE_KEY) != EXIT_SUCCESS )
		exit(EXIT_FAILURE);


	FILE *fptr;

	// Open our driver in read mode
	fptr = fopen( DRIVER_FILE, "r");

	if(fptr == NULL) {
		printf("Not able to open the driver file: %s.\n", DRIVER_FILE);
		exit(EXIT_FAILURE);
	}

	char myLine[100];
	char myPayload[100];
	int  counter=0;

	// Read lines from the file forever
	while(fgets(myLine, 100, fptr)) {
		//printf("%s\n", myLine);
		// Assume that if we read "+1", it means one more person to add to counter.
		if ( strcmp (myLine, "+1\n") == 0 )
			counter++;
		else
			continue; // Nothing to update

		//sscanf(myLine, "%d", &counter);
		//printf("    %d\n", counter);

		// Format the data as json to be send to the mqtt broker 
		sprintf(myPayload, "{ \"type\": \"measure\",\"value\": { \"counter\": \"%d\" }}", counter);
		//printf("%s\n", myPayload);

		// Send it
		sendToMQTT(myPayload);
	}

	int rc;
	// Disconnect from mqtt brocker
	if ((rc = MQTTClient_disconnect(client, 10000)) != MQTTCLIENT_SUCCESS)
	printf("Failed to disconnect, return code %d\n", rc);
	MQTTClient_destroy(&client);

	// Close the file
	fclose(fptr);

	exit (EXIT_SUCCESS);
}
