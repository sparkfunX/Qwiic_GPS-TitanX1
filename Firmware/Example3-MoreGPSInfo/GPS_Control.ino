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

//Send a given command or configuration string to the module
//The input buffer on the MTK is 255 bytes. Caller must keep strings shorter than 255 bytes
boolean sendMTKcommand(String command)
{
  Serial.print("length: ");
  Serial.print(command.length());

  if (command.length() > 255)
  {
    Serial.println("Command message too long!");
    return (false);
  }

  //Arduino can only Wire.write() in 32 byte chunks. Yay.
  for (byte chunk = 0 ; chunk < 7 ; chunk++) //8 chunks * 32 = 256 bytes total so we need to shave one
  {
    Wire.beginTransmission(MT333x_ADDR);
    for (byte x = 0 ; x < 32 ; x++) //Send out 32 bytes
    {
      if ( (chunk * 32 + x) == command.length()) break; //We're done!
      Wire.write(command[(chunk * 32) + x]);
    }
    Wire.endTransmission();

    delay(10); //Slave requires 10 ms to process incoming bytes
  }

  //Send final 31 bytes
  /*Wire.beginTransmission(MT333x_ADDR);
    for (byte x = 0 ; x < 31 ; x++) //Write any remaining bytes up to 255
    {
    if ( (7 * 32 + x) == command.length()) break; //We're done!
    Wire.write(command[(7 * 32) + x]);
    }
    Wire.endTransmission();*/

  Serial.println("Command sent!");

  //Now continue with our normal parsing
  //configureCmd and configureFlag

}

String createMTKsentence(int flagNumber, String flags)
{
  //Build config sentence using flagNumber
  String configSentence = "";
  configSentence += "$PMTK"; //Default header for all GPS config messages

  //Attach the flag number
  //Append any leading zeros
  if (flagNumber < 100) configSentence += "0";
  if (flagNumber < 10) configSentence += "0";
  configSentence += flagNumber;

  //Attach any settings
  if (flags.length() > 0)
  {
    configSentence += flags; //Attach the string of flags
  }

  configSentence += "*"; //Attach end tag

  configSentence += calcCRCforMTK(configSentence); //Attach CRC

  //Attach ending bytes
  configSentence += '\r'; //Carriage return
  configSentence += '\n'; //Line feed

  return (configSentence);
}

//Calculate CRC for MTK messages
//Given a string of characters, XOR them all together and return CRC in string form
String calcCRCforMTK(String sentence)
{
  byte crc = 0;

  //We need to ignore the first character $
  //And the last character *
  for (int x = 1 ; x < sentence.length() - 1 ; x++)
    crc ^= sentence[x]; //XOR this byte with all the others

  String output = "";
  if (crc < 10) output += "0"; //Append leading zero if needed
  output += String(crc, HEX);

  return (output);
}
