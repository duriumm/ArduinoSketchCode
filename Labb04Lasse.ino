int potMeterPin = 0;
int key1Pin = 8;
int key2Pin = 12;
int greenLed = 9;
int blueLed = 10;
int redLed = 11;
int currentActiveLed = 9;

unsigned long previousMillis = 0; 
int lightFadeInterval = 10;
int maxLedVal = 50;
int key1State = LOW;
int key2State = LOW;
int potVal = 0;
int serialPotVal = 0;
int previousPotVal = 0;
int valToChangeGreen = 1;
int valToChangeRed = 1;
int valToChangeBlue = 0;

bool isIncrementing = true;
bool key1IsReady = false;

int button1State; 
int lastButton1State = LOW;
int button2State;
int lastButton2State = LOW;

unsigned long key1LastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long key2LastDebounceTime = 0;
unsigned long debounceDelay = 30;    // the debounce time; increase if the output flickers

bool startFading = false;
bool isFirstTimeFade = true;
bool isSerialPotValActive = false;

long randNumber;

int incomingByte = 0; // for incoming serial data

bool isReadingFromSerial = false;

String textString = "";
char letterReadFromSerialMonitor = 0;

bool isInEnterFadeValueMode = false;
bool isSerialFadeMode = false;

bool isRedLedReadyToFade = false;
bool isGreenLedReadyToFade = false;
bool isBlueLedReadyToFade = false;

bool isButton1ActivatedBySerial = false;


