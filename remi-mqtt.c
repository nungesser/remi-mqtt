#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <MQTTClient.h>

#include "paho.h"

#define DRIVER_FILE "/proc/my_echo"

extern MQTTClient client;

int main(int argc, char* argv[]) {

	// Retrieve env var we need:
	const char* const BROKER_URI = getenv ("BROKER_URI");
	if (BROKER_URI == NULL)
		printf("BROKER_URI not in env\n");

	const char* const DEVICE_ID  = getenv ("DEVICE_ID");
	if (DEVICE_ID == NULL)
		printf("DEVICE_ID  not in env\n");

	const char* const USERNAME   = getenv ("USERNAME");
	if (USERNAME == NULL)
		printf("USERNAME not in env\n");

	const char* const DEVICE_KEY = getenv ("DEVICE_KEY");
	if (DEVICE_KEY == NULL)
		printf("DEVICE_KEY not in env\n");

	if ( BROKER_URI == NULL || DEVICE_ID == NULL || USERNAME == NULL || DEVICE_KEY == NULL)
		exit(EXIT_FAILURE);

	// Init our MQTT client
	if ( initMQTTClient(BROKER_URI, DEVICE_ID, USERNAME, DEVICE_KEY) != EXIT_SUCCESS )
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

	// reset remote counter value
	sendToMQTT("{ \"type\": \"measure\",\"value\": { \"counter\": \"0\" }}");

	// Read lines from the file forever
	while(fgets(myLine, 100, fptr)) {
		//printf("%s\n", myLine);
		// Assume that if we read "+1", it means one more to add to counter.
		if ( strcmp (myLine, "+1\n") == 0 )
			counter++;
		else
			continue; // Nothing to update

		//sscanf(myLine, "%d", &counter);
		//printf("    %d\n", counter);

		// Format the data as json to be send to the mqtt broker 
		sprintf(myPayload, "{ \"type\": \"measure\",\"value\": { \"counter\": \"%d\" }}", counter);

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
