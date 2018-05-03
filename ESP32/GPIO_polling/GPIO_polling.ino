// GPIO polling method of reading sonar messages
// This code is highly timing dependant, not really recommended
// ESP32 is not 5v tolerant! Use of logic level shifter is required
#include <ArduinoJson.h>

const byte sonarInputPin = 2;
volatile unsigned long highPulseWidth = 0;
volatile unsigned long highPulseStart = 0;
bool highPulseActive = false;
int message[33] = {0};
int msgIndex = 0;
// Timing buffer to help ensure we don't miss a pulse if it's a little short.
int timing_buffer = 10;

void setup() {
  Serial.begin(115200);
  pinMode(sonarInputPin, INPUT);
}

void loop() {
  if(digitalRead(sonarInputPin)==HIGH){
    if(!highPulseActive){
      highPulseStart = micros();
      highPulseActive = true;
    }
  } else if(highPulseActive) {
    highPulseActive = false;
    
    // Parse sonar pulse lengths in the copied message to binary.
    highPulseWidth = (((micros() - highPulseStart) + timing_buffer) / 100) - 1;
    
    // Header(end) detected, deal with message
    if(highPulseWidth > 10) {
      // Parse pulse widths to data values and send them over serial
      int dataAval = pulseWidthToInt(message, 1, 9);
      int dataDval = pulseWidthToInt(message, 9, 17);
      int dataCval = pulseWidthToInt(message, 17, 25);
      int dataBval = pulseWidthToInt(message, 25, 33);
      sendDataViaJSON(dataAval, dataBval, dataCval, dataDval);

      // Reset sonar message
      memset(message, 0, sizeof(message));
      msgIndex = 0;
    }
    else { // Not a header/end pulse width, just store the value
      message[msgIndex]=highPulseWidth;
      msgIndex++;
    }
  }

  // Uncomment to see how timing dependant this loop is (code will not work)
  //delay(100);
}


// Take string based binary array, slice it, and convert to long
int pulseWidthToInt(int msg[], int startIndex, int endIndex) {
  String tempData = "";
  for (int i=startIndex;i<endIndex;i++) {
    tempData.concat(msg[i]);
  }
  return strtol( tempData.c_str(), NULL, 2 );
}

void sendDataViaJSON(int dataA, int dataB, int dataC, int dataD) {
  // Pack data into a json object and send it over serial
  StaticJsonBuffer<75> jsonBuffer;
  JsonObject& sonarData = jsonBuffer.createObject();
  sonarData["A"] = dataA;
  sonarData["B"] = dataB;
  sonarData["C"] = dataC;
  sonarData["D"] = dataD;
  sonarData.printTo(Serial);
  Serial.println("");
}

