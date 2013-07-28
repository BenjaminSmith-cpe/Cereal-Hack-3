#include "Wire.h"

//| Buffer to accommodate I2C read
byte dataReceivedBuffer[8];
int accelXData[500];
int accelYData[500];
int accelZData[500];
int xIndex = 0;
int yIndex = 0;
int zIndex = 0;

//| Variables for the elbow position sensor
int elbowSensorReading = 0;

//| Variables for the I2C accelerometer
int16_t accelerometerXData;
int16_t accelerometerYData;
int16_t accelerometerZData;

//| Variables for the I2C accelerometer
int16_t gyroscopeXData;
int16_t gyroscopeYData;
int16_t gyroscopeZData;

//| INITIAL SETUP
//|
//|
//|
void setup() {
  //| Serial connectivity
  Serial.begin(115200);    //| Setup terminal for Bluetooth connection

  Serial.println("Setup beginning");

  analogReadResolution(12);   //| Gotta use that fancy ADC
  pinMode(A0, INPUT);         //| Elbow angle potentiometer 
  pinMode(2,  INPUT);         //| jump to vcc if debug printout is wanted
  pinMode(13,OUTPUT);
  //| I2C Setup
  Wire.begin();

  //| ADXL Accelerometer initialization
  I2CRegisterWrite(0x53, 0x2D, 0x00);         //| reset Power Control register
  I2CRegisterWrite(0x53, 0x2D, 0x16);         //| set Power Control register to sleep
  I2CRegisterWrite(0x53, 0x2D, 0x08);         //| enable active Power Control register
  I2CRegisterWrite(0x53, 0x31, 0b00000100);   //| Data format register
  I2CRegisterWrite(0x53, 0x2C, 0x0F);         //| Data format register
  //I2CRegisterRead(0x53, 0x00, 1, deviceID); //| check to make sure ADXL345 exists
  //Serial.print(deviceID[0], HEX);

  //| Gyro initialization
  I2CRegisterWrite(0x68, 0x15, 0x00);         //| Sample rate divider
  I2CRegisterWrite(0x68, 0x16, 0b11011);      //| two high bits set FS_SEL reg lower three set DLPF_CFG register

}


//| MAIN CONTROL LOOP
//|
//|
//|
void loop(){
  updateAccelerometer();
  updateGyroscope();
  updatePositionSensor(); 

  if(digitalRead(2) == 1){
    sendSensorData(); 
    digitalWrite(13,LOW);
  }else{
    debugSerialPrint(); 
    delay(10);
    digitalWrite(13,HIGH);
  }
}


//| I2C BUS CONTROL
//| This section communicates with the accelerometer and gyroscope through
//| Arduino's Wire library (thx Massimo!)
//|
void updateGyroscope(){
  I2CRegisterRead(0x68, 0x1D, 6, dataReceivedBuffer);

  gyroscopeXData = ((int)dataReceivedBuffer[0] << 8) + dataReceivedBuffer[1];
  gyroscopeZData = ((int)dataReceivedBuffer[2] << 8) + dataReceivedBuffer[3];
  gyroscopeYData = ((int)dataReceivedBuffer[4] << 8) + dataReceivedBuffer[5];
}

//| The update function averages the accelerometer values over 500 datapoints
//| this effectively acts as a low pass filter
void updateAccelerometer(){
  I2CRegisterRead(0x53, 0x32, 6, dataReceivedBuffer);

  accelXData[xIndex] = ((int)dataReceivedBuffer[1] << 8) + dataReceivedBuffer[0];
   //| Data smoothing process
  if(xIndex > 499){
      xIndex = 0;
      accelerometerXData = averageSensorArray(accelXData);
      }else{
        xIndex++;
      }

  accelYData[yIndex] = ((int)dataReceivedBuffer[3] << 8) + dataReceivedBuffer[2];
    if(yIndex > 499){
      yIndex = 0;
      accelerometerYData = averageSensorArray(accelYData);
      }else{
        yIndex++;
      }

  accelZData[zIndex] = ((int)dataReceivedBuffer[5] << 8) + dataReceivedBuffer[4];
    if(zIndex > 499){
      zIndex = 0;
      accelerometerZData = averageSensorArray(accelZData);
      }else{
        zIndex++;
      }
}

void I2CRegisterWrite(int deviceAddress, int registerAdress, byte data){
  Wire.beginTransmission(deviceAddress);
  Wire.write(registerAdress);
  Wire.write(data);
  Wire.endTransmission();
}

