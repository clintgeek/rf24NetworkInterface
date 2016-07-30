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
const uint16_t office_node = 01;
const uint16_t master_bedroom_node = 02;
const uint16_t living_room_node = 03;

// Sensor types
const uint16_t dht_values = 01;
const uint16_t tv_power = 02;
const uint16_t rgb_values = 03;
const uint16_t rgb_brightness = 04;

// channels to subscribe to
const char* office_desk_lamp_command = "home/office/desk_lamp/command";
const char* office_desk_lamp_rgb_command = "home/office/desk_lamp/rgb/command";
const char* office_desk_lamp_brightness_command = "home/office/desk_lamp/brightness/command";
const char* master_bedroom_tv_lamp_command = "home/master/tv_lamp/command";
const char* master_bedroom_tv_lamp_rgb_command = "home/master/tv_lamp/rgb/command";
const char* master_bedroom_tv_lamp_brightness_command = "home/master/tv_lamp/brightness/command";
const char* living_room_tv_lamp_command = "home/living_room/tv_lamp/command";
const char* living_room_tv_lamp_rgb_command = "home/living_room/tv_lamp/rgb/command";
const char* living_room_tv_lamp_brightness_command = "home/living_room/tv_lamp/brightness/command";

// channels to publish to
const char* office_temperature_state = "home/office/temperature/state";
const char* office_humidity_state = "home/office/humidity/state";
const char* office_desk_lamp_command_lamp_rgb_state = "home/office/desk_lamp/rgb/state";
const char* office_desk_lamp_brightness_state = "home/office/desk_lamp/brightness/state";
const char* master_bedroom_temperature_state = "home/master/temperature/state";
const char* master_bedroom_humidity_state = "home/master/humidity/state";
const char* master_bedroom_tv_power_state = "home/master/tv_power/state";
const char* master_bedroom_tv_lamp_rgb_state = "home/master/tv_lamp/rgb/state";
const char* master_bedroom_tv_lamp_brightness_state = "home/master/tv_lamp/brightness/state";
const char* living_room_temperature_state = "home/living_room/temperature/state";
const char* living_room_humidity_state = "home/living_room/humidity/state";
const char* living_room_tv_power_state = "home/living_room/tv_power/state";
const char* living_room_tv_lamp_rgb_state = "home/living_room/tv_lamp/rgb/state";
const char* living_room_tv_lamp_brightness_state = "home/living_room/tv_lamp/brightness/state";

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
  uint16_t sensor_type;
  uint16_t param1;
  uint16_t param2;
  uint16_t param3;
};

// Mosquitto class
class MyMosquitto : public mosqpp::mosquittopp {
  public:
    MyMosquitto() : mosqpp::mosquittopp ("PiBrain") { mosqpp::lib_init(); }

  virtual void on_connect (int rc) { printf("Connected to Mosquitto\n"); }

  virtual void on_disconnect () { printf("Disconnected\n"); }

