#include "DS3231.h"
#include "Wire.h"

DS3231 Clock;

byte Year;
byte Month;
byte Date;
byte DoW;
byte Hour;
byte Minute;
byte Second;
int k=10;

bool Century=false;
bool h12;
bool PM;

#define SensorPin A0            //pH meter Analog output to Arduino Analog Input 0
#define Offset 0.00            //deviation compensate 
#define LED 13 
#define samplingInterval 20
#define printInterval 800
#define ArrayLenth  40    //times of collection
int pHArray[ArrayLenth];   //Store the average value of the sensor feedback
int pHArrayIndex=0; 
float pHValue,voltage;
int droptime = 0;

void GetDateStuff(byte& Year, byte& Month, byte& Day, byte& DoW,
        byte& Hour, byte& Minute, byte& Second) {
    // Call this if you notice something coming in on
    // the serial port. The stuff coming in should be in
    // the order YYMMDDwHHMMSS, with an 'x' at the end.
    boolean GotString = false;
    char InChar;
    byte Temp1, Temp2, Temp3;
    char InString[20];
 
    byte j=0;
    while (!GotString) {
        if (Serial.available()) {
            InChar = Serial.read();
            InString[j] = InChar;
            j += 1;
            if (InChar == 'x') {
                GotString = true;
            }
        }
    }
    Serial.println(InString);
    // Read Year first
    Temp1 = (byte)InString[0] -48;
    Temp2 = (byte)InString[1] -48;
    Year = Temp1*10 + Temp2;
    // now month
    Temp1 = (byte)InString[2] -48;
    Temp2 = (byte)InString[3] -48;
    Month = Temp1*10 + Temp2;
    // now date
    Temp1 = (byte)InString[4] -48;
    Temp2 = (byte)InString[5] -48;
    Day = Temp1*10 + Temp2;
    // now Day of Week
    DoW = (byte)InString[6] - 48;      
    // now Hour
    Temp1 = (byte)InString[7] -48;
    Temp2 = (byte)InString[8] -48;
    Hour = Temp1*10 + Temp2;
    // now Minute
    Temp1 = (byte)InString[9] -48;
    Temp2 = (byte)InString[10] -48;
    Minute = Temp1*10 + Temp2;
    // now Second
    Temp1 = (byte)InString[11] -48;
    Temp2 = (byte)InString[12] -48;
    Second = Temp1*10 + Temp2;

    Temp1 = (byte)InString[13] -48;
    Temp2 = (byte)InString[14] -48;
    Temp3 = (byte)InString[15] -48;
    k = Temp1*100 + Temp2*10 + Temp3;
}
 
void setup() {
    // Start the serial port
    Serial.begin(9600);
 
    // Start the I2C interface
    Wire.begin();

    pinMode(LED,OUTPUT);
    Serial.println("pH meter experiment!"); //Test the serial monitor 
}
 
void loop() {
  static unsigned long samplingTime = millis();
  static unsigned long printTime = millis();
    int i;
    // If something is coming in on the serial line, it's
    // a time correction so set the clock accordingly.
    if (Serial.available()) {
        GetDateStuff(Year, Month, Date, DoW, Hour, Minute, Second);
 
        Clock.setClockMode(false);  // set to 24h
        //setClockMode(true);   // set to 12h
 
        Clock.setYear(Year);
        Clock.setMonth(Month);
        Clock.setDate(Date);
        Clock.setDoW(DoW);
        Clock.setHour(Hour);
        Clock.setMinute(Minute);
        Clock.setSecond(Second);
         
        // Give time at next five seconds
        for (i=0; i<k; i++){
            delay(1000);
            Serial.print(Clock.getYear(), DEC);
            Serial.print("-");
            Serial.print(Clock.getMonth(Century), DEC);
            Serial.print("-");
            Serial.print(Clock.getDate(), DEC);
            Serial.print(" ");
            Serial.print(Clock.getHour(h12, PM), DEC); //24-hr
            Serial.print(":");
            Serial.print(Clock.getMinute(), DEC);
            Serial.print(":");
            Serial.println(Clock.getSecond(), DEC);
            if(i == k-1){
              Serial.println("feeding fish");
              delay(1000);
              Serial.println("end");
              i = 0;
            }
        }
    }

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
  
  delay(1000);

  droptime++;

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
