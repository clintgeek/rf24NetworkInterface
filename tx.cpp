#include <RF24/RF24.h>
#include <RF24Network/RF24Network.h>
#include "mosquittopp.h"
#include "mosquitto.h"
#include <iostream>
#include <string>
#include <sstream>
#include <ctime>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Constants that identify nodes
const uint16_t pi_node = 00; // this node
const uint16_t action_node1 = 01; // home/master/deskLamp
const uint16_t action_node2 = 02; // home/master/tvLight
const uint16_t sensor_node3 = 03; // home/master/temp

// channels to subscribe to
const char* action_channel1 = "home/master/deskLamp";
const char* action_channel2 = "home/master/tvLight";

// channels to publish to
const char* sensor_channel3a = "home/master/temp";
const char* sensor_channel3b = "home/master/humidity";

// Time between checking for packets (in ms)
const unsigned long interval = 1000;

// configure radio
RF24 radio(RPI_V2_GPIO_P1_15, BCM2835_SPI_CS0, BCM2835_SPI_SPEED_8MHZ);
RF24Network network(radio);

// payload to send to action nodes
struct message_action {
  unsigned char mode;
  unsigned char param1;
  unsigned char param2;
  unsigned char param3;
};

// payload sent from sensor nodes
struct message_sensor {
  uint16_t sensor;
  uint16_t param1;
  uint16_t param2;
  uint16_t param3;
};

// Mosquitto class
 class MyMosquitto : public mosqpp::mosquittopp {
  public:
    MyMosquitto() : mosqpp::mosquittopp ("PiBrain") {  mosqpp::lib_init(); }

      virtual void on_connect (int rc) { printf("Connected to Mosquitto\n"); }

      virtual void on_disconnect () { printf("Disconnected\n"); }

      virtual void on_message(const struct mosquitto_message* mosqmessage) {

      // Message received on a channel we subscribe to
      printf("Message found on channel %s: %s\n", mosqmessage->topic, (char*)mosqmessage->payload);

      // determing node target based on channel
      uint16_t target_node = 0;
      if (strcmp(mosqmessage->topic, action_channel1) == 0) {
        target_node = action_node1;
      } else if (strcmp(mosqmessage->topic, action_channel2) == 0) {
        target_node = action_node2;
      } else {
        printf("FAILED: Unknown MQTT channel!");
      }

      message_action actionmessage;
      char messagePayload[13];
      strcpy(messagePayload, (char*)mosqmessage->payload);

      char modeArray[4] = {messagePayload[0], messagePayload[1], messagePayload[2]};
      char paramArray1[4] = {messagePayload[3], messagePayload[4], messagePayload[5]};
      char paramArray2[4] = {messagePayload[6], messagePayload[7], messagePayload[8]};
      char paramArray3[4] = {messagePayload[9], messagePayload[10], messagePayload[11]};
      unsigned int mode = atoi(modeArray);
      unsigned int param1 = atoi(paramArray1);
      unsigned int param2 = atoi(paramArray2);
      unsigned int param3 = atoi(paramArray3);

      actionmessage = (message_action){ mode, param1, param2, param3 };

      // send message on the RF24Network
      printf("Sending instructions to node %i\n", target_node);

      RF24NetworkHeader header(target_node);
      bool ok = network.write(header, &actionmessage, sizeof(actionmessage));

      if (ok){
        printf("Command sent to node %i\n", target_node);
      }else{
        bool sent = false;
        for ( int i = 0; i < 10; i++) {
          delay (50);
          bool retry = network.write(header, &actionmessage, sizeof(actionmessage));
          if (retry) {
            sent = true;
            printf("Sent via retry.\n");
            break;
          }
        }
        if (!sent) { printf("Send failed.\n"); }
      }
    }
 };

 MyMosquitto mosq;

int main(int argc, char** argv)
{
  // Initialize radio
  radio.begin();
  delay(5);
  network.begin(90, pi_node);

  // Initialize MQTT client and subscriptions
  mosq.connect("127.0.0.1");
  mosq.subscribe(0, action_channel1);
  mosq.subscribe(0, action_channel2);

  // main loop
  while (true) {
    network.update();

    if (network.available()){
      printf("Information available");
      RF24NetworkHeader header;
      message_sensor payload;
      network.read(header,&payload,sizeof(payload));

      int sensor = payload.sensor;
      int param1 = payload.param1;
      int param2 = payload.param2;
      int param3 = payload.param3;

      printf(" from sensor: %i\n", sensor);

      if(sensor == sensor_node3) {
        if (param1 == 1) {
          char buffer_temp[50];
        	char buffer_humidity[50];

          printf("Bedroom temp: %i\n", payload.param2);
          printf("Bedroom humidity: %i\n", payload.param3);

        	sprintf (buffer_temp, "mosquitto_pub -t home/master/temp -r -m \"%i\"", param2);
        	sprintf (buffer_humidity, "mosquitto_pub -t home/master/humidity -r -m \"%i\"", param3);
        	system(buffer_temp);
        	system(buffer_humidity);
        }
      } else {
        printf("Unknown sensor!");
      }
    }

    // check for messages on subscribed MQTT channels
    mosq.loop();
    printf(".\n");
  }

  return 0;
}
