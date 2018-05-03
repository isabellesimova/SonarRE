// GPIO interrupt method of reading sonar messages
// Better than polling but probably not as good as using the SPI interface
// ESP32 is not 5v tolerant! Use of logic level shifter is required
#include <ArduinoJson.h>

const byte interruptPin = 2;
volatile unsigned long sTime = 0;
volatile unsigned long hTime = 0;
int message[33] = {0};
int tempMsg[33] = {0};
int msgIndex = 0;
// Timing buffer to help ensure we don't miss a pulse if it's a little short.
int timing_buffer = 10;

void setup() {
  Serial.begin(115200);
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), measurePulse, CHANGE);
}

void loop() {
  // Not ideal, messages are coming in faster than they can be read out of memory, this helps a bit.
  memcpy(tempMsg, message, sizeof tempMsg);
  while (tempMsg[32] == 0){
    memcpy(tempMsg, message, sizeof tempMsg);
  }

  // Parse sonar pulse lengths in the copied message to binary.
  for (int i=0; i<33;i++){
      tempMsg[i] = ((tempMsg[i]+timing_buffer)/100) -1;
  }
  
  // Parse binary to int values and send them over serial.
  int dataAval = pulseWidthToInt(tempMsg, 1, 9);
  int dataDval = pulseWidthToInt(tempMsg, 9, 17);
  int dataCval = pulseWidthToInt(tempMsg, 17, 25);
  int dataBval = pulseWidthToInt(tempMsg, 25, 33);
  sendDataViaJSON(dataAval, dataBval, dataCval, dataDval);

  // Delay to show that interrupts are dealing with message reading.
  delay(1000);
}


// ISR to measure high pulse time.
void measurePulse() {
  // Pin is high, "start the timer".
  if(digitalRead(interruptPin)==HIGH){
    sTime = micros();
  }
  // Pin is low, count the lenght and do the appropriate thing with the pulse.
  else {
    hTime = (micros() - sTime);
    // If pulse is not the header, add it to the message.
    if (hTime < 1000) {
      message[msgIndex]=hTime;
      msgIndex++;
    }
    // Pulse was the header, so reset the message.
    else {
      memset(message, 0, sizeof(message));
      msgIndex = 0;
    }
  }
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
