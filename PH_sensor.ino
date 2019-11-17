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

void setup() {
  pinMode(LED,OUTPUT);
  Serial.begin(9600);
  Serial.println("pH meter experiment!"); //Test the serial monitor 
}

void loop() {
  static unsigned long samplingTime = millis();
  static unsigned long printTime = millis();

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
