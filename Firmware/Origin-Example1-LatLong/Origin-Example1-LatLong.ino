/*
  Reading Lat/Long from the Qwiic GPS module over I2C
  By: Nathan Seidle
  SparkFun Electronics
  Date: April 12th, 2017
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

  Hardware Connections:
  Attach the Qwiic Mux Shield to your RedBoard or Uno.
  Plug two Qwiic MMA8452Q breakout boards into ports 0 and 1.
  Serial.print it out at 115200 baud to serial monitor.

  Available functions:

  To Write:

*/

#include <Wire.h>

#define MT333x_ADDR 0x10 //7-bit unshifted default I2C Address

#define MAX_PACKET_SIZE 8
byte packetData[MAX_PACKET_SIZE]; //Store all incoming I2C bytes
byte gpsData[MAX_PACKET_SIZE]; //The place to store valid incoming data

void setup()
{
  Serial.begin(115200);
  Serial.println("GTOP Read Example");

  Wire.begin();
  Wire.setClock(400000); //Run I2C at fast 400kHz
}

void loop()
{
  if(checkGPS() == true)
  {
    //We have new GPS data to deal with!
  }

  delay(1000);
  
}

//Poll GPS and see if anything new has come in
//Loads the 
boolean checkGPS()
{
  readGPSPacket();

  if (dataAvailable())
  {
    Serial.println();
    Serial.println();
    Serial.print("GPS Data:");

    while (dataAvailable())
    {
      byte validSpot = 0;
      
      //Move valid data from packet to gpsData array
      for (byte x = 0 ; x < MAX_PACKET_SIZE ; x++)
      {
        if (packetData[x] != 0x0A)
          gpsData[validSpot++] = packetData[x];
      }

      if (validSpot > 0)
      {
        for (byte x = 0 ; x < validSpot ; x++)
        {
          //if (packetData[x] == 0x0A) break;

          if (gpsData[x] == '$') Serial.println();
          Serial.write(gpsData[x]);
        }
      }

      delay(2); //Wait for module to refill the buffer

      readGPSPacket(); //Go get new data
    } //End while dataAvaiable()
  }
}

//Checks packetData to see if it contains anything other than garbage 0x0A bytes
boolean dataAvailable()
{
  for (byte x = 0 ; x < MAX_PACKET_SIZE ; x++)
    if (packetData[x] != 0x0A) return (true);

  return (false);
}

//Reads a 8 byte packet from GPS module
void readGPSPacket()
{
  if (Wire.requestFrom(MT333x_ADDR, 8))
  {
    Serial.println("Response");
    while(1);
    
    for (byte x = 0 ; x < 8 ; x++) //Read 8 bytes into the array
      packetData[x] = Wire.read();
  }
  else
  {
    Serial.println("Request failed");
  }
}