void I2CRegisterRead(int deviceAddress, byte registerAddress, int bytesToRead, byte outputData[]) {
  Wire.beginTransmission(deviceAddress);            //start transmission to device 
  Wire.write(registerAddress);                      //sends address to read from
  Wire.endTransmission();                           //end transmission
  
  Wire.beginTransmission(deviceAddress);            //start transmission to device (initiate again)
  Wire.requestFrom(deviceAddress, bytesToRead);     // request 6 bytes from device
  
  int i = 0;
  while(Wire.available())                   //device may send less than requested (abnormal)
  { 
    outputData[i] = Wire.read();            // receive a byte
    i++;
  }
  Wire.endTransmission();                   //end transmission
}


//| POTENTIOMETER POSITION SENSING 
//| This code handles the updates for the analog position sensors
//| it will average the data over 1000 samples and then transmit the result
//| over the Serial bus if the result is more than 1 unit away from the last average
//| (Moved sensor smoothing client side)
// void normalizedAccelData(int Sensor){
//     switch(Sensor){
//         case 1: //read value of elbow sensor
//             elbowSensorReadings[elbowIndexValue] = analogRead(A0);
            
//             //| Data smoothing process
//             if(elbowIndexValue > 999){
//                 elbowIndexValue = 0;
//                 thisSensorValue = averageSensorArray(elbowSensorReadings);
//                 }else{
//                   elbowIndexValue++;
//             }
//         break;
//     }
// }

void updatePositionSensor(){
  elbowSensorReading = analogRead(A0);
}
// int averageSensorArray(int data[]){
//     int total = 0;

//     for(int i = 0; i < 999; i++){
//         total+= data[i];
//     }

//     total = (total / 1000);
//     return total;
// }


//| TRANSMIT SENSOR DATA
//| This code transmits the gathered data in a format agreed upon by 
//| the developers in the protocol specification. This data is destined to
//| be parsed by unity on the PC side
void sendSensorData(){
  Serial.println("start");

  Serial.println(accelerometerXData);
  delay(10);
  Serial.println(accelerometerYData);
  delay(10);
  Serial.println(accelerometerZData);
  delay(10);

  Serial.println(gyroscopeXData);
  delay(10);
  Serial.println(gyroscopeYData);
  delay(10);
  Serial.println(gyroscopeZData);
  delay(10);

  Serial.println(elbowSensorReading);
}


//| DEBUG FUNCTIONS 
//| This code includes a few functions to present sensor data in a human readable
//| format. Grounding digital pin 3 will activate debug mode and illuminate the
//| on board LED assigned to pin 13
//|
void debugPrintPotentiometerData(){
  Serial.print("Elbow position: ");
  Serial.println(elbowSensorReading);
  Serial.println();
}

void debugPrintADXL345RegisterData(){
  Serial.print("Accelerometer Data: ");
  Serial.print(accelerometerXData);
  Serial.print(" - ");
  Serial.print(accelerometerYData);
  Serial.print(" - ");
  Serial.print(accelerometerZData);
  Serial.println();
}

void debugPrintGyroData(){
  Serial.print("Gyroscope Data: ");
  Serial.print(gyroscopeXData);
  Serial.print(" - ");
  Serial.print(gyroscopeZData);
  Serial.print(" - ");
  Serial.print(gyroscopeYData);
  Serial.println();
}

void debugSerialPrint(){
  clearTerminal();
  debugPrintADXL345RegisterData();
  debugPrintGyroData();
  debugPrintPotentiometerData();
}

void clearTerminal(){
  // Serial.write(27);       // ESC command
  // Serial.print("[2J");    // clear screen command
  // Serial.write(27);
  // Serial.print("[H");     // cursor to home command

  //cheater clear for non VT100 compliant terminal emulators
  Serial.println();Serial.println();Serial.println();Serial.println();Serial.println();
  Serial.println();Serial.println();Serial.println();Serial.println();Serial.println();
  Serial.println();Serial.println();Serial.println();Serial.println();Serial.println();
  Serial.println();Serial.println();Serial.println();Serial.println();Serial.println();
  Serial.println();Serial.println();Serial.println();Serial.println();Serial.println();
  Serial.println();Serial.println();Serial.println();Serial.println();Serial.println();
}

int averageSensorArray(int data[]){
  int temp;
  for(int i =0; i < 499; i++){
    temp += data[i];
  }
  temp = temp/500;
  return temp; 
}