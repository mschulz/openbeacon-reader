// Sniff the Openbeacon messages with the NRF24 class.
// Feed the messages to the USB port for upstream processing,
// hopefully using a Raspberry Pi. :-)  This is much simpler than
// using the Netduino as a base platform, running FreeRTOS.  The real hassle
// is to determine just how much processing to do on the Arduino, and how
// much to leave for the upstream processor.
//
// Wire the Sparkfun nRF24L01+ breakout board with a Uno as follows (Arduino pin => Nordic pin)
// GND == GND
// 3.3V == Vcc
// 11 == MOSI
// 12 == MISO
// 13 == SCK
// 10 == CSN for Ethernet
//  7 == CSN for nRF24L01+
//  8 == CE
//       IRQ is left disconnected

#include <NRF24.h>
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <util.h>
#include <EEPROM.h> 

const int MSG_LEN = 16;

void callback(char* topic, byte* payload, unsigned int length)
{
  // handle message arrived
}

// Sert up Ehetnet client
EthernetClient ethClient;

// Set up MQTT service
byte server[] = { 130, 102, 129, 175 };  // winter.ceit.uq.edu.au
PubSubClient client(server, 1883, callback, ethClient);

int greenLEDpin = 4;

// Singleton instance of the radio
NRF24 nrf24(8, 7);

// A couple of buffers to hold crafted strings
static char topicString[80];
static char mqttClientID[80];

void setup() 
{
  static char IPstring[80];
  // Strings used to initialize the nRF24L01+ to work with the badges
  static unsigned char SMAC[] = {0x01, 0x02, 0x03, 0x02, 0x01};
  static unsigned char TMAC[] = {'O', 'C', 'A', 'E', 'B'};
  // EEPROM locations:
  // Unique MAC address can be stored in EEPROM using 'setMACaddress' sketch, which sets the last 3 bytes of this address.
  const int eepromMacFlag = 0; // 1 byte
  int eepromMacAddress = 1; // 3 bytes
  // Default MAC address for the ethernet controller.
  static byte mac[] = {0x90, 0xA2, 0xDA, 0x00, 0xFF, 0xFF}; 

   pinMode(greenLEDpin, OUTPUT);
   
  // start serial port:
  Serial.begin(115200);

  // start the Ethernet connection and the server:
  // Retrieve MAC address from EEPROM if present
  if (EEPROM.read(eepromMacFlag) == '#') {
    Serial.println("Retrieving MAC address from EEPROM");
    for (int i = 3; i < 6; i++)
      mac[i] = EEPROM.read(eepromMacAddress++);
  } else {
    Serial.println("No MAC address stored in EEPROM");
    Serial.println("Using default MAC address");
  }  

  // Publish the MAC address to serial port
  {
    const int outputStringLen = 40;
    static char outputString[outputStringLen];  // buffer used for constructing output strings
    
    snprintf(outputString, outputStringLen, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    Serial.print("MAC Address: "); Serial.println(outputString);
  }

  // Use DHCP to determine my IP address
  Serial.println("DHCP...");
  while (Ethernet.begin(mac) == 0) {
    Serial.println("No DHCP");
    delay(1000);
    Serial.println("DHCP...");
  }  
  Serial.println("IP address: ");
  sprintf(IPstring, "%u.%u.%u.%u", Ethernet.localIP()[0], Ethernet.localIP()[1], Ethernet.localIP()[2], Ethernet.localIP()[3]);
  Serial.println(IPstring);

  // Craft the topic to publish on, using the IP address of this machine
  strcpy(topicString, "/openbeacon/LIB/fastRaw/reader/");
  strcat(topicString, IPstring);
  strcat(topicString, "/");

  // Craft the MQTT client ID, which must be unique.  Use the IP address of this machine to make it so.
  sprintf(mqttClientID, "%u", Ethernet.localIP()[3]);
  strcat(mqttClientID, "-arduino");

  // Start on the RF link
  nrf24.init();
    
  // Sert up all the registers on the nRF24L01+ via the SPI connection
  nrf24.spiWriteRegister(NRF24_REG_00_CONFIG, 0x00);  // Stop nRF
  nrf24.spiWriteRegister(NRF24_REG_01_EN_AA, 0x00);  //Disable Shockburst
  nrf24.spiWriteRegister(NRF24_REG_02_EN_RXADDR, NRF24_ERX_P0); //Set Rx Pipe 0
  nrf24.spiWriteRegister(NRF24_REG_03_SETUP_AW, NRF24_AW_5_BYTES); // Setup AW for 5 bytes
  if (!nrf24.setChannel(81))
    Serial.println("setChannel failed");   
  if (!nrf24.setRF(NRF24::NRF24DataRate2Mbps, NRF24::NRF24TransmitPowerm18dBm))
    Serial.println("setRF failed");     
  nrf24.spiWriteRegister(NRF24_REG_07_STATUS, NRF24_RX_DR|NRF24_TX_DS|NRF24_MAX_RT|0x08); // Clear status regsiter
  nrf24.spiBurstWriteRegister(NRF24_REG_0A_RX_ADDR_P0, SMAC, 5);   
  nrf24.spiBurstWriteRegister(NRF24_REG_10_TX_ADDR, TMAC, 5);    
  if (!nrf24.setPayloadSize(MSG_LEN))
    Serial.println("setPayloadSize failed");    
  nrf24.spiWriteRegister(NRF24_REG_00_CONFIG, NRF24_MASK_RX_DR|NRF24_MASK_TX_DS|NRF24_MASK_MAX_RT|NRF24_EN_CRC|NRF24_PWR_UP|NRF24_PRIM_RX);
}

void loop()
{
  unsigned char len;
  static unsigned char  data_buffer[MSG_LEN];
 
  if (!client.connected()) {
    if (!client.connect(mqttClientID)) {
      delay(1000);
      return;
    }
  }
  
  nrf24.powerUpRx();
  if (nrf24.spiReadRegister(NRF24_REG_07_STATUS) & NRF24_RX_DR) {
    if (nrf24.recv(data_buffer, &len)) {     
      // Only accept 16 byte messages, for now
      if (len != MSG_LEN) {
        client.publish("/openbeacon/LIB/log/", "Bytes in message not equal to 16");
        return;
      }
       
      // Do the minimum amount of processing of the message here so that we don't miss packets if possible.
      // Flash the LED to give a visual indication of traffic rate.
            
      digitalWrite(greenLEDpin, HIGH);         
      client.publish(topicString, data_buffer, 16);  
      digitalWrite(greenLEDpin, LOW);
    }
  }
  client.loop();
}

