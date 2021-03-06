#include "DS3231.h"
#include "Wire.h"
#include <OneWire.h>  
#include <DallasTemperature.h>  
#define ONE_WIRE_BUS 4 

DS3231 Clock;
//1-wire 디바이스와 통신하기 위한 준비  
OneWire oneWire(ONE_WIRE_BUS);  
    
// oneWire선언한 것을 sensors 선언시 참조함.  
DallasTemperature sensors(&oneWire);  
    
//다비아스 주소를 저장할 배열 선언  
DeviceAddress insideThermometer;  

byte DoW;
byte Hour;
byte Minute;
byte Second;
unsigned long feedtime;

bool Century=false;
bool h12;
bool PM;

#define SensorPin A0            //pH meter Analog output to Arduino Analog Input 0
#define Offset 0.00            //deviation compensate 
#define LED 13 
#define R_feed 7
#define samplingInterval 20
#define printInterval 800
#define ArrayLenth  40    //times of collection 
int pHArray[ArrayLenth];   //Store the average value of the sensor feedback
int pHArrayIndex=0; 
float pHValue,voltage;
int droptime = 0;
bool getDateflag = true;
bool GS_flag = false;
int it = 0;

void GetDateStuff() {
    // Call this if you notice something coming in on
    // the serial port. The stuff coming in should be in
    // the order YYMMDDwHHMMSS, with an 'x' at the end.
    char Char_tmp;
    unsigned long T1, T2, T3;
    char String_tmp[10];
    byte abc=0;
    
    if (Serial.available()) {
    if (Serial.read() == 'a') {
      it = 0;
      getDateflag = false;
    }
    }
    if (!getDateflag) {
      while (!GS_flag){
        if (Serial.available()) {
            Char_tmp = Serial.read();
            String_tmp[abc] = Char_tmp;
            abc += 1;
            if (Char_tmp == 't') {
                GS_flag = true;
                getDateflag = true;
            }
        }
      }
//      Serial.println(String_tmp);
      T1 = (byte)String_tmp[0] -48;
      T2 = (byte)String_tmp[1] -48;
      T3 = (byte)String_tmp[2] -48;
      feedtime = T1*100 + T2*10 + T3;
    }
}

void printTemperature(DeviceAddress deviceAddress)  
{  
  float  tempC = sensors.getTempC(deviceAddress);  
      
  Serial.print("Temp C: ");  
  Serial.print(tempC);  
  Serial.print(" Temp F: ");  
       
  Serial.println(DallasTemperature::toFahrenheit(tempC));   
}  
     
void printAddress(DeviceAddress deviceAddress)  
{  
  for (uint8_t i = 0; i < 8; i++)  
  {  
    if (deviceAddress[i] < 16) Serial.print("0");  
        Serial.print(deviceAddress[i], HEX);  
  }  
}  
 
void setup() {
    // Start the serial port
    Serial.begin(9600);
    sensors.begin();
 
    // Start the I2C interface
    Wire.begin();

    pinMode(LED,OUTPUT);
    pinMode(R_feed,OUTPUT);
    Serial.println("pH meter experiment!"); //Test the serial monitor 
 
  Serial.print(sensors.getDeviceCount(), DEC);   
 
  if (sensors.isParasitePowerMode()) Serial.println("ON");  
  else Serial.println("OFF");  
      
  //버스에서 첫번째 장치의 주소를 가져온다.  
  if (!sensors.getAddress(insideThermometer, 0)) Serial.println("Unable to find address for Device 0");   

  printAddress(insideThermometer);  
    
  //데이터시트에서 확인결과 9~12비트까지 설정 가능  
  sensors.setResolution(insideThermometer, 10);  
  
  Serial.print(sensors.getResolution(insideThermometer), DEC);   
}
 
void loop() {
  static unsigned long samplingTime = millis();
  static unsigned long printTime = millis();

  while(1){
    pHArray[pHArrayIndex++]=analogRead(SensorPin);
   if(pHArrayIndex==ArrayLenth)pHArrayIndex=0;
   voltage = avergearray(pHArray, ArrayLenth)*5.0/1024; 
   pHValue = 3.5*voltage+Offset;
   samplingTime=millis();

   Serial.print("Voltage:");
   Serial.print(voltage,2); 
   Serial.print("    pH value: ");
   Serial.println(pHValue,2);
   digitalWrite(LED,digitalRead(LED)^1);
   printTime=millis();
  
  delay(990);

  droptime++;
  it++;

  if(pHValue >= 8.0){
    if(droptime%5 == 0){  //용액 투여 후 pH 변화를 확인하는 여유 시간 5초를 두고 pH용액을 투여. 
                          //여유시간이 없을 경우 변화를 감지하는 시간 부족으로 필요 이상의 용액을 투여할 수 있으므로
    Serial.println("Dropping pH- solution for 1sec");
    }
  }

  if(pHValue < 6.0){
    if(droptime%5 == 0){
    Serial.println("Dropping pH+ solution for 1sec");
    }
  }
  if(droptime>=10) droptime = 0;

  GetDateStuff();
  delay(10);
  
      if (GS_flag) {
        GS_flag = false;
      }
        if(it == feedtime){
          Serial.println("feeding fish");
          digitalWrite(R_feed, HIGH);
          delay(4785);
          Serial.println("end");
          digitalWrite(R_feed, LOW);
          it = 0;
        }
        Serial.print(it);
        Serial.print(" : ");
        Serial.println(feedtime);
  
  Serial.print("Requesting temperatures...");  
  //sensors.requestTemperatures();   //연결되어 있는 전체 센서의 온도 값을 읽어옴  
  sensors.requestTemperaturesByIndex(0); //첫번째 센서의 온도값 읽어옴  
  Serial.println("DONE");  
     
  //센서에서 읽어온 온도를 출력  
  printTemperature(insideThermometer); 
  }
}

double avergearray(int* arr, int number)
{
  int i;
  int max,min;
  double avg; 
  long amount=0;
  if(number<=0)
  {
    Serial.println("Error number for the array to avraging!/n");
    return 0;
  }
  if(number<5)
  {
    //less than 5, calculated directly statistics
    for(i=0;i<number;i++)
    {
      amount+=arr[i];
    }
    avg = amount/number; 
    return avg;
  }
  else
  {
    if(arr[0]<arr[1])
    {
      min = arr[0];
      max=arr[1];
    }
    else
    {
      min=arr[1];
      max=arr[0];
    }
    for(i=2;i<number;i++)
    {
      if(arr[i]<min)
      {
        amount+=min;        //arr<min
        min=arr[i]; 
      }
      else
      {
        if(arr[i]>max)
        {
           amount+=max;    //arr>max
           max=arr[i]; 
        }
        else
        {
           amount+=arr[i]; //min<=arr<=max
        }
      }
    }
    avg = (double)amount/(number-2);
  }
  return avg;  
}
