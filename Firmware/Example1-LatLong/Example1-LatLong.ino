/*
  Reading Lat/Long from the Qwiic GPS module over I2C
  By: Nathan Seidle
  SparkFun Electronics
  Date: April 12th, 2017
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).



  Hardware Connections:
  Attach the Qwiic Mux Shield to your RedBoard or Uno.
  Plug two Qwiic MMA8452Q breakout boards into ports 0 and 1.
  Serial.print it out at 9600 baud to serial monitor.

  Serial.print it out at 9600 baud to serial monitor.

  Available functions:

  To Write:

*/

#include <Wire.h>

#define MT333x_ADDR 0x10 //7-bit unshifted default I2C Address

#define MAX_PACKET_SIZE 255
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

//Reads a 255 byte packet from GPS module
void readGPSPacket()
{
  //Arduino can only Wire.read() in 32 byte chunks. Yay.
  for (byte chunk = 0 ; chunk < 7 ; chunk++) //8 chunks * 32 = 256 bytes total so we need to shave one
  {
    if (Wire.requestFrom(MT333x_ADDR, 32))
    {
      for (byte x = 0 ; x < 32 ; x++) //Read 32 bytes into the 256 array
        packetData[(chunk * 32) + x] = Wire.read();
    }
    else
    {
      Serial.println("Request failed");
    }
  }

  //Read final 31 bytes
  if (Wire.requestFrom(MT333x_ADDR, 31))
  {
    for (byte x = 0 ; x < 31 ; x++) //Read 32 bytes into the 256 array
      packetData[(7 * 32) + x] = Wire.read();
  }
  else
  {
    Serial.println("Request failed");
  }
}
