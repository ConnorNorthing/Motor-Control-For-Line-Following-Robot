//#include <PID_v1.h>

//Variables for serial
bool canSend = false;
byte serialBuffer[64] = {0};
int serialIndex = -1;
float Analog;
bool QTP2_130_bool=false;
bool QTP2_212_bool=false;
bool QTP2_001_bool=false;
bool QTP4_212_bool=false;
bool QTP5_211_bool=false;
float LeftMotorSpeed,RightMotorSpeed=0;
float RightMotorPWM,LeftMotorPWM=0;
float RightMotorPWM_Persentage,LeftMotorPWM_Persentage=0;
int RightMotorPWM_Persentage_Val,LeftMotorPWM_Persentage_Val=0;
int RightMotorPWM_Persentage_Dec,LeftMotorPWM_Persentage_Dec=0;
int LeftMotorSpeed_Val,RightMotorSpeed_Val=0;
int LeftMotorSpeed_Dec,RightMotorSpeed_Dec=0;
float Max_speed=0;
double RPM=0;
int RPMfinal_Val=0;
int RPMfinal_Deci=0;
int Offset = 1;

//Battery Persentage
float analogValue=0;
float input_voltage=0;
float input_analog=0;
float input_voltage_Persentage=0;
int input_voltage_Persentage_Val=0;
int input_voltage_Persentage_Deci=0;
int analogPin=0;

//Pins
int Reverse = 5;
int Forward = 6;

//161:
int beta = 0;
int beta2 = 0;
int beta3=0;

//RPM variables
volatile unsigned long ticks = 0;
volatile unsigned long prevTime = 0;
volatile int RPMfinal = 0;

void setup() {
   //Serial
   Serial.begin(19200);
   //Pins
   pinMode(Forward, OUTPUT);
   //Interrupt for motor RPM
   attachInterrupt(digitalPinToInterrupt(3), ISR_CountTicksM1, RISING);
   pinMode(A0,INPUT);
   pinMode(12,OUTPUT);
   pinMode(11,OUTPUT);
   pinMode(10,OUTPUT);
   pinMode(9,OUTPUT);

   for(int i=255; i<=0; i--){
    analogWrite(Forward, i);
    delay(30);
   }
   for(int i=0; i<=255; i++){
    analogWrite(Forward, i);
    delay(30);
   }
   
   analogWrite(Reverse, 0);
   for(int i=0; i<=255; i++){
    analogWrite(Reverse, i);
    delay(30);
   }
   
}

void loop() {
  // put your main code here, to run repeatedly:

  //SERIAL
  checkComms();
  Max_speed=40.0;
  if(QTP2_130_bool)
  {
//   analogWrite(Forward, 0);
//   delay(5000);
   Serial.write(96);
   Serial.write(20);
   Serial.write(20);
   Serial.write(0);
//   analogWrite(Forward, 255);
   BatteryPersentage ();
   Serial.write(97);
   Serial.write((int)input_voltage_Persentage_Val);
   Serial.write(0);
   Serial.write((int)input_voltage_Persentage_Deci);
    
    QTP2_130_bool=false;
  }

  if(QTP2_212_bool)
  {
    BatteryPersentage ();
    //Control byte
    Serial.write(161);
    Serial.write(0);
    Serial.write(0);
    Serial.write(0);
   //Control byte
    Serial.write(162);
    Serial.write(0);
    Serial.write(0);
    Serial.write(0);
    Serial.write(163);
   //First Data Byte
    Serial.write(0);
    Serial.write(0);
    Serial.write(0);
    QTP2_212_bool=false;
  }

  if(QTP2_001_bool)
  {
    analogWrite(Forward, 255);
    QTP2_001_bool=false;
  }

    if(QTP4_212_bool)
  {
    RightMotorPWM=255-((RightMotorSpeed/Max_speed)*255.0);
    analogWrite(Forward, RightMotorPWM);
    delay(2000);
    
    RightMotorPWM_Persentage=(RightMotorPWM/255.0)*100;
    RightMotorPWM_Persentage_Val=RightMotorPWM_Persentage;
    RightMotorPWM_Persentage_Dec=(RightMotorPWM_Persentage-RightMotorPWM_Persentage_Val)*100;
    LeftMotorPWM_Persentage=RightMotorPWM_Persentage;
    LeftMotorPWM_Persentage_Val=RightMotorPWM_Persentage_Val;
    LeftMotorPWM_Persentage_Dec=RightMotorPWM_Persentage_Dec;
    LeftMotorPWM=RightMotorPWM;
    
    BatteryPersentage ();
    Serial.write(161);
    Serial.write((int)input_voltage_Persentage_Val);
    Serial.write(0);
    Serial.write((int)input_voltage_Persentage_Deci);
    Serial.write(162);
    Serial.write(100-RightMotorPWM_Persentage_Val);
    Serial.write(100-LeftMotorPWM_Persentage_Val);
    Serial.write(RightMotorPWM_Persentage_Dec);
    Serial.write(163);
    Serial.write((int)RPMfinal_Val);
    Serial.write((int)RPMfinal_Val);
    Serial.write((int)RPMfinal_Deci);
    QTP4_212_bool=false;
  }

  if(QTP5_211_bool){
    //Reverse
   for(int i=0; i<=255; i++){
    analogWrite(Forward, 255); 
   }
   analogWrite(Reverse,128);
   delay(2000);
   for(int i=128; i<=255; i++){ 
    analogWrite(Reverse, 255);
    delay(4);
   }
   //Output
   Serial.write(228);
   Serial.write(0);
   Serial.write(0);
   Serial.write(0);

   QTP5_211_bool = false;
  }
}

