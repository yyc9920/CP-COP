#include <SoftwareSerial.h>
#include <OneWire.h>  
#include <DallasTemperature.h>  
    
//DS18B20 온도 센서의 데이터선인 가운데 핀을 아두이노 4번에 연결합니다.   
#define ONE_WIRE_BUS 4  
    
//1-wire 디바이스와 통신하기 위한 준비  
OneWire oneWire(ONE_WIRE_BUS);  
    
// oneWire선언한 것을 sensors 선언시 참조함.  
DallasTemperature sensors(&oneWire);  
    
//다비아스 주소를 저장할 배열 선언  
DeviceAddress insideThermometer; 
const byte rxPin=2;    // rx 핀을2번으로 지정
const byte txPin=3;    // tx 핀을3번으로 지정
SoftwareSerial BTSerial(rxPin,txPin);   //블루투스 시리얼 핀 설정
int command; 
void setup() 
{
  Serial.begin(9600);                    // 9600 속도로 시리얼 통신 시작
  BTSerial.begin(9600);         // 블루투스 통신 속도를 9600으로 설정
  //1-wire 버스 초기화  
  sensors.begin();
  
  //발견한 디바이스 갯수  
  Serial.print("Found ");  
  Serial.print(sensors.getDeviceCount(), DEC);  
  Serial.println(" devices.");  
    
  // parasite power 모드일 때에는  2핀(GND와 DQ 핀)만 연결하면 됨.  
  Serial.print("Parasite power is: ");   
  if (sensors.isParasitePowerMode()) Serial.println("ON");  
  else Serial.println("OFF");  
      
     
  //버스에서 첫번째 장치의 주소를 가져온다.  
  if (!sensors.getAddress(insideThermometer, 0)) Serial.println("Unable to find address for Device 0");   
      
  //버스에서 발견한 첫번째 장치의 주소 출력  
  Serial.print("Device 0 Address: ");  
  printAddress(insideThermometer);  
  Serial.println();  
    
  //데이터시트에서 확인결과 9~12비트까지 설정 가능  
  sensors.setResolution(insideThermometer, 10);  
     
  Serial.print("Device 0 Resolution: ");  
  Serial.print(sensors.getResolution(insideThermometer), DEC);   
  Serial.println();  
}
// 온도를 출력하는 함수  
void printTemperature(DeviceAddress deviceAddress)  
{  
  //섭씨 온도를 가져옴  
  float  tempC = sensors.getTempC(deviceAddress);  
      
  Serial.print("Temp C: ");  
  Serial.print(tempC);  
  Serial.print(" Temp F: ");  
      
  //화씨 온도로 변환  
  Serial.println(DallasTemperature::toFahrenheit(tempC));   
}  
    
//디바이스 주소를 출력하는 함수  
void printAddress(DeviceAddress deviceAddress)  
{  
  for (uint8_t i = 0; i < 8; i++)  
  {  
    if (deviceAddress[i] < 16) Serial.print("0");  
        Serial.print(deviceAddress[i], HEX);  
  }  
}  
    
void loop() {
  Serial.print("Requesting temperatures...");  
  //sensors.requestTemperatures();   //연결되어 있는 전체 센서의 온도 값을 읽어옴  
  sensors.requestTemperaturesByIndex(0); //첫번째 센서의 온도값 읽어옴  
  Serial.println("DONE");  
     
  //센서에서 읽어온 온도를 출력  
  printTemperature(insideThermometer);  
  command = BTSerial.read();//블루투스로 오는 신호
  /*if(command == ???) //블루투스 신호로 특정 신호가 올때
  {
    BTSerial.write(insideTermometer);//보드에서 블루투스로 온도값을 보내줌
  }*/
}
