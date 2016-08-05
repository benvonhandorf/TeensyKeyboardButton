#include <Bounce.h>

int led_pin = 16;
int switch_pin = 15;
long timeForLight = 1000*60*5;

int timeSinceSwitch = 0;

Bounce switchButton = Bounce(switch_pin, 20);

void setup() {
  // put your setup code here, to run once:
  pinMode(led_pin, OUTPUT);
  pinMode(switch_pin, INPUT_PULLDOWN);

//  Serial.begin(57600);
}

void loop() {
  // put your main code here, to run repeatedly:

  if(switchButton.update()) {
    if(switchButton.risingEdge()) {
      timeSinceSwitch = millis();
      Keyboard.set_modifier(0);
      Keyboard.set_key1(KEY_F5);
      Keyboard.send_now();

      delay(25);

      Keyboard.set_modifier(0);
      Keyboard.set_key1(0);
      Keyboard.send_now();
    }
  }

  long time = millis();

  if(time - timeSinceSwitch > timeForLight) {
    long timeOverDeadline = (time - timeSinceSwitch) - timeForLight;
    float percentOver = ((float) timeOverDeadline) / timeForLight;
    int value = min(percentOver * 255, 255);

//    Serial.print("Setting Value ");
//    Serial.println(value);

    analogWrite(led_pin, value);
  } else {
    analogWrite(led_pin, 0);
  }
}