void setup() {
  pinMode(key1Pin, INPUT);
  pinMode(key2Pin, INPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(blueLed, OUTPUT);
  pinMode(redLed, OUTPUT);
  //randomSeed(analogRead(1));
  Serial.begin(9600);
  welcomeAndHelpScreen();
}
void loop() { 
  checkForInput();
  changeToNextLightColor();
  completeFadeMode();
}
void welcomeAndHelpScreen(){
  Serial.println(F("\n\n"));
  Serial.println(F("Hello! This is the help screen. Type in the text string to give your command, when you are ready to send, press ctrl+Enter."));
  Serial.println(F("Change the light color: changelight\nStart fadelight mode: startfade\n"
  "Change fade pulse speed: fadespeed  ,and follow the instructions on screen.\n"
  "Turn lights off: stoplight\nShow this helpscreen again: help\n"));
  
}
void checkForInput(){
  if (Serial.available() > 0) {    // is a character available?
    letterReadFromSerialMonitor = Serial.read();       // get the character
    
    if (letterReadFromSerialMonitor != '\n') {
      // a character of the string was received
      textString += letterReadFromSerialMonitor;
    }
    else {
      // end of string
      /*Serial.print("This is your text: ");
      Serial.println(textString);
      */
      if(textString.equals("startfade")){
        startFading = true;
        Serial.println(F("Fade is ongoing."));
        // EFTER JAG AKTIVERAT FADE GENOM SERIELMONITOR SÅ BLINKAR DEN SOM FAN
      }
      else if(textString.equals("stoplight")){
        startFading = false;
        turnOffAllLightsOnce(); // NÄR MAN SKRIVER STOPFADE SÅ SKA LAMPORNA STÄNGAS OCH INTE BA PAUSAS
        Serial.println(F("Lights stopped."));
      }
      else if(textString.equals("fadespeed") && isInEnterFadeValueMode == false){
        isInEnterFadeValueMode = true;
        Serial.println("Enter a number between 1 and 50 to set fade speed: ");
      }
      else if(isInEnterFadeValueMode == true && textString.toInt() >= 1 && textString.toInt() <= 50){ //JAG TRODDE FLICKERING PROBLEMET LÅG HÄR!!!
          serialPotVal = textString.toInt();   
          Serial.print(F("Fade speed set to: "));    
          Serial.println(serialPotVal);                                                       // det låg INTE här, måste vara i fade?????
          isSerialPotValActive = true;
          isInEnterFadeValueMode = false;         
      }     
      else if(textString.equals("changelight")){
        isButton1ActivatedBySerial = true;
        Serial.print(F("Light changed to: ")); 
        textString = lampNumberToName(currentActiveLed); 
        Serial.println(textString);
      }
      else if(textString.equals("help")){
        welcomeAndHelpScreen();
      }
      // clear the string for reuse
      textString = ""; 
    }
  }
}
String lampNumberToName(int currentActiveLedInput){
  String returnString = "";
  if(currentActiveLedInput == 9){
    returnString = "Green";
  }
  else if(currentActiveLedInput == 10){
    returnString = "Blue";
  }
  else if(currentActiveLedInput == 11){
    returnString = "Red";
  }
  return returnString;
}
void turnOffAllLightsOnce(){
  analogWrite(greenLed, 0);
  analogWrite(redLed, 0);
  analogWrite(blueLed, 0);
}
void completeFadeMode(){
  unsigned long currentMillis = millis(); // not used huh?? remove??
  if(isSerialPotValActive == false){
    potVal = analogRead(potMeterPin);
    potVal = map(potVal, 0, 1023, 50, 1);
  }
  else if (isSerialPotValActive == true) {
    potVal = serialPotVal;
    previousPotVal = analogRead(potMeterPin);
    
    //if(previousPotVal < (analogRead(potMeterPin) - 5) || previousPotVal > (analogRead(potMeterPin) + 5)){ // funkar också?? wtf
    if(abs(analogRead(potMeterPin) - previousPotVal > 5)){
      previousPotVal = analogRead(potMeterPin);
      isSerialPotValActive = false;
    }    
    //Serial.print("analog read potval: ");
    //Serial.println(analogRead(potMeterPin)); 
  }
      // read the state of the switch into a local variable:
  int key2reading = digitalRead(key2Pin);
  if (key2reading != lastButton2State) {
    // reset the debouncing timer
    key2LastDebounceTime = millis();
  }
  if ((millis() - key2LastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:
    // if the button state has changed:
    if(startFading == true){
      
      if(isFirstTimeFade == true){
        turnOffAllLightsOnce();
        isBlueLedReadyToFade = true;
        valToChangeBlue = 2;
        isFirstTimeFade = false;
      }

      // WE COULD MAKE ALL SEPERATE LEDS INTO FUNCTIONS INSTEAD. DO IT WHEN U HAVE TIME!!!

      // BLUE LED STARTING // BLUE LED STARTING // BLUE LED STARTING // BLUE LED STARTING
      if(isBlueLedReadyToFade == true && currentMillis - previousMillis > potVal && valToChangeBlue > 1){
          valToChangeBlue = newFadeLight(blueLed, valToChangeBlue);
          previousMillis = currentMillis;
        }
      else if(isBlueLedReadyToFade == true && valToChangeBlue <= 1){
        valToChangeGreen = 2;
        turnOffLed_1_TurnOnLed_2_(blueLed, greenLed, isBlueLedReadyToFade, isGreenLedReadyToFade);
        isIncrementing = true;
      }   

      // GREEN LED STARTING // GREEN LED STARTING // GREEN LED STARTING // GREEN LED STARTING
      if(isGreenLedReadyToFade == true && currentMillis - previousMillis > potVal && valToChangeGreen > 1){
        valToChangeGreen = newFadeLight(greenLed, valToChangeGreen);
        previousMillis = currentMillis;
        }
      else if(isGreenLedReadyToFade == true && valToChangeGreen <= 1){
        valToChangeRed = 2;
        turnOffLed_1_TurnOnLed_2_(greenLed, redLed, isGreenLedReadyToFade, isRedLedReadyToFade);
        isIncrementing = true;
      }   

      // RED LED STARTING // RED LED STARTING // RED LED STARTING // RED LED STARTING
      if(isRedLedReadyToFade == true && currentMillis - previousMillis > potVal && valToChangeRed > 1){
        valToChangeRed = newFadeLight(redLed, valToChangeRed);
        previousMillis = currentMillis;
        }
      else if(isRedLedReadyToFade == true && valToChangeRed <= 1){
        valToChangeBlue = 2;
        turnOffLed_1_TurnOnLed_2_(redLed, blueLed, isRedLedReadyToFade, isBlueLedReadyToFade);
        isIncrementing = true;
      }   
    } 
    
    if (key2reading != button2State) {
      button2State = key2reading;      
      // only toggle the LED if the new button state is HIGH     
      if (button2State == HIGH) {
        startFading = true;   
      }     
    }
  }
  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastButton2State = key2reading;
}
void turnOffLed_1_TurnOnLed_2_(int ledToTurnOff, int ledToTurnOn, bool &ledToDeactivate, bool &ledToActivate){
  analogWrite(ledToTurnOff, 0);
  ledToDeactivate = false;
  ledToActivate = true;
  analogWrite(ledToTurnOn, 2);
}
void changeToNextLightColor(){
    // read the state of the switch into a local variable:
  int key1reading = digitalRead(key1Pin);
  if (key1reading != lastButton1State) {
    // reset the debouncing timer
    key1LastDebounceTime = millis();
  }
  if ((millis() - key1LastDebounceTime) > debounceDelay || isButton1ActivatedBySerial == true) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:
    // if the button state has changed:
    if (key1reading != button1State || isButton1ActivatedBySerial == true) {
      button1State = key1reading;  
      // only toggle the LED if the new button state is HIGH
      if (button1State == HIGH || isButton1ActivatedBySerial == true) {
        activateOneLedOnly(currentActiveLed); 
        analogWrite(currentActiveLed, maxLedVal);
        startFading = false;
        isFirstTimeFade = true; 
      }
      // whenever you release your button the active led will increment(unless 11 then -> 9)
     if(button1State == LOW || isButton1ActivatedBySerial == true){
        if(currentActiveLed == 11){
          currentActiveLed = 9;
        }
        else{
          currentActiveLed++;
        }
        isButton1ActivatedBySerial = false;       
      }    
    }
  }
  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastButton1State = key1reading;
}
void activateOneLedOnly(int ledToActivate){
  if(ledToActivate == greenLed){
    analogWrite(greenLed, maxLedVal);
    analogWrite(redLed, 0);
    analogWrite(blueLed, 0);
  }
  else if(ledToActivate == blueLed){
    analogWrite(blueLed, maxLedVal);
    analogWrite(redLed, 0);
    analogWrite(greenLed, 0);
  }
  else if(ledToActivate == redLed){
    analogWrite(redLed, maxLedVal);
    analogWrite(blueLed, 0);
    analogWrite(greenLed, 0);
  }
}
int newFadeLight(int ledToFadeInput, int ledValueToChange){
  

    if(ledValueToChange >= maxLedVal){
      isIncrementing = false;
    }
    else if(ledValueToChange <= 1){
      isIncrementing = true;
    }


    if(isIncrementing == true){
      ledValueToChange ++;
        if(ledValueToChange >= maxLedVal){
          ledValueToChange = maxLedVal;
        }
    }
    else if(isIncrementing == false){
      ledValueToChange --;
      if(ledValueToChange <= 1){
        ledValueToChange = 1;
      }
    } 
    analogWrite(ledToFadeInput, ledValueToChange);
    return ledValueToChange; 
}

