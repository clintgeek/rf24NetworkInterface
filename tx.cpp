#include <RF24/RF24.h>
#include <RF24Network/RF24Network.h>
#include </usr/include/mosquittopp.h>
#include </usr/include/mosquitto.h>
#include <iostream>
#include <string>
#include <sstream>
#include <ctime>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

using namespace mosqpp;

// Constants that identify nodes
const uint16_t pi_node = 0;
const uint16_t action_node1 = 1; // home/master/deskLamp
const uint16_t action_node2 = 2; // home/master/tvLight
const uint16_t sensor_node3 = 3; // currently unused

// channels to subscribe to
const char* action_channel1 = "home/master/deskLamp";
const char* action_channel2 = "home/master/tvLight";

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


      printf("ParamArray1: %s\n", paramArray1);
      printf("Param1: %i\n", param1);

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
  network.update();

  // Initialize MQTT client and subscriptions
  mosq.connect("127.0.0.1");
  mosq.subscribe(0, action_channel1);
  mosq.subscribe(0, action_channel2);

  // main loop
  while (true) {
    // this is where we will monitor the RF24Network for updates from sensors

    // check for messages on subscribed MQTT channels
    mosq.loop();
    printf(".\n");

    // give a little pause and do it all again
    delay(interval);
  }

  return 0;
}
