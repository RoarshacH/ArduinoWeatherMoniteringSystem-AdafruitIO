#include <dht11.h>
#include <SFE_BMP180.h>
#include <Wire.h>
#include "RTClib.h"

dht11 DHT11;
RTC_DS3231 rtc;
SFE_BMP180 pressure;

#define ALTITUDE 441.0 // Altitude of Home(Gampola) in meters

int ambientLight = 0;
double humidity= 0;
double temperature = 0;
int rainfall=0;
double P,p0,a;

//define Sensors;
int tempIn = 7;
// light = A1
// rainfall = A0

//Define Lights
int greenLED = 6;
int redRGB = 5;
int greenRGB = 4;
int blueRGB = 3;

int pressureArray[10]={0};
byte counter=0;
byte delta_time=0;
int delayTime = 5000;
int t_hour=0;
int t_minute=0;

void setup() {
  Serial.begin(9600);
  pinMode(A0, INPUT);       //Rain
  pinMode(A1, INPUT);       //Light
  pinMode(greenLED, OUTPUT);
  pinMode(redRGB, OUTPUT);
  pinMode(greenRGB, OUTPUT);
  pinMode(blueRGB, OUTPUT); 
  pressure.begin();

  delay(200);
}

void loop() {
  // record the values
  io.run(); //for the MQT transfer
  ambientLight = analogRead(A1); //  light = A1
  rainfall = analogRead(A0); // rainfall = A0
  int chk = DHT11.read(tempIn);
  humidity = (double)DHT11.humidity,2;
  temperature = (double)DHT11.temperature,2;

  if ((chk > 0) &&(pressure.begin())){
      digitalWrite(greenLED, HIGH);
      Serial.println("GREEN");
    }
  else
  {// Connection Error
    digitalWrite(redRGB, HIGH);
  } 
//  for the pressure sensor
  P = pressureMeasure();
  p0 = station2sealevel(P,ALTITUDE, temperature);
  
//  Print Values  
  printValues(humidity,temperature, ambientLight, rainfall,P , p0, pressureArray);
  delay(delayTime);
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
        else RGBled((int)1);
      } 
      else RGBled((int)1);
    }
    else RGBled((int)1);
  }
  else RGBled((int)1);
  
  
  }
//PRinting the Values
void printValues(float humidity, float temp, int light, int rain, double P , double p0,int pressureArray[]){
  Serial.println();
  
  Serial.print("Humidity: ");
  Serial.println(humidity);

  Serial.print("Temperature(C): ");
  Serial.print(temp);
  Serial.print(" deg C, ");
  Serial.println("");

  Serial.print("Barometric Pressure: ");
  printPressure(P, p0);

  Serial.print("Ambient Light: ");
  Serial.println(light);
  
  Serial.print("Rain Fall: ");
  Serial.print(rain);
  rainText(rain);
  
//  forecast(pressureArray,p0);

  Serial.println("");
 }

 
//Void Print Pressure
char printPressure(double P,double p0){   
  // Print out the measurement:
  Serial.print("Absolute pressure: ");
  Serial.print(P,2);
  Serial.print(" mb, ");
  Serial.print(P*0.0295333727,2);
  Serial.println(" Hg");

  Serial.print("Relative (sea-level) pressure: ");
  Serial.print(p0,2);
  Serial.print(" mb, ");
  Serial.print(p0*0.0295333727,2);
  Serial.println(" Hg");
//    
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
      Serial.println("Warning");
      break;
  }
  
  }
//Rain Sting OUT
void rainText(int rain){
  int range = map(rain, 0, 255, 0, 3);

  switch (range)
    {
      case 0:
        Serial.println("RAINING");
        break;

      case 1:
        Serial.println("RAIN WARNING");
        break;

      case 2:
        Serial.println("NOT RAINING");
        break;
  }
}
  
//sea level calculation for zambretti

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

char forecast(int pressureArray[], double p0 ){
  DateTime now = rtc.now();
  int t_hour2=now.hour();
  int t_minute2=now.minute();
  int Z=0;
//One Minute Intervals
  if (t_hour2!=t_hour or t_minute2!=t_minute){
    delta_time++;
    if (delta_time>10){
      delta_time=0;
      
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
    }
     Z=calc_zambretti((pressureArray[9]+pressureArray[8]+pressureArray[7])/3,(pressureArray[0]+pressureArray[1]+pressureArray[2])/3, now.month());
     Serial.println("-----------------------");
     Serial.print("Forecast: ");
     Serial.println("Z Value"+ Z);
//     Serial.println("Date"+ (char)now.day()+ " " +(char)now.month()+" "+(char)now.year());
//     Serial.println("Time"+ (char)now.hour()+":" +(char)now.minute());
     
      if (pressureArray[9]>0 and pressureArray[0]>0){
        if (pressureArray[9]+pressureArray[8]+pressureArray[7]-pressureArray[0]-pressureArray[1]-pressureArray[2]>=3){
      //RAISING
        if (Z<3){
          Serial.print("Sunny");
        }
        else if (Z>=3 and Z<=9){
          Serial.print("Sunny Cloudy");
        }
        else if (Z>9 and Z<=17)
          Serial.print("Cloudy");
        else if (Z>17){
          Serial.print("Rainy");
          }
      }

        else if (pressureArray[0]+pressureArray[1]+pressureArray[2]-pressureArray[9]-pressureArray[8]-pressureArray[7]>=3){
      //FALLING
        if (Z<4)
          Serial.print("Sunny");
        else if (Z>=4 and Z<14){
          Serial.print("Sunny Cloudy");
        }
        else if (Z>=14 and Z<19){
          Serial.print("Worsening");
        }
        else if (Z>=19 and Z<21)
          Serial.print("Cloudy");
        else if (Z>=21){
          Serial.print("Rainy");
          }
      }
      else{
       //STEADY
       if (Z<5)
          Serial.print("Sunny");
        else if (Z>=5 and Z<=11){
          Serial.print("Sunny Cloudy");
        }
        else if (Z>11 and Z<14)
          Serial.print("Cloudy");
        else if (Z>=14 and Z<19){
          Serial.print("Worsening");
        }
        else if (Z>19){
          Serial.print("Rainy");
        }
      }
     } 
     else{
      if (p0<1005)
        Serial.print("Rainy");
      else if (p0>=1005 and p0<=1015)
        Serial.print("Cloudy");
      else if (p0>1015 and p0<1025)
        Serial. print("Sunny Cloudy");
      else
        Serial.print("Rainy");
     }
    Serial.println();
    Serial.println("-----------------------"); 
    t_hour=t_hour2;
    t_minute=t_minute2;
  }
  }
