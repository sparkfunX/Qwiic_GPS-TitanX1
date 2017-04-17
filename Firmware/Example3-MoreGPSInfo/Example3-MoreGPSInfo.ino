/*
  Reading Lat/Long from the Qwiic GPS module over I2C
  By: Nathan Seidle
  SparkFun Electronics
  Date: April 12th, 2017
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

  This example shows the regular info as well as Altitude, SIV, and HDOP.

  Hardware Connections:
  Attach the Qwiic Mux Shield to your RedBoard or Uno.
  Plug two Qwiic MMA8452Q breakout boards into ports 0 and 1.
  Serial.print it out at 115200 baud to serial monitor.
*/

#include <Wire.h>
#include <TinyGPS++.h> //From: https://github.com/mikalhart/TinyGPSPlus

TinyGPSPlus gps; //Declare gps object

void setup()
{
  Serial.begin(115200);
  Serial.println("GPS Read Example");

  Wire.begin();
  Wire.setClock(400000); //Run I2C at fast 400kHz

  //enableDebugging(); //Turn on printing of GPS strings
  disableDebugging(); //Turn on printing of GPS strings
}

void loop()
{
  //checkGPS() will poll the module to see if there is new GPS data to play with
  if (checkGPS() == true)
  {
    //We have new GPS data to deal with!
    Serial.println();

    Serial.print(F("Date: "));
    Serial.print(gps.date.month());
    Serial.print(F("/"));
    Serial.print(gps.date.day());
    Serial.print(F("/"));
    Serial.print(gps.date.year());

    if (gps.time.isValid())
    {
      Serial.print((" Time: "));
      if (gps.time.hour() < 10) Serial.print(F("0"));
      Serial.print(gps.time.hour());
      Serial.print(F(":"));
      if (gps.time.minute() < 10) Serial.print(F("0"));
      Serial.print(gps.time.minute());
      Serial.print(F(":"));
      if (gps.time.second() < 10) Serial.print(F("0"));
      Serial.print(gps.time.second());
    }

    Serial.println(); //Done printing time

    if (gps.location.isValid())
    {
      Serial.print("Location: ");
      Serial.print(gps.location.lat(), 6);
      Serial.print(F(", "));
      Serial.print(gps.location.lng(), 6);
      Serial.println();
    }
    else
    {
      Serial.print(F("Location not yet valid"));
    }

    if (gps.altitude.isValid())
    {
      Serial.print(F("Altitude Meters: "));
      Serial.print(gps.altitude.meters());
      Serial.print(F(" Feet: "));
      Serial.print(gps.altitude.feet());
    }

    if (gps.satellites.isValid())
    {
      Serial.print(F(" Satellites in View: "));
      Serial.print(gps.satellites.value());
    }

    if (gps.hdop.isValid())
    {
      Serial.print(F(" HDOP:"));
      Serial.print(gps.hdop.value());
    }

    Serial.println(); //Done printing alt, siv, hdop


  }
  else
  {
    //Serial.println(F("GPS not yet valid"));
  }

  delay(500);
}


