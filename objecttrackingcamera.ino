#include <ServoTimer2.h>
#include <AltSoftSerial.h>

#define PACKAGE_LENGTH 20
#define DEBUG

// Declare the Servo pin 
int servoPinHorizon = 3;
int servoPinVertical = 5;
// Create a servo object 
ServoTimer2 servoHorizon ;
ServoTimer2 servoVertical;
int DELTA_ANGLE = 1;
int INIT_ANGLE = 45;

// AltSoftSerial always uses these pins:
//
// Board          Transmit  Receive   PWM Unusable
// -----          --------  -------   ------------
// Arduino Uno        9         8         10

// Arduino Uno Pin 9 <==> Bluetooth 4.0 UART CC2541 HM-10 RX
// Arduino Uno Pin 8 <==> Bluetooth 4.0 UART CC2541 HM-10 TX
// Arduino Uno Pin VCC <==> Bluetooth 4.0 UART CC2541 HM-10 VCC
// Arduino Uno Pin GND <==> Bluetooth 4.0 UART CC2541 HM-10 GND

AltSoftSerial hmSerial;
int en_pin = 10;
char buf[PACKAGE_LENGTH];
bool receive = false;
int ledState = LOW;
bool tracking = false;
int objCenterX;
int camW;
int objCenterY;
int camH;
int camCenterX;
int camCenterY;

void setup() {
  // put your setup code here, to run once:
  hmSerial.begin(19200);
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);

  for(int i=0 ; i < 20; i++){
    buf[i] = '\0';
  }

   servoHorizon.attach(servoPinHorizon);
   servoVertical.attach(servoPinVertical);
   servoHorizon.write(angle2Value(INIT_ANGLE));
   servoVertical.write(angle2Value(INIT_ANGLE));
}

void loop() {
  if (Serial.available()) {
    String data = Serial.readString();
    sendCommand(data);
  }
  readSerialHM();
  controlServos();
  delay(10);
}

int angle2Value(int angle){
  return map(angle, 0, 90, 750, 1500);
}

void readSerialHM(){
  String reply;
  String strX;
  String strW;
  String strY;
  String strH;
  while (hmSerial.available()) {
    reply = hmSerial.readStringUntil('\n');
    strX = getValue(reply,',',0);
    strW = getValue(reply,',',1);
    strY = getValue(reply,',',2);
    strH = getValue(reply,',',3);
    objCenterX = strX.toInt();
    camW = strW.toInt();
    objCenterY = strY.toInt();
    camH = strH.toInt();
    camCenterX = camW/2;
    camCenterY = camH/2;
    tracking = true;
  }

#ifdef DEBUG
  if(reply.length() > 0){
    if(ledState==LOW){
      digitalWrite(LED_BUILTIN, HIGH);
      ledState = HIGH;
    }else{
      digitalWrite(LED_BUILTIN, LOW);
      ledState = LOW;
    }
    receive = true;
    Serial.println(reply);
    // if(strX.equals("")==false)
    //   Serial.println(objCenterX);
    // if(strW.equals("")==false)
    //   Serial.println(camW);
    // if(strY.equals("")==false)
    //   Serial.println(objCenterY);
    // if(strH.equals("")==false)
    //   Serial.println(camH);
  }
#endif
}

void controlServos(){
  if(tracking==true){
    tracking=false;
    int angleHorizon  = INIT_ANGLE + float((camCenterX-objCenterX)*DELTA_ANGLE)*0.03f;
    if(angleHorizon>90){
      angleHorizon = 90;
    }else if(angleHorizon<0){
      angleHorizon = 0;
    }
    servoHorizon.write(angle2Value(angleHorizon));

    int angleVertical = INIT_ANGLE + float((camCenterY-objCenterY)*DELTA_ANGLE)*0.03f;
    if(angleVertical>90){
      angleVertical = 90;
    }else if(angleVertical<0){
      angleVertical = 0;
    }
    servoVertical.write(angle2Value(angleVertical));
    
    // Serial.print(angleHorizon);
    // Serial.print('\t');
    // Serial.println(angleVertical);
  }
}

void sendCommand(String command){
  if (command.length() > PACKAGE_LENGTH){
    return;
  }
  // Serial.println("sendcommand");
  // Serial.println(command);
  command.toCharArray(buf,command.length());
  buf[command.length()]='\r';
  buf[command.length()+1]='\n';
  hmSerial.println(buf);
  receive = false;
  for(int i=0 ; i < 20; i++){
      buf[i] = '\0';
  }
}

String getValue(String data, char separator, int index)
{
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}