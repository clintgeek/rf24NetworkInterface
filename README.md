# rf24NetworkInterface
An interface between the rf24Network library and MQTT on a rasPi
A simplified, ESP8266 compatible version is located in arduinoProjects as `this_node`.

This program is designed to run on a RaspberryPi and interfaces between the rf24Network network and the local Mosquitto broker.

### Dependencies:
* rf24 and rf24Network Libraries
	* Install the `rf24` and `rf24Network` libraries using the installer script. The remaining libraries are unnecessary.

		~~~
		wget http://tmrh20.github.io/RF24Installer/RPi/install.sh
		chmod +x install.sh
		./install.sh
		~~~

* Mosquitto Client
	* Add the repo and key, update apt-get, and install necessary client libraries.

		~~~
		wget http://repo.mosquitto.org/debian/mosquitto-repo.gpg.key
		sudo apt-key add mosquitto-repo.gpg.key
		rm mosquitto-repo.gpg.key
		cd /etc/apt/sources.list.d/
		sudo wget http://repo.mosquitto.org/debian/mosquitto-jessie.list
		sudo apt-get update
		sudo apt-get install mosquitto mosquitto-clients libmosquittopp1 libmosquittopp-dev
		~~~

To make: `sudo make tx`
To run: `sudo ./tx`
