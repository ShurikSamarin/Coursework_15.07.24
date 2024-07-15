#include "MQTTClient.h"
#include "MQTTClientPersistence.h"
#include "pubsub_opts.h"

#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include "time.h"

#define ADDRESS "192.168.88.129:1883"
#define CLIENTID "ExampleClientPub"
#define TOPIC "/node-red/temp"
#define PAYLOAD "50"
#define QOS 1
#define TIMEOUT 10000L


int main()
{
	bool isExit = 0;

	FILE *file;
	file = fopen("temp1.csv","w");
	
	MQTTClient client;
	MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
	
	int rc;

	MQTTClient_create(&client, ADDRESS, CLIENTID,
		MQTTCLIENT_PERSISTENCE_NONE, NULL);
	conn_opts.keepAliveInterval = 20;
	conn_opts.cleansession = 1;
	conn_opts.username = "IoT";
	conn_opts.password = "student1";
	
	if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
	{
		printf("Failed to connect, returm code %d\n", rc);
		exit(-1);
	}
	printf("Success in connect, return code %d\n", rc);
	rc = MQTTClient_subscribe(client, TOPIC, QOS);
	if (rc != MQTTCLIENT_SUCCESS && rc !=QOS)
	{
		fprintf(stderr, "Error %d subscribing to topic %s\n", rc, TOPIC);
		exit(-2);
	}
	printf("Success %d subscribing to topic %s\n", rc, TOPIC);
	printf("Waiting for the message\n");
	while(isExit!=1)
	{
		time_t mytime = time (NULL);
		struct tm *now = localtime(&mytime);
		
		char* topicName = TOPIC;
		int topicLen = strlen(PAYLOAD);
		MQTTClient_message* message = NULL;
		
		rc = MQTTClient_receive(client, &topicName, &topicLen, &message, 1000);
		if (rc == MQTTCLIENT_DISCONNECTED)
			rc = MQTTClient_connect(client, &conn_opts);
		else if (message)
		{
			printf("%s\n", (char*)message->payload);
			
			char str[255]={0};
			char time[20];
			char date[20];
			
			strftime(date,sizeof(str),"%D",now);
			strftime(time,sizeof(str),"%T",now);

			strcat(str,(char*)message->payload);
			printf("%s\n",str);
			printf("%s\n",time);
			printf("%s\n",date);
			fprintf(file, "Temp: %s Time: %s Date: %s\n", str, time,date);
			
			fflush(stdout);
			MQTTClient_freeMessage(&message);
			MQTTClient_free(topicName);
			printf("\n=> Press any key and Enter, # to stop connection\n");
			char c;
			while((c=getchar())!='\n')
				if(c=='#')
				{
					isExit=1;
				}
		}
	}
exit:
	fclose(file);
	MQTTClient_disconnect(client, 10000);
	MQTTClient_destroy(&client);
	return EXIT_SUCCESS;
}
