#include <dht11.h>
#include <SFE_BMP180.h>
#include <Wire.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

#include <Ethernet.h>
#include <EthernetClient.h>
#include <Dns.h>
#include <Dhcp.h>


#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "VDilshan"
#define AIO_KEY         "aio_AyXG43aICgDUvVWton4tJNJxKrJP"

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

dht11 DHT11;
SFE_BMP180 pressure;

#define ALTITUDE 441.0 // Altitude of Home(Gampola) in meters

//Set up the ethernet client
EthernetClient client;

Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

// You don't need to change anything below this line!
#define halt(s) { Serial.println(F( s )); while(1);  }

int ambientLight = 0;
double humidity= 0;
double temperature = 0;
int rainfall=0;
double P,p0;

//define Sensors;
int tempIn = 7;

//Define Lights
int greenLED = 6;
int redRGB = 5;
int greenRGB = 4;
int blueRGB = 3;

/****************************** Feeds ***************************************/

// Setup a feed called 'photocell' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish humidityMQT = Adafruit_MQTT_Publish(&mqtt,  AIO_USERNAME "/feeds/Humidity");
Adafruit_MQTT_Publish tempMQT     = Adafruit_MQTT_Publish(&mqtt,  AIO_USERNAME "/feeds/Temprature");
Adafruit_MQTT_Publish pressureMQT = Adafruit_MQTT_Publish(&mqtt,  AIO_USERNAME "/feeds/Barometric Pressure(Current Altitude)");
Adafruit_MQTT_Publish seaLvpressureMQT = Adafruit_MQTT_Publish(&mqtt,  AIO_USERNAME "/feeds/Barometric Pressure (Sea level)");
Adafruit_MQTT_Publish lightMQT = Adafruit_MQTT_Publish(&mqtt,  AIO_USERNAME "/feeds/Ambient Light");
Adafruit_MQTT_Publish rainfallMQT = Adafruit_MQTT_Publish(&mqtt,  AIO_USERNAME "/feeds/Rainfall");

// Setup a feed called 'onoff' for subscribing to changes.
Adafruit_MQTT_Subscribe onoffbutton = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/onoff");

void setup() {
  Serial.begin(115200);
  pinMode(A0, INPUT);       //Rain
  pinMode(A1, INPUT);       //Light
  pinMode(greenLED, OUTPUT);
  pinMode(redRGB, OUTPUT);
  pinMode(greenRGB, OUTPUT);
  pinMode(blueRGB, OUTPUT); 
  pressure.begin();

  // Initialise the Client
  Serial.print(F("\nInit the Client..."));
  Ethernet.begin(mac);
  delay(1000); //give the ethernet a second to initialize
  

  mqtt.subscribe(&onoffbutton);
  delay(1000);
}

uint32_t t=0;
uint32_t h=0;
uint32_t p=0;
uint32_t ps=0;
uint32_t l=0;
uint32_t r=0;

void loop() {
  
  MQTT_connect();
  // this is our 'wait for incoming subscription packets' busy subloop
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(1000))) {
    if (subscription == &onoffbutton) {
      Serial.print(F("Got: "));
      Serial.println((char *)onoffbutton.lastread);
    }
  }
  ambientLight = analogRead(A1); 
  rainfall = analogRead(A0); 
  int chk = DHT11.read(tempIn);
  humidity = (double)DHT11.humidity,2;
  temperature = (double)DHT11.temperature,2;
  Serial.print(temperature);

  P = pressureMeasure();
  p0 = pressure.sealevel(P,ALTITUDE);

  t = (int)temperature;
  if (! tempMQT.publish(t)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }

  h = (int)humidity;
  if (! humidityMQT.publish(h)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }
  p = (int)P;
   if (! pressureMQT.publish(p)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }
  ps= (int)p0;
  if (! seaLvpressureMQT.publish(ps)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }
  r = (int)rainfall;
  if (! rainfallMQT.publish(r)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }
  l = (int)ambientLight;
  if (! lightMQT.publish(l)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }
  
  // ping the server to keep the mqtt connection alive
  if(! mqtt.ping()) {
    mqtt.disconnect();
  }
  delay(12000);
}

void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
  }
  Serial.println("MQTT Connected!");
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
 
//For green light blink
void blinkGreen(){
  digitalWrite(greenLED, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);                       // wait for a second
  digitalWrite(greenLED, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);                       // wait for a second
  }

void RGBled(int type){
  switch (type)
  {
    case 1:
      digitalWrite(redRGB, HIGH);
      break;
    case 2:
      digitalWrite(blueRGB, HIGH);
      break;
    case 3:
      digitalWrite(greenRGB, HIGH);
      break;
    default:
      digitalWrite(blueRGB, LOW);
      digitalWrite(greenRGB, LOW);
      digitalWrite(redRGB, LOW);
      break;
  } 
}
