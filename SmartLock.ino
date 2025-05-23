#include <Servo.h>
#include "SR04.h"
#include <LiquidCrystal.h>
#include <IRremote.h>

// Pin Definitions
#define TRIG_PIN 4
#define ECHO_PIN 2
#define GREEN_LED 5
#define RED_LED 6
#define RECEIVER_PIN 7
#define SERVO_PIN 3
#define BUZZER_PIN A5

// LCD Pins: RS, EN, D4, D5, D6, D7
LiquidCrystal lcd(8, 13, 9, 10, 11, 12);

// IR Remote Button Definitions
#define POWER_BUTTON 0xFFA25D
#define OK_BUTTON 0xFFB04F
#define PLAY_BUTTON 0xFF02FD
#define FUNC_BUTTON 0xFFE21D

// Global Instances
Servo myservo;
SR04 sr04(ECHO_PIN, TRIG_PIN);
IRrecv irrecv(RECEIVER_PIN);
decode_results results;

// System State Definitions
enum SystemState {
  STATE_IDLE,
  STATE_AWAITING_PASSWORD,
  STATE_PASSWORD_SUCCESS,
  STATE_LOCK_CONTROL,
  STATE_PASSWORD_CHANGE,
  STATE_LOCKED_OUT
};
SystemState currentState = STATE_IDLE;

// Globals
bool powerIsOn = true;
bool lockOn = false;
String enteredCode = "";
String correctCode = "1234";
int failedAttempts = 0;

void setup() {
  lcd.begin(16, 2);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  Serial.begin(9600);
  myservo.attach(SERVO_PIN);
  irrecv.enableIRIn();
}

int mapIRtoDigit(unsigned long code) {
  switch (code) {
    case 0xFF6897: return 0;
    case 0xFF30CF: return 1;
    case 0xFF18E7: return 2;
    case 0xFF7A85: return 3;
    case 0xFF10EF: return 4;
    case 0xFF38C7: return 5;
    case 0xFF5AA5: return 6;
    case 0xFF42BD: return 7;
    case 0xFF4AB5: return 8;
    case 0xFF52AD: return 9;
    default: return -1;
  }
}

void beep(int times, int duration) {
  for (int i = 0; i < times; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(duration);
    digitalWrite(BUZZER_PIN, LOW);
    delay(duration);
  }
}

void handleTooManyAttempts() {
  currentState = STATE_LOCKED_OUT;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Too many tries");
  for (int i = 10; i > 0; i--) {
    lcd.setCursor(0, 1);
    lcd.print("Wait: ");
    lcd.print(i);
    delay(1000);
  }
  failedAttempts = 0;
  enteredCode = "";
  currentState = STATE_IDLE;
  lcd.clear();
}

void lockControl() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(lockOn ? "Unlocking..." : "Locking...");
  int start = lockOn ? 180 : 0;
  int end = lockOn ? 0 : 180;
  int step = lockOn ? -1 : 1;
  for (int pos = start; pos != end + step; pos += step) {
    myservo.write(pos);
    delay(15);
  }
  lockOn = !lockOn;
  delay(1000);
  lcd.clear();
}

void promptPasswordChange() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("New PW via Serial");
  Serial.println("Enter new 4-digit password:");
  while (true) {
    if (Serial.available()) {
      String newCode = Serial.readStringUntil('\n');
      newCode.trim();
      if (newCode.length() == 4) {
        correctCode = newCode;
        lcd.clear();
        lcd.print("Password Updated");
        Serial.println("Password updated.");
        delay(2000);
        currentState = STATE_IDLE;
        return;
      } else {
        Serial.println("Invalid. 4 digits only.");
      }
    }
  }
}

void loop() {
  if (powerIsOn && currentState == STATE_IDLE && sr04.Distance() < 50) {
    currentState = STATE_AWAITING_PASSWORD;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Enter Password:");
  }

  if (irrecv.decode(&results)) {
    unsigned long code = results.value;
    irrecv.resume();

    if (code == POWER_BUTTON) {
      powerIsOn = !powerIsOn;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(powerIsOn ? "Power ON" : "Power OFF");
      currentState = powerIsOn ? STATE_IDLE : STATE_IDLE;
      return;
    }

    if (!powerIsOn) return;

    switch (currentState) {
      case STATE_AWAITING_PASSWORD:
        if (code == OK_BUTTON) {
          if (enteredCode == correctCode) {
            lcd.clear();
            lcd.print("Correct!");
            beep(2, 200);
            digitalWrite(GREEN_LED, HIGH);
            delay(500);
            digitalWrite(GREEN_LED, LOW);
            currentState = STATE_PASSWORD_SUCCESS;
            lcd.clear();
            lcd.print("PLAY: Lock");
            lcd.setCursor(0, 1);
            lcd.print("FUNC: Change PW");
          } else {
            lcd.clear();
            lcd.print("Wrong PW");
            delay(1000);
            lcd.clear();
            lcd.print("Try again!");
            beep(1, 500);
            digitalWrite(RED_LED, HIGH);
            delay(500);
            digitalWrite(RED_LED, LOW);
            failedAttempts++;
            if (failedAttempts >= 5) handleTooManyAttempts();
            else currentState = STATE_AWAITING_PASSWORD;
          }
          enteredCode = "";
        } else {
          int digit = mapIRtoDigit(code);
          if (digit != -1 && enteredCode.length() < 4) {
            enteredCode += String(digit);
            lcd.setCursor(0, 1);
            lcd.print(enteredCode);
          }
        }
        break;

      case STATE_PASSWORD_SUCCESS:
        if (code == PLAY_BUTTON) {
          currentState = STATE_LOCK_CONTROL;
          lockControl();
          currentState = STATE_IDLE;
        } else if (code == FUNC_BUTTON) {
          currentState = STATE_PASSWORD_CHANGE;
        }
        break;

      case STATE_PASSWORD_CHANGE:
        promptPasswordChange();
        break;

      case STATE_LOCKED_OUT:
        break;

      case STATE_IDLE:
        break;
    }
  }
}
