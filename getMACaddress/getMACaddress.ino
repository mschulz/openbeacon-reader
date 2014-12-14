#include <SPI.h>
#include <util.h>
#include <EEPROM.h> 
#include <Ethernet.h>

// EEPROM locations
const int eepromMacFlag = 0; // 1 byte
const int eepromMacAddress = 1; // 3 bytes

// Default MAC address for the ethernet controller.
// Unique MAC address can be stored in EEPROM using Set_MAC_Address sketch
byte mac[] = {0x90, 0xA2, 0xDA, 0x00, 0xFF, 0xFF};

const int outputStringLen = 200;
char outputString[outputStringLen];  // buffer used for constructing output strings

void setup() {
  // start serial port:
  Serial.begin(9600);

  // start the Ethernet connection and the server:
  // Retrieve MAC address from EEPROM if present
  if (EEPROM.read(eepromMacFlag) == '#') {
    Serial.println("Retrieving MAC address from EEPROM");
    for (int i = 0; i < 3; i++) {
      mac[i+3] = EEPROM.read(eepromMacAddress + i);
    }
  } 
  else {
    Serial.println("No MAC address stored in EEPROM");
    Serial.println("Using default MAC address");
  }  

  snprintf(outputString, outputStringLen, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  Serial.print("MAC Address: ");
  Serial.println(outputString);

  Serial.println("DHCP...");

  while (Ethernet.begin(mac) == 0) {
    Serial.println("No DHCP");
    delay(1000);
    Serial.println("DHCP...");
  }
  Serial.println("IP address: ");
  Serial.println(Ethernet.localIP());

}

void loop()
{
}
