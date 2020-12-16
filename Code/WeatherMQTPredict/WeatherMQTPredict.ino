#include <dht11.h>
#include <SFE_BMP180.h>
#include <Wire.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "RTClib.h"

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
RTC_DS3231 rtc;
SFE_BMP180 pressure;

#define ALTITUDE 485.0 // Altitude of Home(Gampola) in meters

//Set up the ethernet client
EthernetClient client;

Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

#define halt(s) { Serial.println(F( s )); while(1);  }

//define the variables
int ambientLight = 0;
double humidity= 0;
double temperature = 0;
int rainfall=0;
double P,p0;

//define Sensors;
int tempIn = 7;

const int buzzer = 9;

//Define Lights
int greenLED = 8;
int redRGB = 6;
int greenRGB =5;
int blueRGB = 3;

int pressureArray[10]={0};
byte counter=0;
byte delta_time=0;
//int delayTime = 5000;
int rounds=0;
int t_hour=0;
int t_minute=0;

/****************************** Feeds ***************************************/

// Setup a feed called 'photocell' for publishing.
// defining the feeds 
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
  pinMode(buzzer, OUTPUT); 
  pressure.begin();
  RGBled(1);
  // Initialise the Client
  Serial.print(F("\nInit the Client..."));
  Ethernet.begin(mac); 
  delay(1000); //give the ethernet a second to initialize
  mqtt.subscribe(&onoffbutton);
  RGBled(0);
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
  
  if((humidity == 0)|| (temperature == 0)){
    blinkRed(4);
    }
    
//  Serial.print(temperature);
  P = pressureMeasure();
  P = P-13;
  p0 = station2sealevel(P,ALTITUDE,temperature);

  t = (int)temperature;
  if (! tempMQT.publish(t)) {
    Serial.println(F("Failed"));
    blinkRed(2);
  } else {
    Serial.println(F("OK!"));
  }

  h = (int)humidity;
  if (! humidityMQT.publish(h)) {
    Serial.println(F("Failed"));
    blinkRed(2);
  } else {
    Serial.println(F("OK!"));
  }
  p = (int)P;
   if (! pressureMQT.publish(p)) {
    Serial.println(F("Failed"));
    blinkRed(2);
  } else {
    Serial.println(F("OK!"));
  }
  ps= (int)p0;
  if (! seaLvpressureMQT.publish(ps)) {
    Serial.println(F("Failed"));
    blinkRed(2);
  } else {
    Serial.println(F("OK!"));
  }
  r = (int)rainfall;
  if (! rainfallMQT.publish(r)) {
    Serial.println(F("Failed"));
    blinkRed(2);
  } else {
    Serial.println(F("OK!"));
  }
  l = (int)ambientLight;
  if (! lightMQT.publish(l)) {
    Serial.println(F("Failed"));
    blinkRed(2);
  } else {
    Serial.println(F("OK!"));
  }
  blinkGreen();
  // ping the server to keep the mqtt connection alive
  if(! mqtt.ping()) {
    mqtt.disconnect();
    digitalWrite(greenLED, LOW);
  }
  printValues(humidity, temperature, ambientLight, rainfall, P , p0);
  forecast(pressureArray,P);
  rainText(rainfall);
  
//to show the led change
//  testLED(int value);
  
  delay(12000);
  RGBled(0);
}

void blinkRed(int amount){
    for (int i = 0; i < amount; i++){
        RGBled(1);
        delay(500);
        RGBled(0);
      }
  }

void buzzerOne(){
  tone(buzzer, 1000); // Send 1KHz sound signal...
  delay(1000);        // ...for 1 sec
  noTone(buzzer);     // Stop sound...
  delay(1000); 
  }

void testLED(int value){
    int testArray[10]={1000,1001,999,1000,1000,1000,1000,1002,1000,1000};
    forecast(testArray, value);
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
       blinkRed(1);
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
  }
  digitalWrite(greenLED, HIGH);
  RGBled(0);
  Serial.println("MQTT Connected!");
}

void rainText(int rain){
  int range = map(rain, 0, 1024, 0, 3);

  switch (range)
    {
      case 0:
        Serial.println("RAINING");
        buzzerOne();
        buzzerOne();
        buzzerOne();
//      Buzzer goes off 3 times
        break;

      case 1:
        Serial.println("RAIN WARNING");
        buzzerOne();
//      buzzer 1 time
        break;

      case 2:
        Serial.println("NOT RAINING");
        break;
  }
}

