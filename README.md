# Remi MQTT POC project #

The purpose of this project is to demonstrate the path of data from low level Linux to cloud.
Imagine we have a hardware input event coming in, that data has to be catch by the Linux kernel, then acceced by an application in the user space. The app will published that data on an MQTT broker which is a data-bus for cloud like usage.

Finally, one last layer will be to embed the application in a Docker container.

NB: This project has been made under a Ubuntu 22.04 OS. Should be straight forward for most debian like config.

## Low level layer ##

To simulate the hardware input event, we will build a Linux module which will expose an interface file in **/proc/**. Through this file, the module will echo data coming in towards the app reading the file.
ex: In one terminal write "+1" to the driver file interface, and in another one read the file, you will get the "+1".

This is under the **echo-module** repository:
[github.com/nungesser/echo-module.git](https://github.com/nungesser/echo-module.git)


### How to build the module ###

On top of regular build env (gcc, make, ...) you will need the Linux headers of your machine.
Depending on your machine it may look like one of these command to install them:
``` console
 # apt install linux-headers-amd64
 # apt install linux-headers-generic
 # apt install linux-headers-generic-hwe-22.04
```
To get an idea of the package name you need, check the output of:
``` console
 $ dpkg -l | grep linux-image
 $ uname -r
```
If headers are there, the following command should return without error:
``` console
 $ ls -l /lib/modules/`uname -r`/build
lrwxrwxrwx 1 root root 39 juil. 13 15:22 /lib/modules/6.2.0-26-generic/build -> /usr/src/linux-headers-6.2.0-26-generic
```

Then, you can go in **echo-module** folder and build the module: 
``` console
 $ git clone https://github.com/nungesser/echo-module.git
 $ cd echo-module
 $ make
```
This will build the **my_echo.ko** kernel object (module).


### How to use it ###

Check it's not already running:
``` console
 $ lsmod | grep my_echo
```
Load it:
``` console
 # insmod my_echo.ko
```
It should say Welcome in the kernel log. Check **dmesg**.

At this point you should find the driver interface in /proc/ as my_echo.
``` console
 $ ls -l /proc/my_echo
-rw-rw-rw- 1 root root 0 août  28 11:22 /proc/my_echo
```

Open 2 terminals:

  * In the first:
```console
 $ echo +1 > /proc/my_echo
```
  * In a 2nd one:
``` console
 $ cat /proc/my_echo
+1
```
It should display what has been send to it (+1).

NB: There is a 200ms wait timer. If too much data comming in within this amount of time, it's simply dropped.


To unload the driver:
``` console
 # rmmod my_echo
```


## Application layer ##

The application sources are under the **remi-mqtt** [repository](https://github.com/nungesser/remi-mqtt.git) you read this file from.
``` console
 $ cd remi-mqtt
```
The application needs the **my_echo** module running on one side.
The app reads the **/proc/my_echo** driver interface file and increment a counter value every time it reads "+1".

On the other side, the app needs a **MQTT broker** to connect on.

You need to provide environments variable:

  * **BROKER_URI** (format: `tcp://<host>:<port>`) giving the address of the MQTT broker.
  * **DEVICE_ID**: device id
  * **DEVICE_KEY**: the password

NB: the username is currently embedded at the top of the remi-mqtt.c source file as device-iot.

There is a **env.sh** file where you can set your variables, then source the file:
``` console
 $ source env.sh 
```
The application will pubish the counter value on the following topic of the MQTT brocker:
```
iot/dev/${DEVICE_ID}/data
```

### How to build ###

The application use a MQTT client library that needs to be installed
```console
 # apt install libpaho-mqtt-dev
```
Then, run:
``` console
 $ make
```
This will build **remi-mqtt** bin


### How to use it ###

You need a MQTT brocker.
Ex:
``` console
 # apt install mosquitto
```
This will install that MQTT brocker, available then on localhost.

You would need a MQTT client comand to spy what's happening on the brocker.
``` console
 # apt install mosquitto-clients
```
To check what's up:
``` console
 $ mosquitto_sub -h localhost -t iot/dev/# -v
 $ mosquitto_sub -h localhost -t iot/dev/# -u username -P password -v
```
Either one, depending on login authentification or not and feel free to replace localhost by the ip of your brocker.

Keep that terminal open.


You need the **my_echo** module loaded as described in the Low level layer part.

In it's own terminal launch the app:

``` console
 $ ./remi-mqtt
```

In a 3rd terminal,

write +1 values in the file interface of my_echo module:
``` console
 $ echo +1 > /proc/my_echo
 $ echo +1 > /proc/my_echo
```
The app terminal should display this kind of message:
```
Publication of { "type": "measure","value": { "counter": "1" }}
on topic iot/dev/IRsensor/data
```

The mosquitto_sub terminal should display:
```
iot/dev/device_id/data { "type": "measure","value": { "counter": "1" }}
iot/dev/device_id/data { "type": "measure","value": { "counter": "2" }}
```

If you keep writing +1 to **/proc/my_echo**, you will see the counter increasing.
In echo-module repo, there is a **sensor_run.sh** script that will do it every second. 


## Docker container layer ##

### Requirements ###
You will need a docker daemon installed.
Have a look at [the official docker install page](https://docs.docker.com/engine/install/).

Then, to be able to use the **docker-compose.yml** file, first:
``` console
 # apt install docker-compose
```

### How to build the docker image ###

To build a docker image that embeded the app:
``` console
 $ sudo make docker-build
```
This will create and add locally a docker image called **debian-mqtt**.
The app is compilled during the docker build.


### How to use it ###

To start the container:
Edit the **docker-compose.yml** and set your own values to the env var **BROKER_URI**, **DEVICE_ID**, **DEVICE_KEY**.

Start the container:
``` console
 $ sudo docker-compose up -d
```
Check it's running
``` console
 $ sudo docker ps
```
should show a **remi-ctn** container.

To rebuild the image, you have first to stop and remove the container:
``` console
 $ sudo docker-compose down
```


## TODO ##
- Move the MQTT part of the app in a dedicated thread so the application could keep counting in case of network delay or issue.
- Make the docker image lighter.
- Check why the container takes time to stop.
- Build for Rapsberry pi 4.
- Use a json lib.