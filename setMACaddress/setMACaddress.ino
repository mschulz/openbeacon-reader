/*
 * Set_MAC_Address
 * 
 * Prompts for a MAC address from the serial port and stores it in EEPROM for use by later sketches.
 * 
 */
  
#include <EEPROM.h>

const int eepromMacFlag = 0; // 1 byte
int eepromMacAddress = 1; // 3 bytes

// we need fundamental FILE definitions and printf declarations
#include <stdio.h>

// create a FILE structure to reference our UART output function

static FILE uartout = {0} ;

// create a output function
// This works because Serial.write, although of
// type virtual, already exists.
static int uart_putchar (char c, FILE *stream)
{
    Serial.write(c) ;
    return 0 ;
}

// Set up ethernet
// assign a MAC address for the ethernet controller.
// MAC address is stored in EEPROM on initial run
byte mac[] = {0x90, 0xA2, 0xDA, 0x00, 0x00, 0x00};

byte loadMAC = true;

void parseLine()
{
  static char buffer[32];
  char *bp;

  bp = buffer;
  while (1) {
    while (Serial.available() == 0)
      ;
    char ch = Serial.read();
    
    if (ch == '\r' || ch == '\n') {
      *bp = '\0';
      sscanf(buffer, "%02x:%02x:%02x", &mac[3], &mac[4], &mac[5]);
      return;
    } else if (bp < &buffer[32]) {
     *bp++ = ch;
    }
  }
}

char * printMac(byte * mac)
{
  char macstr[18];
  snprintf(macstr, 18, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  Serial.println(macstr);
}

void setup()
{
  // start serial port:
  Serial.begin(9600);

  Serial.println("");
  
  // fill in the UART file descriptor with pointer to writer.
   fdev_setup_stream (&uartout, uart_putchar, NULL, _FDEV_SETUP_WRITE);

   // The uart is the standard output device STDOUT.
   stdout = &uartout ;
   
  // Retrieve MAC address from EEPROM if present
  if (EEPROM.read(eepromMacFlag) == '#') {
    Serial.print("MAC address already in EEPROM: ");
    for (int i = 0; i < 3; i++)
      mac[i+3] = EEPROM.read(eepromMacAddress + i);
    printMac(mac);
   
    Serial.print("Overwrite (Y/n)? ");
    while (Serial.available() == 0)
      ; // do nothing
      
    char c = Serial.read();
    
    if (c == 'n' || c == 'N')
      loadMAC = false;
    Serial.println("");
  } 
  
  if (loadMAC) {   
    Serial.println("Initialising MAC address");
    Serial.println("");
    Serial.print("Enter MAC address n:n:n ");
    Serial.print(": ");
    while (Serial.available() == 0)
        ; // do nothing
    Serial.read(); // Throw away CR or NL character
    parseLine();

    for (int i = 3; i < 6; i++)
      EEPROM.write(eepromMacAddress++, mac[i]);

    EEPROM.write(eepromMacFlag, '#');

    Serial.println("");
    Serial.print("Saved MAC Address:");
    printMac(mac);
  } else
    Serial.println("Aborted");
}

void loop()
{
}