  virtual void on_message(const struct mosquitto_message* mosqmessage) {

    // Message received on a channel we subscribe to
    printf("Message found on channel %s: %s\n", mosqmessage->topic, (char*)mosqmessage->payload);

    // determing node target based on channel
    uint16_t target_node = 0;
    bool ha_rgb = false;
    if (strcmp(mosqmessage->topic, office_desk_lamp_command) == 0) {
      target_node = office_node;
    } else if (strcmp(mosqmessage->topic, office_desk_lamp_brightness_command) == 0) {
      target_node = office_node;
    } else if (strcmp(mosqmessage->topic, office_desk_lamp_rgb_command) == 0) {
      target_node = office_node;
      ha_rgb = true;
    } else if (strcmp(mosqmessage->topic, master_bedroom_tv_lamp_command) == 0) {
      target_node = master_bedroom_node;
    } else if (strcmp(mosqmessage->topic, master_bedroom_tv_lamp_brightness_command) == 0) {
      target_node = master_bedroom_node;
    } else if (strcmp(mosqmessage->topic, master_bedroom_tv_lamp_rgb_command) == 0) {
      target_node = master_bedroom_node;
      ha_rgb = true;
    } else if (strcmp(mosqmessage->topic, living_room_tv_lamp_command) == 0) {
      target_node = living_room_node;
    } else if (strcmp(mosqmessage->topic, living_room_tv_lamp_brightness_command) == 0) {
      target_node = living_room_node;
    } else if (strcmp(mosqmessage->topic, living_room_tv_lamp_rgb_command) == 0) {
      target_node = living_room_node;
      ha_rgb = true;
    } else {
      printf("FAILED: Unknown MQTT channel!");
    }

    message_action actionmessage;
    char messagePayload[13];
    strcpy(messagePayload, (char*)mosqmessage->payload);

    unsigned int mode;
    unsigned int param1;
    unsigned int param2;
    unsigned int param3;

    if (ha_rgb) {
      char * rgb;
      int c = 0;
      mode = 001;

      printf ("Splitting string \"%s\" into tokens:\n",messagePayload);
      rgb = strtok (messagePayload,",");
      while (rgb != NULL) {
        if (c == 0) {
          param1 = atoi(rgb);
        } else if (c == 1) {
          param2 = atoi(rgb);
        } else {
          param3 = atoi(rgb);
        }
        c++;
        printf ("%s\n",rgb);
        rgb = strtok (NULL, ",");
      }
    } else {
      char modeArray[4] = { messagePayload[0], messagePayload[1], messagePayload[2] };
      char paramArray1[4] = { messagePayload[3], messagePayload[4], messagePayload[5] };
      char paramArray2[4] = { messagePayload[6], messagePayload[7], messagePayload[8] };
      char paramArray3[4] = { messagePayload[9], messagePayload[10], messagePayload[11] };
      mode = atoi(modeArray);
      param1 = atoi(paramArray1);
      param2 = atoi(paramArray2);
      param3 = atoi(paramArray3);
    }
    actionmessage = (message_action){ mode, param1, param2, param3 };

    // send message on the RF24Network
    printf("Sending instructions to node %i\n", target_node);

    RF24NetworkHeader header(target_node);
    bool ok = network.write(header, &actionmessage, sizeof(actionmessage));

    if (ok){
      printf("Command sent to node %i\n", target_node);
    } else {
      bool sent = false;
      for (int i = 0; i < 10; i++) {
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
  mosq.subscribe(0, office_desk_lamp_command);
  mosq.subscribe(0, office_desk_lamp_rgb_command);
  mosq.subscribe(0, office_desk_lamp_brightness_command);
  mosq.subscribe(0, master_bedroom_tv_lamp_command);
  mosq.subscribe(0, master_bedroom_tv_lamp_rgb_command);
  mosq.subscribe(0, master_bedroom_tv_lamp_brightness_command);
  mosq.subscribe(0, living_room_tv_lamp_command);
  mosq.subscribe(0, living_room_tv_lamp_rgb_command);
  mosq.subscribe(0, living_room_tv_lamp_brightness_command);

  // main loop
  while (true) {
    network.update();

    if (network.available()){
      printf("Information available");
      RF24NetworkHeader header;
      message_sensor payload;
      network.read(header,&payload,sizeof(payload));

      int sensor_id = header.from_node;
      int sensor_type = payload.sensor_type;
      int param1 = payload.param1;
      int param2 = payload.param2;
      int param3 = payload.param3;

      printf(" from sensor: %i\n", sensor_id);

      if(sensor_id == master_bedroom_node) {
        if (sensor_type == dht_values) {
          char buffer_temp[100];
        	char buffer_humidity[100];

          printf("Bedroom temp: %i\n", param1);
          printf("Bedroom humidity: %i\n", param2);
        	sprintf (buffer_temp, "mosquitto_pub -t home/master/temperature/state -r -m \"%i\"", param1);
        	sprintf (buffer_humidity, "mosquitto_pub -t home/master/humidity/state -r -m \"%i\"", param2);
        	system(buffer_temp);
        	system(buffer_humidity);
        } else if (sensor_type == tv_power){
          char buffer_tv_status[50];
          char tv_state[4];

          if (param1 == 1) {
            strcpy(tv_state, "ON");
          } else {
            strcpy(tv_state, "OFF");
          }

          printf("Bedroom TV is: %s\n", tv_state);
          sprintf (buffer_tv_status, "mosquitto_pub -t home/master/tv_power/state -r -m \"%s\"", tv_state);
          system(buffer_tv_status);
        } else if (sensor_type == rgb_values) {
          char buffer_rgbval[100];
          char values_rgb[12];
          char red[4];
          char green[4];
          char blue[4];

          sprintf(red, "%d", param1);
          sprintf(green, "%d", param2);
          sprintf(blue, "%d", param3);
          strcpy (values_rgb, red);
          strcat (values_rgb, ", ");
          strcat (values_rgb, green);
          strcat (values_rgb, ", ");
          strcat (values_rgb, blue);

          printf("Master Bedroom RGB values: \"%s\"", values_rgb);
          sprintf(buffer_rgbval, "mosquitto_pub -t home/master/tv_lamp/rgb/state -r -m \"%s\"", values_rgb);
          system(buffer_rgbval);
        } else if (sensor_type == rgb_brightness) {
          char buffer_rgbbright[100];

          printf("Master Bedoom brightness: \"%i\"", param1);
          sprintf(buffer_rgbbright, "mosquitto_pub -t home/master/tv_lamp/brightness/state -r -m \"%i\"", param1);
          ststem(buffer_rgbbright);
        } else {
          printf("Unknown sensor type!");
        }
      } else if(sensor_id == living_room_node) {
        if (sensor_type == dht_values) {
          char buffer_temp[100];
        	char buffer_humidity[100];

          printf("Living Room temp: %i\n", param1);
          printf("Living Room humidity: %i\n", param2);
        	sprintf (buffer_temp, "mosquitto_pub -t home/living_room/temperature/state -r -m \"%i\"", param1);
        	sprintf (buffer_humidity, "mosquitto_pub -t home/living_room/humidity/state -r -m \"%i\"", param2);
        	system(buffer_temp);
        	system(buffer_humidity);
        } else if (sensor_type == tv_power){
          char buffer_tv_status[50];
          char tv_state[4];

          if (param1 == 1) {
            strcpy(tv_state, "ON");
          } else {
            strcpy(tv_state, "OFF");
          }

          printf("Living Room TV is: %s\n", tv_state);
          sprintf (buffer_tv_status, "mosquitto_pub -t home/living_room/tv_power/state -r -m \"%s\"", tv_state);
          system(buffer_tv_status);
        } else if (sensor_type == rgb_values) {
          char buffer_rgbval[100];
          char values_rgb[12];
          char red[4];
          char green[4];
          char blue[4];

          sprintf(red, "%d", param1);
          sprintf(green, "%d", param2);
          sprintf(blue, "%d", param3);
          strcpy (values_rgb, red);
          strcat (values_rgb, ", ");
          strcat (values_rgb, green);
          strcat (values_rgb, ", ");
          strcat (values_rgb, blue);

          printf("Living Room RGB values: \"%s\"", values_rgb);
          sprintf(buffer_rgbval, "mosquitto_pub -t home/living_room/tv_lamp/rgb/state -r -m \"%s\"", values_rgb);
          system(buffer_rgbval);
        } else if (sensor_type == rgb_brightness) {
          char buffer_rgbbright[100];

          printf("Living Room brightness: \"%i\"", param1);
          sprintf(buffer_rgbbright, "mosquitto_pub -t home/living_room/tv_lamp/brightness/state -r -m \"%i\"", param1);
          ststem(buffer_rgbbright);
        } else {
          printf("Unknown sensor type!");
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