void printValues(float humidity, float temp, int light, int rain, double P , double p0){
//  Serial.println();
//  
  Serial.print("Humidity: ");
  Serial.println(humidity);

  Serial.print("Temperature(C): ");
  Serial.print(temp);
  Serial.print(" deg C, ");
  Serial.println("");
//
//  Serial.print("Barometric Pressure: ");
////  printPressure(P, p0);
//
  Serial.print("Ambient Light: ");
  Serial.println(light);
//  
  Serial.print("Rain Fall: ");
  Serial.print(rain);
  Serial.println("");
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
        blinkRed(3);
      }
      blinkRed(3);
    }
    blinkRed(3);
  } 
  blinkRed(3);
}
 
//For green light blink
void blinkGreen(){
  digitalWrite(greenLED, LOW);   // turn the LED on (HIGH is the voltage level)
  delay(500);                       // wait for a second
  digitalWrite(greenLED, HIGH);    // turn the LED off by making the voltage LOW
  delay(500);                       // wait for a second
  }

void RGBled(int type){
  switch (type)
  {
    case 1:
      analogWrite(redRGB, 255); // red
      break;
    case 2:
      analogWrite(blueRGB, 255); //blue
      break;
    case 3:
      analogWrite(greenRGB, 255); //green
      break;
    case 4:
      analogWrite(greenRGB, 255); //yellow
      analogWrite(redRGB, 255);
      break;
    case 5:
      analogWrite(greenRGB, 255); //white
      analogWrite(redRGB, 255);
      analogWrite(blueRGB, 255);
      break;
    case 6:
      analogWrite(redRGB, 170);
      analogWrite(blueRGB, 255); //purple
      break;
    case 7:
      analogWrite(greenRGB, 255); //cyan
      analogWrite(blueRGB, 255);
      break;
    default:
      analogWrite(redRGB, 0);
      analogWrite(blueRGB, 0);
      analogWrite(greenRGB, 0);
      break;
  } 
}

int station2sealevel(int p, int height, int t)
{
  return (double) p*pow(1-0.0065*(double)height/(t+0.0065*(double)height+273.15),-5.275);
}

int myabs(int val){
  if (val<0)
    return -val;
  return val;
  }

int calc_zambretti(int curr_pressure, int prev_pressure, int mon){
        if (curr_pressure<prev_pressure){
          //FALLING
          if (mon>=4 and mon<=9)
          //summer
          {
            if (curr_pressure>=1030)
              return 2;
            else if(curr_pressure>=1020 and curr_pressure<1030)
              return 8;
            else if(curr_pressure>=1010 and curr_pressure<1020)
              return 18;
            else if(curr_pressure>=1000 and curr_pressure<1010)
              return 21;
            else if(curr_pressure>=990 and curr_pressure<1000)
              return 24;
            else if(curr_pressure>=980 and curr_pressure<990)
              return 24;
            else if(curr_pressure>=970 and curr_pressure<980)
              return 26;
            else if(curr_pressure<970)
              return 26;
          }
          else{
          //winter
            if (curr_pressure>=1030)
              return 2;
            else if(curr_pressure>=1020 and curr_pressure<1030)
              return 8;
            else if(curr_pressure>=1010 and curr_pressure<1020)
              return 15;
            else if(curr_pressure>=1000 and curr_pressure<1010)
              return 21;
            else if(curr_pressure>=990 and curr_pressure<1000)
              return 22;
            else if(curr_pressure>=980 and curr_pressure<990)
              return 24;
            else if(curr_pressure>=970 and curr_pressure<980)
              return 26;
            else if(curr_pressure<970)
              return 26;
          }
        }
        else if (curr_pressure>prev_pressure){
          //RAISING
          if (mon>=4 and mon<=9){
            //summer
            if (curr_pressure>=1030)
              return 1;
            else if(curr_pressure>=1020 and curr_pressure<1030)
              return 2;
            else if(curr_pressure>=1010 and curr_pressure<1020)
              return 3;
            else if(curr_pressure>=1000 and curr_pressure<1010)
              return 7;
            else if(curr_pressure>=990 and curr_pressure<1000)
              return 9;
            else if(curr_pressure>=980 and curr_pressure<990)
              return 12;
            else if(curr_pressure>=970 and curr_pressure<980)
              return 17;
            else if(curr_pressure<970)
              return 17;
          }
          else
            //winter
           {
            if (curr_pressure>=1030)
              return 1;
            else if(curr_pressure>=1020 and curr_pressure<1030)
              return 2;
            else if(curr_pressure>=1010 and curr_pressure<1020)
              return 6;
            else if(curr_pressure>=1000 and curr_pressure<1010)
              return 7;
            else if(curr_pressure>=990 and curr_pressure<1000)
              return 10;
            else if(curr_pressure>=980 and curr_pressure<990)
              return 13;
            else if(curr_pressure>=970 and curr_pressure<980)
              return 17;
            else if(curr_pressure<970)
              return 17;
           }
        }
        else{
            if (curr_pressure>=1030)
              return 1;
            else if(curr_pressure>=1020 and curr_pressure<1030)
              return 2;
            else if(curr_pressure>=1010 and curr_pressure<1020)
              return 11;
            else if(curr_pressure>=1000 and curr_pressure<1010)
              return 14;
            else if(curr_pressure>=990 and curr_pressure<1000)
              return 19;
            else if(curr_pressure>=980 and curr_pressure<990)
              return 23;
            else if(curr_pressure>=970 and curr_pressure<980)
              return 24;
            else if(curr_pressure<970)
              return 26;

        }
}

