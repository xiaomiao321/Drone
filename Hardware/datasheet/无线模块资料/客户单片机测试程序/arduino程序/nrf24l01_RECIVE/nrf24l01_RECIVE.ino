#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
RF24 radio(9, 10); // CE, CSN
const byte addresses [][6] = {"00001", "00002"};   
 //发送地址和接收地址
int button_pin = 2;
boolean button_state = 0;
boolean button_state1 = 0;
int led_pin = 3;
void setup() {
  pinMode(led_pin, OUTPUT);
  Serial.begin(9600);
  radio.begin();                          
  radio.openWritingPipe(addresses[0]);     
  radio.openReadingPipe(1, addresses[1]);  
  radio.setPALevel(RF24_PA_MIN);           
}
void loop() 
{
  delay(5);
  radio.startListening();                    
  //设置为接收端
  if (radio.available())                  
  //等待接收数据
  {
    radio.read(&button_state, sizeof(button_state));
    if(button_state == HIGH)
  {
     digitalWrite(led_pin, HIGH);
  }
  else
  {
     digitalWrite(led_pin, LOW);
  }
  delay(5);
  
  radio.stopListening();                    
   //设置为发送端
  button_state1 = digitalRead(button_pin);
  radio.write(&button_state1, sizeof(button_state1));         //发送数据
  }
}
