#include <dht11.h>
#include <SFE_BMP180.h>
#include <Wire.h>

dht11 DHT11;
SFE_BMP180 pressure;

#define ALTITUDE 441.0 // Altitude of Home(Gampola) in meters

int ambientLight = 0;
float humidity= 0;
float temp = 0;
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

void setup() {
  // put your setup code here, to run once:
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
  ambientLight = analogRead(A1); //  light = A1
  rainfall = analogRead(A0); // rainfall = A2
  int chk = DHT11.read(tempIn);
  humidity = (float)DHT11.humidity,2;
  temp = (float)DHT11.temperature,2;
  
  if ((chk > 0) &&(pressure.begin())){
      digitalWrite(greenLED, HIGH);
      println("GREEN");
    }
  else
  {// Connection Error
    digitalWrite(redRGB, HIGH);
  }
  
 

//  Print Values  
  printValues(humidity,temp, ambientLight, rainfall);
  delay(5000);
  
  
}
//PRinting the Values
void printValues(float humidity, float temp, int light, int rain){
  Serial.println();
  
  Serial.print("Humidity: ");
  Serial.println(humidity);

  Serial.print("Temprature(C): ");
  Serial.print(temp);
  Serial.print(" deg C, ");
  Serial.println("");

  Serial.print("Barometric Pressure: ");
  printPressure((double(temp)));

  Serial.print("Ambient Light: ");
  Serial.println(light);
  
  Serial.print("Rain Fall: ");
  rainText(rain);
  Serial.println("-----------------------");
  Serial.print("Forecast: ");
  Serial.println();
  Serial.println("-----------------------");   
 }
//Void Print Pressure
char printPressure(double T){  
  char status = pressure.getPressure(P,T);
  if (status != 0)
  {
    // Print out the measurement:
    Serial.print("Absolute pressure: ");
    Serial.print(P,2);
    Serial.print(" mb, ");
    Serial.print(P*0.0295333727,2);
    Serial.println(" Hg");

    p0 = station2sealevel(P,ALTITUDE, T);
    Serial.print("Relative (sea-level) pressure: ");
    Serial.print(p0,2);
    Serial.print(" mb, ");
    Serial.print(p0*0.0295333727,2);
    Serial.println(" Hg");
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
      Serial.println("Warning");
      break;
  }
  
  }
//Rain Sting OUT
void rainText(int rain){
  int range = map(rain, 0, 512, 0, 3);

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