char forecast(int pressureArray[], double p0){
  DateTime now = rtc.now();
  int t_hour2=now.hour();
  int t_minute2=now.minute();
  int Z=0;
//One Minute Intervals
  if (t_hour2!=t_hour or t_minute2!=t_minute){  
      if (counter==10)
      {
        for (int i=0; i<9;i++){
          pressureArray[i]=pressureArray[i+1];
        }
        pressureArray[counter-1]=p0; 
      }
      else{
        pressureArray[counter]=p0;  
        counter++;
      }
     Z=calc_zambretti((pressureArray[9]+pressureArray[8]+pressureArray[7])/3,(pressureArray[0]+pressureArray[1]+pressureArray[2])/3, now.month());
     Serial.println("-----------------------");
     Serial.print("Forecast: ");
     Serial.println(Z);
     
      if (pressureArray[9]>0 and pressureArray[0]>0){
        if (pressureArray[9]+pressureArray[8]+pressureArray[7]-pressureArray[0]-pressureArray[1]-pressureArray[2]>=3){
      //RAISING
        if (Z<3){
          Serial.print("Sunny");
          RGBled(4);
        }
        else if (Z>=3 and Z<=9){
          Serial.print("Sunny Cloudy");
          RGBled(5);
        }
        else if (Z>9 and Z<=17){
           Serial.print("Cloudy");
           RGBled(7);
        }
        else if (Z>17){
          Serial.print("Rainy");
          RGBled(2);
          }
      }

        else if (pressureArray[0]+pressureArray[1]+pressureArray[2]-pressureArray[9]-pressureArray[8]-pressureArray[7]>=3){
      //FALLING
        if (Z<4){
          Serial.print("Sunny");
          RGBled(4);
        }
        else if (Z>=4 and Z<14){
          Serial.print("Sunny Cloudy");
          RGBled(5);
        }
        else if (Z>=14 and Z<19){
          Serial.print("Worsening");
          RGBled(6);
        }
        else if (Z>=19 and Z<21){
          Serial.print("Cloudy");
          RGBled(7);
        }
        else if (Z>=21){
          Serial.print("Rainy");
          RGBled(2);
          }
      }
      else{
       //STEADY
       if (Z<5){
          Serial.print("Sunny");
          RGBled(4);
       }
        else if (Z>=5 and Z<=11){
          Serial.print("Sunny Cloudy");
          RGBled(5);
        }
        else if (Z>11 and Z<14){
          Serial.print("Cloudy");
          RGBled(7);
        }
        else if (Z>=14 and Z<19){
          Serial.print("Worsening");
          RGBled(6);
        }
        else if (Z>19){
          Serial.print("Rainy");
          RGBled(2);
        }
      }
     } 
     else{
      if (p0<1005){
        Serial.print("Rainy");
        RGBled(2);
      }
      else if (p0>=1005 and p0<=1015){
        Serial.print("Cloudy");
        RGBled(7);
      }
      else if (p0>1015 and p0<1025){
        Serial. print("Sunny Cloudy");
        RGBled(5);
      }
      else{
        Serial.print("Rainy");
        RGBled(2);
      }
     }
    Serial.println();
    Serial.println("-----------------------"); 
    t_hour=t_hour2;
    t_minute=t_minute2;
  }
  }
