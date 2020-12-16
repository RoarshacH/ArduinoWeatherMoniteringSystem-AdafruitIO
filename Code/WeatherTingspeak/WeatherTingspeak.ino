#include "ThingSpeak.h"
#include <Ethernet.h>
#include "secrets.h"
#include <dht11.h>
#include <SFE_BMP180.h>


byte mac[] = SECRET_MAC;

// Set the static IP address to use if the DHCP fails to assign
IPAddress ip(192, 168, 0, 177);
IPAddress myDns(192, 168, 0, 1);
dht11 DHT11;
SFE_BMP180 pressure;

#define ALTITUDE 485.0 // Altitude of Home(Gampola) in meters

EthernetClient client;

unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;

// Initialize our values
int tempIn = 7;
double temperature = 0;
double humidity= 0;
float P;



void setup() {
  Ethernet.init(10);  // Most Arduino Ethernet hardware
  Serial.begin(115200);  //Initialize serial
    
  // start the Ethernet connection:
  Serial.println("Initialize Ethernet with DHCP:");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
      while (true) {
        delay(1); // do nothing, no point running without Ethernet hardware
      }
    }
    if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
    }
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip, myDns);
  } else {
    Serial.print("  DHCP assigned IP ");
    Serial.println(Ethernet.localIP());
  }
  // give the Ethernet shield a second to initialize:
  pressure.begin();
  delay(1000);
  
  ThingSpeak.begin(client);  // Initialize ThingSpeak
}

void loop() {

  int chk = DHT11.read(tempIn);
  humidity = (double)DHT11.humidity,2;
  temperature = (double)DHT11.temperature,2;
  P = pressureMeasure();
  Serial.println(P);
   // set the fields with the values
  ThingSpeak.setField(1, (int)temperature);
  ThingSpeak.setField(2, (int)humidity);
  ThingSpeak.setField(3, P);
  
  // write to the ThingSpeak channel 
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if(x == 200){
    Serial.println("Channel update successful.");
  }
  else{
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }
    
  delay(20000); // Wait 20 seconds to update the channel again
}

double pressureMeasure(){
  char status;
  double P,T;
  status = pressure.startTemperature();  
  if (status != 0)
  {
    // Wait for the measurement to complete:
    delay(status);
    status = pressure.getTemperature(T);
    if (status != 0)
    {
      status = pressure.startPressure(3);
      if (status != 0)
      {
        // Wait for the measurement to complete:
        delay(status);
        status = pressure.getPressure(P,T);
        if (status != 0)
        {
          // return measurement:
          return (P);
        }
      } 
    }
  }   
}
