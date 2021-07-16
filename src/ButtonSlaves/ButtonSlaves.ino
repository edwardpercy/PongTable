#include <Wire.h>
#define I2CPin 7
#include <Ewma.h>

float Calibrations[5];
float CoverCalibrations[5];

Ewma adcFilter1(0.1);
Ewma adcFilter2(0.1);
Ewma adcFilter3(0.1);
Ewma adcFilter4(0.1);
Ewma adcFilter5(0.1);

float takeAverage(int PINNo){
  float total = 0;
  for (int x=0; x < 10; x ++){


    
    total = total + analogRead(PINNo);
    delay(20);

  }  

  return total/10;     
}


void setup() {
  Serial.begin(9600);
  Wire.begin(I2CPin);                // join i2c bus with address #8
  Wire.onRequest(requestEvent); // register event
  pinMode(4, INPUT); digitalWrite(4, HIGH); 
  pinMode(5, INPUT); digitalWrite(5, HIGH); 
  pinMode(6, INPUT); digitalWrite(6, HIGH); 
  pinMode(7, INPUT); digitalWrite(7, HIGH); 
}


bool sentFlag = true;
char bufferArray[10];
bool calibrating = true;

void calibrate(){
  int state = -1;
  while(calibrating == true){
    int Button1 = digitalRead(4);
    int Button2 = digitalRead(5);
    int Button3 = digitalRead(6);
    int Button4 = digitalRead(7);
    
    if (state == -1){
      for (int x = 0; x <= 2000; x++){
        delay(1);
        }
       state = 0;
    }

    
    else if(state == 0){
       if (Button1 == 0 or Button2 == 0 or Button3 == 0 or Button4 == 0){
          String toSend = 'O' + sState(analogRead(A1), 1) +
              sState(analogRead(A2), 2) +  sState(analogRead(A3), 3) + sState(analogRead(A6), 4) + 
              String(Button1) + String(Button2) + String(Button3) + 
             String(Button4);
      
        
          toSend.toCharArray(bufferArray, 10);

          //NO CUP CALIBRATION
          Calibrations[0] = takeAverage(A0);
          Calibrations[1] = takeAverage(A1);
          Calibrations[2] = takeAverage(A2);
          Calibrations[3] = takeAverage(A3);
          Calibrations[4] = takeAverage(A6);
          
          for (int x = 0; x <= 2000; x++){
           delay(1);
          }
          state = 1;
        } 
        
    }
    else if (state == 1){
        if (Button1 == 0 || Button2 == 0 || Button3 == 0 || Button4 == 0){
          String toSend = 'U' + sState(analogRead(A1), 1) +
              sState(analogRead(A2), 2) +  sState(analogRead(A3), 3) + sState(analogRead(A6), 4) + 
              String(Button1) + String(Button2) + String(Button3) + 
             String(Button4);
      
        
          toSend.toCharArray(bufferArray, 10);

          //CUP CALIBRATION
          CoverCalibrations[0] = takeAverage(A0);
          CoverCalibrations[1] = takeAverage(A1);
          CoverCalibrations[2] = takeAverage(A2);
          CoverCalibrations[3] = takeAverage(A3);
          CoverCalibrations[4] = takeAverage(A6);
          
          for (int x = 0; x <= 2000; x++){
            delay(1);
          }
          state = 2;
        } 
    }
    else if (state == 2){
      calibrating = false;  
      state = -1;
    }    
    delay(100);  
  }
}


void loop() {

  //Serial.print(analogRead(A0));
  float filtered1 = adcFilter1.filter(analogRead(A0));
  delay(10);
  float filtered2 = adcFilter2.filter(analogRead(A1));
  delay(10);
  float filtered3 = adcFilter3.filter(analogRead(A2));
  delay(10);
  float filtered4 = adcFilter4.filter(analogRead(A3));
  delay(10);
  float filtered5 = adcFilter5.filter(analogRead(A6));
  
  //Serial.print(",");
  Serial.print(filtered1);
  Serial.print(",");
  Serial.print(CoverCalibrations[0]);
  Serial.print(",");
  Serial.print(Calibrations[0]);
  Serial.print(",");
  Serial.println((((CoverCalibrations[0]-Calibrations[0])/2)*1.5) + Calibrations[0]);
  
  

  int Button1 = digitalRead(4);
  int Button2 = digitalRead(5);
  int Button3 = digitalRead(6);
  int Button4 = digitalRead(7);

  if (sentFlag == true){
    String toSend = sState(filtered1, 0)+ sState(filtered2, 1) +
          sState(filtered3, 2) +  sState(filtered4, 3) + sState(filtered5, 4) + 
          String(Button1) + String(Button2) + String(Button3) + 
         String(Button4);
  
    
    toSend.toCharArray(bufferArray, 10);
    sentFlag = false;
  }

  if (Button1 == 0 && Button2 == 0 && Button3 == 0 && Button4 == 0){
    delay(3000);
    Button1 = digitalRead(4);
    Button2 = digitalRead(5);
    Button3 = digitalRead(6);
    Button4 = digitalRead(7);
    if (Button1 == 0 && Button2 == 0 && Button3 == 0 && Button4 == 0){
       String toSend = 'C' + sState(filtered1, 0)+ sState(filtered2, 1) +
          sState(filtered3, 2) +  sState(filtered4, 3) + sState(filtered5, 4) + 
          String(Button1) + String(Button2) + String(Button3) + 
         String(Button4);
  
    
      toSend.toCharArray(bufferArray, 10);
      sentFlag = false;
      calibrating = true;
      calibrate();
      
    }
  }
  //delay(50);
}



String sState(int ARead, int i){
  float centre = (CoverCalibrations[i]-Calibrations[i])/2;
 
  if(ARead >= (Calibrations[i] + (centre*1.5))){
    return "1";
  }
  else{
    return "0";  
  }
  
}

// function that executes whenever data is requested by master
// this function is registered as an event, see setup()
void requestEvent() {

  sentFlag = true;
  Wire.write(bufferArray); 
  
  
}
