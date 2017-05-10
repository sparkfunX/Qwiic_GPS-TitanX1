/*
  Reading Lat/Long from the Qwiic GPS module over I2C
  By: Nathan Seidle
  SparkFun Electronics
  Date: April 12th, 2017
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

  This example shows how to configure the module including:
    Update rate (more please!) (packet# 220)
    Enable/disable sentences (314)
    Pulse-per-second LED (285)
    Set Balloon mode (80km limit) (886)
    Set baud rate (251)
    Reset to factory defaults (104)

  For more information see the MTK NMEA Packet datasheet.

  Note that these settings are stored in battery backed memory and will be retained for
  approximately 16 days without power.

  Hardware Connections:
  Attach the Qwiic Mux Shield to your RedBoard or Uno.
  Plug two Qwiic MMA8452Q breakout boards into ports 0 and 1.
  Serial.print it out at 115200 baud to serial monitor.
*/

#include <Wire.h>

#include "SparkFun_I2C_GPS_Library.h" //Use Library Manager or download here: https://github.com/sparkfun/SparkFun_I2C_GPS_Library

I2CGPS myI2CGPS; //Hook object to the library

#include <TinyGPS++.h> //From: https://github.com/mikalhart/TinyGPSPlus

TinyGPSPlus gps; //Declare gps object

//The following tells the TinyGPS library to scan for the PMTK001 sentence
//This sentence is the response to a configure command from the user
//Field 1 is the packet number, 2 indicates if configuration was successful
TinyGPSCustom configureCmd(gps, "PMTK001", 1); //Packet number
TinyGPSCustom configureFlag(gps, "PMTK001", 2); //Success/fail flag

String configString;

boolean debug = false;

void setup()
{
  Serial.begin(115200);
  Serial.println("GPS Configuration Example");

  Wire.begin();
  Wire.setClock(400000); //Run I2C at fast 400kHz

  //enableDebugging(); //Turn on printing of GPS strings
  myI2CGPS.disableDebugging(); //Turn on printing of GPS strings
  debug = false;

  //The responses to these commands are parsed when we do normal listening and parsing
  //from the GPS module

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
    }
    else if (incoming == '2')
    {
      //Packet 285: Set Pulse per second to occur after 3D fix, and blink for 250ms
      configString = myI2CGPS.createMTKpacket(285, ",2,25");
      myI2CGPS.sendMTKpacket(configString);
    }
    else if (incoming == '3')
    {
      //Packet 314: The SET_NMEA_OUTPUT command is very long
      //20 types of sentences are allowed: GLL / RMC / VTG / GGA / GSA / GSV / GRS / GST
      //The number in the position sets the "output once every X position fixes"
      //Example: ",1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0" = GLL only, output every fix
      //Example: ",0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0" = RMC only, output every fix
      //Example: ",0,0,1,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0" = VTG every fix, GGA every 4th fix
      //Example: ",-1" = Reset to defaults
      configString = myI2CGPS.createMTKpacket(314, ",0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0");
      myI2CGPS.sendMTKpacket(configString);
    }
    else if (incoming == '4')
    {
      //Packet 886: Set navigation mode Normal=0 / Fitness=1 / Aviation=2 / Balloon=3 (80km space limit!)
      //Normal = general purpose
      //Fitness = Running and walking, less than 5m/s movement
      //Aviation = high dynamic large acceleration movement
      //Balloon = For high altitude ballooning! Allows max height to be up to 80km instead of 10km
      configString = myI2CGPS.createMTKpacket(886, ",3");
      myI2CGPS.sendMTKpacket(configString);
    }
    else if (incoming == '5')
    {
      //Packet 251: Set serial baud rate to 57600
      configString = myI2CGPS.createMTKpacket(251, ",57600");
      myI2CGPS.sendMTKpacket(configString);
    }
    else if (incoming == '6')
    {
      //Packet 301: Set DGPS mode
      //0 = No DGPS source
      //1 = RTCM
      //2 = SBAS(Includes WAAS/EGNOS/GAGAN/MSAS)
      configString = myI2CGPS.createMTKpacket(301, ",2");
      myI2CGPS.sendMTKpacket(configString);
    }
    else if (incoming == '8')
    {
      if (debug == false)
      {
        myI2CGPS.enableDebugging();
        debug = true;
      }
      else
      {
        myI2CGPS.disableDebugging();
        debug = false;
      }
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
    }
    else
    {
      printMenu();
    }
  }

  //available() returns the number of new bytes available from the GPS module
  while (myI2CGPS.available())
  {
    gps.encode(myI2CGPS.read()); //Feed the GPS parser with a new .read() byte
  }

  //We have new GPS data to deal with!
  if (gps.location.isValid())
  {
    Serial.print("Loc:");
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(","));
    Serial.print(gps.location.lng(), 6);

    Serial.print(F(" SIV:"));
    Serial.print(gps.satellites.value());

    Serial.print(F(" HDOP:"));
    Serial.print(gps.hdop.value());

    Serial.println();
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
  Serial.println(F("2) Enable PPS LED"));
  Serial.println(F("3) Turn off all sentences but RMC&GGA"));
  Serial.println(F("4) Enable high altitude balloon mode"));
  Serial.println(F("5) Set serial baud rate to 57600bps"));
  Serial.println(F("6) Enable DGPS/SBAS"));
  Serial.println(F("8) Enable/Disable Debugging"));
  Serial.println(F("9) Reset module"));
}

