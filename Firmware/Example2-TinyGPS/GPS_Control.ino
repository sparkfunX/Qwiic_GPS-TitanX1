/*
  Basic control functions of the MT333x GPS module
*/

#define MT333x_ADDR 0x10 //7-bit unshifted default I2C Address

#define MAX_PACKET_SIZE 255
byte packetData[MAX_PACKET_SIZE]; //Store all incoming I2C bytes
byte gpsData[MAX_PACKET_SIZE]; //The place to store valid incoming data

boolean gpsDebugging = false; //A flag for debugging

//Poll GPS and see if anything new has come in
//Return true if we've received a complete sentence
//True does not mean valid lock, just that we've gotten enough data (full NMEA sentence) to check
boolean checkGPS()
{
  boolean newGPS = false;

  long startTime = millis();

  readGPSPacket(); //Check to see if new I2C bytes are available on the GPS module

  if (dataAvailable())
  {
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
          if (gps.encode(gpsData[x]) == true) //Feed the GPS parser
          {
            newGPS = true; //newGPS becomes true once gps.encode has enough data
          }

          if (gpsDebugging == true)
          {
            if (gpsData[x] == '$') Serial.println();
            Serial.write(gpsData[x]);
          }
        }
      }

      //delay(2); //Wait for module to refill the buffer

      readGPSPacket(); //Go get new data
    } //End while dataAvaiable()
  }

  if (gpsDebugging == true)
  {
    Serial.println();
    Serial.print(F("Time to obtain data: "));
    Serial.println(millis() - startTime);
  }

  return (newGPS); //Returns true if gps.encode() is true, then we can query GPS info
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
  }

  //Read final 31 bytes
  if (Wire.requestFrom(MT333x_ADDR, 31))
  {
    for (byte x = 0 ; x < 31 ; x++) //Read 32 bytes into the 256 array
      packetData[(7 * 32) + x] = Wire.read();
  }
}

//Enables serial printing of the raw incoming GPS characters
//Helpful for seeing what is coming in
void enableDebugging()
{
  gpsDebugging = true;
}

//Turn off printing of GPS character streams
void disableDebugging()
{
  gpsDebugging = false;
}
