#include <Wire.h>
#include <FastLED.h>
#include <Ewma.h>

#define I2CPin 8
#define I2CButtonPin 7 //I2CPin - 1 

#define WIDTH 30
#define HEIGHT 1
#define NUM_LEDS WIDTH * HEIGHT

#define DATA_PIN 3

CRGB leds[NUM_LEDS];


int pinIndex[5] = {A0,A1,A2,A3,A6} ;
float Calibrations[5];
float CoverCalibrations[5];
char received[10] = {'0','0','0','0','0','0','0','0','0','0'};
Ewma adcFilter1(0.1);
Ewma adcFilter2(0.1);
Ewma adcFilter3(0.1);
Ewma adcFilter4(0.1);
Ewma adcFilter5(0.1);

void FILLARRAY(struct CRGB Colour){
  for (int x = 0; x < 30;x++){
    leds[x] = Colour;
    
  }
  LEDS.show();
}
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

  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  
  pinMode(4, INPUT); digitalWrite(4, HIGH); 
  pinMode(5, INPUT); digitalWrite(5, HIGH); 
  pinMode(6, INPUT); digitalWrite(6, HIGH); 
  pinMode(7, INPUT); digitalWrite(7, HIGH); 
  
}

char bufferArray[15];
bool calibrating = true;

void calibrate(){
  int state = 0;
  while (calibrating == true){
    
    
    int indexer = 0;
    Serial.print("\n");
    Wire.requestFrom(I2CButtonPin, 10);
    
    while (Wire.available() && (indexer <= 9)) { // slave may send less than requested
      received[indexer] = (Wire.read()); // receive a byte as character
      //Serial.print(Wire.read());
      indexer += 1;
    }
  
    if (state == 0){ //Start Calibration
      FILLARRAY(CRGB::Green);
      if (received[0] == 'O'){
       
        Calibrations[0] = takeAverage(A0);
        Calibrations[1] = takeAverage(A1);
        Calibrations[2] = takeAverage(A2);
        Calibrations[3] = takeAverage(A3);
        Calibrations[4] = takeAverage(A6);
        state = 1;
      }
    }
  
    else if (state == 1){ //calibration
       FILLARRAY(CRGB::Red);
       if (received[0] == 'U'){
       
        CoverCalibrations[0] = takeAverage(A0);
        CoverCalibrations[1] = takeAverage(A1);
        CoverCalibrations[2] = takeAverage(A2);
        CoverCalibrations[3] = takeAverage(A3);
        CoverCalibrations[4] = takeAverage(A6);
        state = 2;
      }
    }

    else if (state == 2){
      calibrating = false; 
      FILLARRAY(CRGB::Purple); 
    
    }
  
    delay(90);
    FILLARRAY(CRGB::Black);
    delay(10);
  }
}




void loop() {

  float filtered[5]; 
  filtered[0] = adcFilter1.filter(analogRead(A0));
  filtered[1] = adcFilter2.filter(analogRead(A1));
  filtered[2] = adcFilter3.filter(analogRead(A2));
  filtered[3] = adcFilter4.filter(analogRead(A3));
  filtered[4] = adcFilter5.filter(analogRead(A6));
  
  int indexer = 0;
  Serial.print("\n");
  Wire.requestFrom(I2CButtonPin, 10);
  
  while (Wire.available() && (indexer <= 9)) { // slave may send less than requested
    received[indexer] = (Wire.read()); // receive a byte as character
    //Serial.print(Wire.read());
    indexer += 1;
  }

  //FILLARRAY(CRGB::Black);
  for(int x = 0; x <= 4; x ++){
    if (sState(filtered[x], x) == "1"){
    //if (sState(AVERAGED[x], x) == "1"){
 
      leds[(x+5)*3] = CRGB::Purple;
      leds[(x+5)*3 + 1] = CRGB::Purple;
      leds[(x+5)*3 + 2] = CRGB::Purple;
      
    }
    else{
      leds[(x+5)*3] = CRGB::Red;
      leds[(x+5)*3 + 1] = CRGB::Red;
      leds[(x+5)*3 + 2] = CRGB::Red;
    }
    LEDS.show();
  

  }

  for(int x = 0; x <= 4; x ++){
    if (received[x] == '1'){
 
      leds[x*3] = CRGB::Purple;
      leds[x*3 + 1] = CRGB::Purple;
      leds[x*3 + 2] = CRGB::Purple;
    }
    else{
      leds[x*3] = CRGB::Red;
      leds[x*3 + 1] = CRGB::Red;
      leds[x*3 + 2] = CRGB::Red;
    }
    LEDS.show();
   
  }
  String toSend = (sState(filtered[0], 0) + sState(filtered[1], 1)
  + sState(filtered[2], 2) + sState(filtered[3], 3)  + sState(filtered[4], 4) + String(received[0]) + 
  String(received[1]) + String(received[2]) + String(received[3]) + String(received[4]) + String(received[5]) + 
  String(received[6]) + String(received[7]) + String(received[8])); 

  
  toSend.toCharArray(bufferArray, 15);

  if(received[0] == 'C'){
    calibrating = true;
    calibrate();   
  }
  


}

String sState(int ARead, int i){
  int centre = (CoverCalibrations[i]-Calibrations[i])/2;
  if(ARead > (Calibrations[i] + centre)){
    
    return "1";
  }
  else{
    return "0";  
  }
  
}


// function that executes whenever data is requested by master
// this function is registered as an event, see setup()
void requestEvent() {
 
  
  Wire.write(bufferArray); 

}
