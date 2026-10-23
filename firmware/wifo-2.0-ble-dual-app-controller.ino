// --- Motor driver pins --- 
#define ENA 5 
#define ENB 6 
#define IN1 7 
#define IN2 8 
#define IN3 9 
#define IN4 10 
 
// --- Lights and buzzer pins --- 
#define FRONT_LIGHT 2 
#define BACK_LIGHT 3 
#define REVERSE_BUZZER 4 
 
// --- Bluetooth indicator --- 
#define BT_LED 12 
#define BT_STATE_PIN 13 
 
// --- Indicators --- 
#define FRONT_LEFT_IND A5 
#define FRONT_RIGHT_IND A4 
#define BACK_LEFT_IND A3 
#define BACK_RIGHT_IND A2 
 
// --- State variables --- 
bool frontLightState = false; 
bool backLightManual = false; 
int speedValue = 200; 
 
bool hazardOn = false; 
bool carMoving = false; 
bool rightIndicatorOn = false; 
bool leftIndicatorOn = false; 
 
unsigned long lastIndicatorBlink = 0; 
bool indicatorState = false; 
 
unsigned long lastBlink = 0; 
bool ledState = false; 
 
// Speed table: 0 to 9 → 0% to 100% (0–255 PWM) 
const int SPEED_TABLE[10] = {0, 28, 56, 85, 113, 142, 170, 198, 227, 255}; 
 
void setup() { 
  Serial.begin(9600);  // Bluetooth module on pins 0 (RX) and 1 (TX) 
 
  pinMode(ENA, OUTPUT); 

  pinMode(ENB, OUTPUT); 
  pinMode(IN1, OUTPUT); 
  pinMode(IN2, OUTPUT); 
  pinMode(IN3, OUTPUT); 
  pinMode(IN4, OUTPUT); 
  pinMode(FRONT_LIGHT, OUTPUT); 
  pinMode(BACK_LIGHT, OUTPUT); 
  pinMode(REVERSE_BUZZER, OUTPUT); 
  pinMode(BT_LED, OUTPUT); 
  pinMode(BT_STATE_PIN, INPUT); 
 
  pinMode(FRONT_LEFT_IND, OUTPUT); 
  pinMode(FRONT_RIGHT_IND, OUTPUT); 
  pinMode(BACK_LEFT_IND, OUTPUT); 
  pinMode(BACK_RIGHT_IND, OUTPUT); 
 
  analogWrite(ENA, speedValue); 
  analogWrite(ENB, speedValue); 
 
  stopCar(); 
} 
 
void loop() { 
  handleBluetoothLED(); 
  handleIndicators(); 
 
  if (Serial.available()) { 
    char bt = Serial.read(); 
    handleBluetoothChar(bt); 
  } 
} 
 
// === Bluetooth LED blinking === 
void handleBluetoothLED() { 
  int btConnected = digitalRead(BT_STATE_PIN); 
  if (btConnected == HIGH) { 
    unsigned long currentMillis = millis(); 
    if (currentMillis - lastBlink >= 200) { 
      lastBlink = currentMillis; 
      ledState = !ledState; 
      digitalWrite(BT_LED, ledState ? HIGH : LOW); 
    } 
  } else { 
    digitalWrite(BT_LED, LOW); 
  } 
} 
 
// === Bluetooth command handler (BOTH APPS - NO DUPLICATES) === 

void handleBluetoothChar(char bt) { 
  switch (bt) { 
 
    // ========== MOVEMENT ========== 
    case 'F': forward(); break; 
    case 'B': backward(); break; 
    case 'L': left(); break; 
    case 'R': right(); break; 
    case 'G': forwardLeft(); break; 
    case 'I': forwardRight(); break; 
    case 'H': backwardLeft(); break; 
    case 'J': backwardRight(); break; 
    case 'S': stopCar(); break; 
 
    // ========== LIGHTS ========== 
    case 'W':  
      digitalWrite(FRONT_LIGHT, HIGH);  
      frontLightState = true;  
      break; 
    case 'w':  
      digitalWrite(FRONT_LIGHT, LOW);   
      frontLightState = false;  
      break; 
    case 'U':  
      backLightManual = true;   
      digitalWrite(BACK_LIGHT, HIGH);  
      break; 
    case 'u':  
      backLightManual = false;  
      digitalWrite(BACK_LIGHT, LOW);   
      break; 
 
    // ========== HAZARD ========== 
    case 'X': 
      if (!carMoving) hazardOn = true; 
      break; 
    case 'x': 
      hazardOn = false; 
      allIndicatorsOff(); 
      break; 
 
    // ========== SPEED (0-9 → 0%-100%) ========== 
    case '0': case '1': case '2': case '3': case '4': 
    case '5': case '6': case '7': case '8': case '9': 
      setSpeed(SPEED_TABLE[bt - '0']); 
      break; 
 
    // ========== STOP ALL ========== 

    case 'D': 
      stopCar(); 
      digitalWrite(FRONT_LIGHT, LOW);  frontLightState = false; 
      digitalWrite(BACK_LIGHT, LOW);   backLightManual = false; 
      hazardOn = false; 
      leftIndicatorOn = false; 
      rightIndicatorOn = false; 
      allIndicatorsOff(); 
      break; 
 
    // ========== OLD APP COMPATIBILITY ========== 
    case 'Q': setSpeed(255); break;  // Max speed (same as '9') 
    case 'K': 
      if (!carMoving) { 
        hazardOn = !hazardOn; 
        if (!hazardOn) allIndicatorsOff(); 
      } 
      break; 
 
    // Ignore unknown 
    default: break; 
  } 
} 
 
