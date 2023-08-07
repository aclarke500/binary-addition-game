#include <LiquidCrystal.h>
LiquidCrystal lcd(2, 3, 4, 5, 6, 7);  // LCD pins
// global vars
const int modePin = 8;  // mode - signed or unsigned
int mode = 0;           // 0 is unsgined, 1 is signed
int buttonPins[4] = { 9, 10, 11, 12 };
int bitVector[4] = { 0, 0, 0, 0 };  // set the number to 0
const int length = sizeof(buttonPins) / sizeof(buttonPins[0]);

// math equation
int num1, num2, ans;  // give scope to inputs to math equation -> num1 + num2
String operand;       // + or -
String equation;

int numOfClicks; // amount of button clicks it takes for user to get ans

/* FUNCTIONS, SETUP AND LOOP AT BOTTOM */

/**
* Recursive integer power function
* @param{int} base 
* @param{int} exponent
* @returns{int} result
*/
int power(int base, int exponent) {
  if (exponent == 0) {
    return 1;
  } else if (exponent == 1) {
    return base;
  }
  return base * power(base, exponent - 1);
}

void setMathEquation() {
  int m = random(0, 2);
  if (m == 1) {
    operand = '+';
    setAdditionNumbers();
  } else {
    operand = '-';
    setSubtractionNumbers();
    
  }
  setEquationString();
}

/**
* Updates num1 and num2 for an addition question
*/
void setAdditionNumbers() {
  int upperBound = power(2, length) - 1;
  num1 = random(upperBound);  // 0-15
  int range = upperBound - num1;
  num2 = random(range + 1);
}

/**
* Updates num1 and num2 for subtraction
*/
void setSubtractionNumbers() {
  int lowerBound = power(2, length - 1);  // min for 2s complement
  int upperBound = power(2, length) - 1;  // max for unsigned since user can toggle 2s complement

  ans = random(-1*lowerBound, upperBound);
  num1 = random(-50, 50);
  num2 = num1 - ans;
}

/*
* formats and sets the value of the equation string
*/
void setEquationString() {
  equation = String(num1) + " " + operand + " " + num2;
}

/*
* Gets the correct answer for the equation
*/
int getAnswer() {
  if (operand == "+")
    return num1 + num2;
  else if(operand == "-")
    return num1 - num2;
}


/**
* Converts the bit vector to a binary number. Signed or unsigned toggled by m
*@returns{int} base 10 representaton of the bit vector
*/
int getNumber() {
  int value = 0;

  for (int i = 0; i < length; i++) {
    // if signed int, use twos complement (largest int is negative)
    if (i == length - 1 && mode == 1 && bitVector[i] == 1) {
      value = value - power(2, i);
    } else if (bitVector[i] == 1) {
      value = value + power(2, i);
    }
  }
  return value;
}
/**
* Updates values, returns whether or not anything changed
* @returns{bool} 0 if no changes, 1 if changes
*/
bool updateValues() {
  bool foundChangedValue = false;
  // check the buttons and update bitVector accordingly
  for (int i = 0; i < length; i++) {
    int result = digitalRead(buttonPins[i]);
    if (result) {
      foundChangedValue = true;
      if (bitVector[i] == 1) {
        bitVector[i] = 0;
      } else if (bitVector[i] == 0) {
        bitVector[i] = 1;
      }
    }
  }
  // toggle mode if clicked
  int modeResult = digitalRead(modePin);
  if (modeResult) {
    foundChangedValue = true;
    if (mode == 1) {
      mode = 0;
    } else if (mode == 0) {
      mode = 1;
    }
  }
  return foundChangedValue;
}

/**
* Prints out values, intended to called iff values change
*/
void updateLogger() {
  int result = getNumber();
  Serial.print("Result: ");
  Serial.println(result);

  for (int i = 0; i < length; i++) {
    Serial.print(bitVector[i]);
  }
  Serial.println("");
}

/**
* Logs out relevant data for analytics
*/
void logData(int time){
  String s = String(num1) + "," + String(num2) + "," + ans + "," + operand + "," + time + "," + numOfClicks;
  Serial.println(s);
}

/**
* Resets every value in the bit vector to 0
* i.e bitVector = [0, 0, ... 0]
*/
void clearBits() {
  for (int i = 0; i < length; i++) {
    bitVector[i] = 0;
  }
  mode = 0; // sets signed int to off
}

/**
* Updates the LCD display
*/
void updateDisplay() {
  int result = getNumber();
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print(result);
  lcd.setCursor(0, 0);
  lcd.print(equation);
}

/**
* Updates LCD display with a congratulations message for the user
*/
void congratulateUser(int time) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Correct!");
  lcd.setCursor(0, 1);
  lcd.print("Time: ");
  lcd.print(time);
  lcd.print("ms");
  delay(1000);
}

void setup() {
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("Welcome to");
  lcd.setCursor(0, 1);
  lcd.print("BITZ CLASH!");
  delay(2500);

  lcd.clear();
  Serial.begin(9600);
  // column names for analytics
  String s = "num1,num2,ans,time,numOfClicks";
  Serial.println(s);
}

void loop() {
  setMathEquation();
  updateDisplay();
  numOfClicks = 0;
  long int timeStart = millis();  // as time goes up, it may exceed int max
  while (getNumber() != getAnswer()) {
    bool valuesChanged = updateValues();
    delay(100);
    if (valuesChanged) {
      delay(250);
      numOfClicks++;
      updateDisplay();
    }
  }
  long int timeEnd = millis();  // the time they get the right answer
  int elapsed = timeEnd - timeStart;
  clearBits(); // also disables 2s complement
  congratulateUser(elapsed);
  logData(elapsed);
}