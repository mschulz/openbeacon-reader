#Openbeacon Reader on the Arduino

Here is a simple rendition of an Openbeacon reader running on an Arduino. There are no frills in this implementation.  The system consists of:

* an Arduino Uno + Ethernet shield OR a Freetronics Eleven
* an nRF24L01+ from Sparkfun
* an antenna (to increase the receive range)


<img src="http://ceit.uq.edu.au/system/files/blog/obarduinoreader.jpg" width="200px" height="200px"/>

##Operation

We are interested in using the raw output from  the readers in a number of different research projects, some simultaneously.  To this end, we take the data from the reader and deliver it to a publish/subscribe server running the MQTT protocol. 

MQTT is a topic-based pubsub protocol.  In our lab, raw data is published to the topic:

```
/openbeacon/LIB/fastRaw/reader/<IP address>/
```

Any other messages from the reader are sent to the topic: 

```
/openbeacon/LIB/log
```
The current version of the software only reports on this log topic if a received message is NOT 16 bytes long (the size of an Openbeacon message).

Mark Schulz
October 13 2014