// === MOVEMENT FUNCTIONS === 
void forward() { 
  carMoving = true; 
  hazardOn = false; 
  leftIndicatorOn = false; 
  rightIndicatorOn = false; 
 
  analogWrite(ENA, speedValue); 
  analogWrite(ENB, speedValue); 
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW); 
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW); 
 
  updateBackLight(false); 
  digitalWrite(REVERSE_BUZZER, LOW); 
  allIndicatorsOff(); 
} 
 
void backward() { 
  carMoving = true; 
  hazardOn = false; 
  leftIndicatorOn = false; 
  rightIndicatorOn = false; 
 
  analogWrite(ENA, speedValue); 

  analogWrite(ENB, speedValue); 
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH); 
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH); 
 
  updateBackLight(true); 
  digitalWrite(REVERSE_BUZZER, HIGH); 
  allIndicatorsOff(); 
} 
 
void left() { 
  carMoving = true; 
  hazardOn = false; 
  rightIndicatorOn = false; 
  leftIndicatorOn = true; 
 
  analogWrite(ENA, speedValue); 
  analogWrite(ENB, speedValue); 
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW); 
  digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH); 
 
  updateBackLight(false); 
  digitalWrite(REVERSE_BUZZER, LOW); 
} 
 
void right() { 
  carMoving = true; 
  hazardOn = false; 
  leftIndicatorOn = false; 
  rightIndicatorOn = true; 
 
  analogWrite(ENA, speedValue); 
  analogWrite(ENB, speedValue); 
  digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH); 
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW); 
 
  updateBackLight(false); 
  digitalWrite(REVERSE_BUZZER, LOW); 
} 
 
void forwardLeft() { 
  carMoving = true; 
  hazardOn = false; 
  leftIndicatorOn = true; 
  rightIndicatorOn = false; 
 
  analogWrite(ENA, speedValue); 
  analogWrite(ENB, speedValue / 2); 
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW); 

  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW); 
 
  updateBackLight(false); 
  digitalWrite(REVERSE_BUZZER, LOW); 
} 
 
void forwardRight() { 
  carMoving = true; 
  hazardOn = false; 
  rightIndicatorOn = true; 
  leftIndicatorOn = false; 
 
  analogWrite(ENA, speedValue / 2); 
  analogWrite(ENB, speedValue); 
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW); 
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW); 
 
  updateBackLight(false); 
  digitalWrite(REVERSE_BUZZER, LOW); 
} 
 
void backwardLeft() { 
  carMoving = true; 
  hazardOn = false; 
  leftIndicatorOn = true; 
  rightIndicatorOn = false; 
 
  analogWrite(ENA, speedValue); 
  analogWrite(ENB, speedValue / 2); 
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH); 
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH); 
 
  updateBackLight(true); 
  digitalWrite(REVERSE_BUZZER, HIGH); 
} 
 
void backwardRight() { 
  carMoving = true; 
  hazardOn = false; 
  rightIndicatorOn = true; 
  leftIndicatorOn = false; 
 
  analogWrite(ENA, speedValue / 2); 
  analogWrite(ENB, speedValue); 
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH); 
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH); 
 
  updateBackLight(true); 

  digitalWrite(REVERSE_BUZZER, HIGH); 
} 
 
void stopCar() { 
  carMoving = false; 
  digitalWrite(IN1, LOW); 
  digitalWrite(IN2, LOW); 
  digitalWrite(IN3, LOW); 
  digitalWrite(IN4, LOW); 
 
  updateBackLight(false); 
  digitalWrite(REVERSE_BUZZER, LOW); 
} 
 
// === LIGHTS === 
void updateBackLight(bool reversing) { 
  digitalWrite(BACK_LIGHT, (reversing || backLightManual) ? HIGH : LOW); 
} 
 
// === SPEED CONTROL === 
void setSpeed(int val) { 
  speedValue = val; 
  analogWrite(ENA, speedValue); 
  analogWrite(ENB, speedValue); 
} 
 
// === INDICATORS === 
void allIndicatorsOff() { 
  digitalWrite(FRONT_LEFT_IND, LOW); 
  digitalWrite(FRONT_RIGHT_IND, LOW); 
  digitalWrite(BACK_LEFT_IND, LOW); 
  digitalWrite(BACK_RIGHT_IND, LOW); 
} 
 
void handleIndicators() { 
  unsigned long currentMillis = millis(); 
  if (currentMillis - lastIndicatorBlink >= 400) { 
    lastIndicatorBlink = currentMillis; 
    indicatorState = !indicatorState; 
 
    if (hazardOn && !carMoving) { 
      digitalWrite(FRONT_LEFT_IND, indicatorState); 
      digitalWrite(FRONT_RIGHT_IND, indicatorState); 
      digitalWrite(BACK_LEFT_IND, indicatorState); 
      digitalWrite(BACK_RIGHT_IND, indicatorState); 
    }  
    else if (leftIndicatorOn) { 
      digitalWrite(FRONT_LEFT_IND, indicatorState); 

      digitalWrite(BACK_LEFT_IND, indicatorState); 
      digitalWrite(FRONT_RIGHT_IND, LOW); 
      digitalWrite(BACK_RIGHT_IND, LOW); 
    } 
    else if (rightIndicatorOn) { 
      digitalWrite(FRONT_RIGHT_IND, indicatorState); 
      digitalWrite(BACK_RIGHT_IND, indicatorState); 
      digitalWrite(FRONT_LEFT_IND, LOW); 
      digitalWrite(BACK_LEFT_IND, LOW); 
    }  
    else { 
      allIndicatorsOff(); 
    } 
  } 
}