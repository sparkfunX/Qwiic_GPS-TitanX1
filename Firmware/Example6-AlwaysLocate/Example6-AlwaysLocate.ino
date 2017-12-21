/*
  Configure GPS module for low power operate
  By: Nathan Seidle
  SparkFun Electronics
  Date: April 12th, 2017
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

  This example shows how to configure the module including:
    AlwaysLocate (packet # 225)

  For more information see the MTK NMEA Packet datasheet.

  Note that these settings are stored in battery backed memory and will be retained for
  approximately 16 days without power.

  Hardware Connections:
  Attach a Qwiic shield to your RedBoard or Uno.
  Plug the Qwiic sensor into any port.
  Serial.print it out at 115200 baud to serial monitor.
*/

#include <SparkFun_I2C_GPS_Arduino_Library.h> //Use Library Manager or download here: https://github.com/sparkfun/SparkFun_I2C_GPS_Arduino_Library
I2CGPS myI2CGPS; //Hook object to the library

#include <TinyGPS++.h> //From: https://github.com/mikalhart/TinyGPSPlus
TinyGPSPlus gps; //Declare gps object

//The following tells the TinyGPS library to scan for the PMTK001 sentence
//This sentence is the response to a configure command from the user
//Field 1 is the packet number, 2 indicates if configuration was successful
TinyGPSCustom configureCmd(gps, "PMTK001", 1); //Packet number
TinyGPSCustom configureFlag(gps, "PMTK001", 2); //Success/fail flag

String configString;

boolean debug = false; //Keeps track of the enable/disable of debug printing within the GPS lib

void setup()
{
  Serial.begin(115200);
  Serial.println("GPS Configuration Example");

  if (myI2CGPS.begin() == false)
  {
    Serial.println("Module failed to respond. Please check wiring.");
    while (1); //Freeze!
  }
  Serial.println("GPS module found!");

  //myI2CGPS.enableDebugging(); //Turn on printing of GPS strings
  myI2CGPS.disableDebugging(); //Turn off printing of GPS strings
  debug = false;

  printMenu();
}

void loop()
{
  if (Serial.available())
  {
    byte incoming = Serial.read();
    if (incoming == '1')
    {
      //Packet 220: Set time between fixes (update rate)
      //Milliseconds between output. 100 to 10,000 is allowed.
      //Example 1 Hz: ",1000"
      //Example 2 Hz: ",500"
      //Example 10 Hz: ",100"
      //NOTE: You must increase the baud rate to 57600bps or higher to reach 10Hz
      configString = myI2CGPS.createMTKpacket(220, ",100");
      myI2CGPS.sendMTKpacket(configString);

      Serial.println(F("You must increase the baud rate to 57600bps or higher to reach 10Hz"));
    }
    else if (incoming == '2')
    {
      //Packet 225: Enter Standby or Backup mode for power saving
      //PMTK255, Type, Run time, Sleep time, Second run time, Second sleep time
      //6 types of modes
      
      configString = myI2CGPS.createMTKpacket(225, ",0"); //Go to normal mode
      myI2CGPS.sendMTKpacket(configString);
    }
    else if (incoming == '3')
    {
      //Packet 225: Enter Standby or Backup mode for power saving
      //PMTK255, Type, Run time, Sleep time, Second run time, Second sleep time
      //6 types of modes
      
      configString = myI2CGPS.createMTKpacket(225, ",0"); //Go to normal mode
      myI2CGPS.sendMTKpacket(configString);

      delay(1000);
      
      configString = myI2CGPS.createMTKpacket(223, ",1,25,180000,60000"); //Recommended to be used with 225
      myI2CGPS.sendMTKpacket(configString);

      delay(1000);
      
      configString = myI2CGPS.createMTKpacket(225, ",1,3000,12000,18000,72000"); //Periodic backup mode, 3s to fix, 12s sleep, 18s fix, 72s sleep
      myI2CGPS.sendMTKpacket(configString);
    }
    else if (incoming == '4')
    {
      //Packet 225: Enter Standby or Backup mode for power saving
      //PMTK255, Type, Run time, Sleep time, Second run time, Second sleep time
      //6 types of modes
      
      configString = myI2CGPS.createMTKpacket(225, ",0"); //Go to normal mode
      myI2CGPS.sendMTKpacket(configString);

      delay(1000);
      
      configString = myI2CGPS.createMTKpacket(225, ",9"); //Periodic AlwaysLocate backup mode
      myI2CGPS.sendMTKpacket(configString);
    }

    else if (incoming == '9')
    {
      //Packet 104: Reset to factory defaults
      //Erases all system and user configurations. Does cold start.
      //Note: Unit does not respond with an ACK packet
      //Note: If you're using an external anteanna you must power cycle the module
      //so that it detects the external antenna
      configString = myI2CGPS.createMTKpacket(104, "");
      myI2CGPS.sendMTKpacket(configString);

      Serial.println(F("Reset command sent. No ACK is returned for this command."));
    }
    else
    {
      printMenu();
    }
  }

  while (myI2CGPS.available()) //available() returns the number of new bytes available from the GPS module
  {
    gps.encode(myI2CGPS.read()); //Feed the GPS parser with a new .read() byte
  }

  if (gps.time.isUpdated()) //Check to see if new GPS info is available
  {
    displayInfo();
  }

  //Check to see if we got a response from any command we recently sent
  if (configureCmd.isUpdated())
  {
    Serial.print("Packet ");
    Serial.print(configureCmd.value());
    Serial.print(": ");

    switch (configureFlag.value()[0])
    {
      case '0':
        Serial.print(F("Invalid command"));
        break;
      case '1':
        Serial.print(F("Unsupported command"));
        break;
      case '2':
        Serial.print(F("Action failed"));
        break;
      case '3':
        Serial.print(F("Command successful"));
        break;
    }

    Serial.println();
  }

  delay(10);
}

void printMenu(void)
{
  Serial.println();
  Serial.println(F("1) Set update rate to 10Hz"));
  Serial.println(F("2) Go to normal mode"));
  Serial.println(F("3) Go to periodic backup mode"));
  Serial.println(F("4) Go to AlwaysLocate backup mode"));
  Serial.println(F("9) Reset module"));
}

void displayInfo()
{
  //We have new GPS data to deal with!

  if (gps.time.hour() < 10) Serial.print(F("0"));
  Serial.print(gps.time.hour());
  Serial.print(F(":"));
  if (gps.time.minute() < 10) Serial.print(F("0"));
  Serial.print(gps.time.minute());
  Serial.print(F(":"));
  if (gps.time.second() < 10) Serial.print(F("0"));
  Serial.print(gps.time.second());

  if (gps.location.isValid())
  {
    Serial.print(" Loc:");
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(","));
    Serial.print(gps.location.lng(), 6);

    Serial.print(F(" SIV:"));
    Serial.print(gps.satellites.value());

    Serial.print(F(" HDOP:"));
    Serial.print(gps.hdop.value());
  }

  Serial.println();
}

