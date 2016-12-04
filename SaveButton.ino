#include <Bounce2.h>

int led_pin = 16;
int switch_pin = 15;
long timeForFastLight = 1000 * 60 * 2;
long timeForLight = 1000 * 60 * 1;
long timeForLongpress = 2000;

long timeSinceSwitch = 0;

Bounce switchButton = Bounce();

void setup() {
  pinMode(switch_pin, INPUT_PULLDOWN);

  pinMode(led_pin, OUTPUT);

  switchButton.attach(switch_pin);
  switchButton.interval(20);

  Serial.begin(57600);
}

int buttonMode = 0;
int totalModes = 2;

long longpressTimer = 0;

int displayCommandSize = 100;
int displayCommands[100];
int displayCommandsSet = 0;
int displayTimeSlice = 10;
bool lockDisplayCommands = false;

bool checkModeChange() {
  if (switchButton.risingEdge()) {
    longpressTimer = millis();
    return false;
  } else if (switchButton.fallingEdge()) {
    if (millis() - longpressTimer > timeForLongpress) {
      int newMode = (buttonMode + 1) % totalModes;
      buttonMode = newMode;
      Serial.print("Changing to mode ");
      Serial.println(buttonMode);

      int slicesPerBlink = displayCommandSize / (newMode + 1);
      int slicesPerBlinkHalf = slicesPerBlink / 2;
      int valuePerHalfSlice = 255 / slicesPerBlinkHalf;
      int offset = 0;

      for(int i = 0 ; i <= newMode ; i++) {
        for(int slice=0 ; slice < slicesPerBlink ; slice++) {
          if(slice < slicesPerBlinkHalf) {
            displayCommands[offset] = valuePerHalfSlice * slice;
          } else {
            displayCommands[offset] = valuePerHalfSlice * (slicesPerBlink - slice);
          }
          offset++;
        }
      }
      displayCommandsSet = offset;
      lockDisplayCommands = true;
      
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

void performSaveButtonLoop() {
  if (switchButton.risingEdge()) {
    timeSinceSwitch = millis();
    Keyboard.set_modifier(0);
    Keyboard.set_key1(KEY_F5);
    Keyboard.send_now();

    delay(25);

    Keyboard.set_modifier(0);
    Keyboard.set_key1(0);
    Keyboard.send_now();
  }

  long time = millis();

  if (time - timeSinceSwitch > timeForFastLight) {
    if (!lockDisplayCommands) {
      int sliceValue = (255 / (displayCommandSize / 2));
      for (int i = 0 ; i < displayCommandSize / 2 ; i++) {
        displayCommands[i] =  sliceValue * i;
      }
      for (int i = displayCommandSize / 2 ; i < displayCommandSize ; i++) {
        displayCommands[i] = sliceValue * (displayCommandSize - i);
      }
      displayCommandsSet = displayCommandSize;
    }
  } else if (time - timeSinceSwitch > timeForLight) {
    long timeOverDeadline = (time - timeSinceSwitch) - timeForLight;
    float percentOver = ((float) timeOverDeadline) / timeForLight;
    int value = min(percentOver * 255, 255);

    if (!lockDisplayCommands) {
      displayCommands[0] = value;
      displayCommandsSet = 1;
    }
  } else {
    if (!lockDisplayCommands) {
      displayCommands[0] = 0;
      displayCommandsSet = 1;
    }
  }
}

int lastDisplayTime = 0;
int lastDisplayCommand = 0;

void updateDisplay() {
  int offset = millis() - lastDisplayTime;

  int increment = offset / displayTimeSlice;

  if (increment > 0) {
    int newDisplayCommand = (lastDisplayCommand + increment) % displayCommandsSet;

    if (newDisplayCommand == 0) {
      lockDisplayCommands = false;
    }

    lastDisplayCommand = newDisplayCommand;

    analogWrite(led_pin, displayCommands[lastDisplayCommand]);

    lastDisplayTime = millis();
  }
}

boolean isClicking = false;
long lastClick = 0;
long clickInterval = 25;

void performMouseClickLoop() {
  if (switchButton.risingEdge()) {
    isClicking = !isClicking;
  }

  if (isClicking) {
    if (!lockDisplayCommands) {
      displayCommands[0] = 255;
      displayCommandsSet = 1;
    }

    long offset = millis() - lastClick;

    Serial.println(offset);

    if (offset > clickInterval) {
      Serial.println("Clicking");
      Mouse.click();
      lastClick = millis();
    }
  } else {
    if (!lockDisplayCommands) {
      displayCommands[0] = 0;
      displayCommandsSet = 1;
    }
  }
}

void loop() {
  bool runLoop = true;
  if (switchButton.update()) {
    if (checkModeChange()) {
      runLoop = false;
    }
  }

  if (runLoop) {
    switch (buttonMode) {
      case 0:
        performSaveButtonLoop();
        break;
      case 1:
        performMouseClickLoop();
        break;
    }
  }

  updateDisplay();
}