void checkComms(){
  //Bytes check
  if (Serial.available()>0){
    int lastRead = 0;
    int number = Serial.available();
    byte readbytes[64] = {0};
    Serial.readBytes(readbytes,number);

    for (int i = 0; i < number; i++){
      serialBuffer[++serialIndex] = readbytes[i];
    }

    for (int i = 0; i < serialIndex; i++){
      //Lees:
      if ((serialBuffer[i] == 112) && (i+3 <= serialIndex)){
        //Ons het n 112 gekry:
        QTP2_130_bool=true;
        lastRead = i+3;
      }

      if ((serialBuffer[i] == 146) && (i+3 <= serialIndex)){
        //Ons het n 146 gekry:
        if(serialBuffer[i+1]==0&&serialBuffer[i+2]==0){
        QTP2_212_bool=true;
        lastRead = i+3;
        }
        else{
          QTP4_212_bool=true;
          LeftMotorSpeed=serialBuffer[i+1];
          RightMotorSpeed=serialBuffer[i+2];
          lastRead = i+3;
        }
      }
         if ((serialBuffer[i] == 145) && (i+3 <= serialIndex)){
        //Ons het n 146 gekry:
        if(serialBuffer[i+1]==1){
        QTP5_211_bool=true;
        lastRead = i+3;
        }
      }

      if ((serialBuffer[i] == 1) && (i+3 <= serialIndex)){
        //Ons het n 162 gekry:
        QTP2_001_bool=true;
        lastRead = i+3;
      }
   }

    if ((lastRead) == 0){return;}
    
    lastRead++;
    int prevSerialIndex = serialIndex;
    serialIndex = -1;
    for (int i = lastRead; i <= prevSerialIndex; i++){
      serialBuffer[++serialIndex] = serialBuffer[i];
    }
  }
}

void BatteryPersentage (){
   analogValue = analogRead (A0);   
   //Serial.write(analogValue);
   //analogValue = 200;

   input_analog = ((analogValue) / 1024.0);
   input_voltage=((input_analog*5)+0.8);
   if(input_voltage==0){
    input_voltage_Persentage=0;
   }
   input_voltage_Persentage =(((input_voltage)/5.0)*100);
   input_voltage_Persentage_Val = input_voltage_Persentage ;
   input_voltage_Persentage_Deci= (input_voltage_Persentage-input_voltage_Persentage_Val)*100;

   if (input_voltage <= 5.0 && input_voltage > 3.75 ){
    digitalWrite(9, HIGH);
    digitalWrite(12,HIGH);
    digitalWrite(11,HIGH);
    digitalWrite(10,HIGH);
   }
   else if (input_voltage <= 3.75 && input_voltage > 2.5 ){
    digitalWrite(12, HIGH);
    digitalWrite(9,LOW);
    digitalWrite(11,HIGH);
    digitalWrite(10,HIGH);
   }
   else if (input_voltage <= 2.5 && input_voltage > 1.25 ){
    digitalWrite(11, HIGH);
    digitalWrite(9,LOW);
    digitalWrite(12,LOW);
    digitalWrite(10,HIGH);
   }
   else {
    digitalWrite(10, HIGH);
    digitalWrite(9,LOW);
    digitalWrite(12,LOW);
    digitalWrite(11,LOW);
  }
}

//Interrupt Service Routine
void ISR_CountTicksM1() { //interrupt service routine to count the right encoder ticks
  ticks++;

  if (ticks >= 2){
    unsigned long currTime = micros();
    double Time = (currTime-prevTime);
    RPM = ((((2.0/Time)*(60.0/975)*(1000000.0))));
    RPMfinal_Val = (RPM + Offset);
    RPMfinal_Deci= abs((RPM-RPMfinal_Val)*100);
    
    prevTime = currTime;
    ticks=0;
  }
}
