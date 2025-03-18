#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Keypad.h>
#include <MIDIUSB.h>

// ----------------------
// Configuración del OLED
// ----------------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET     -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ----------------------
// Multiplexores
// ----------------------
const uint8_t mpSelPins[4] = {2, 3, 4, 5};
const uint8_t mpAnalogPin = A0;
uint16_t leerCanalMultiplexor(uint8_t canal) {
  for (uint8_t i = 0; i < 4; i++) {
    digitalWrite(mpSelPins[i], bitRead(canal, i));
  }
  delayMicroseconds(5);
  return analogRead(mpAnalogPin);
}

const uint8_t mux2SelPins[4] = {6, 7, 8, 9};
const uint8_t mux2AnalogPin = A1;
uint16_t leerCanalMux2(uint8_t canal) {
  for (uint8_t i = 0; i < 4; i++) {
    digitalWrite(mux2SelPins[i], bitRead(canal, i));
  }
  delayMicroseconds(5);
  return analogRead(mux2AnalogPin);
}

// ----------------------
// Potenciómetros y Joysticks
// ----------------------
const uint8_t numPots = 9;
const uint8_t avgWindow = 10;
uint16_t potBuffer[numPots][avgWindow] = {0};
uint8_t potIndex[numPots] = {0};
uint32_t potSum[numPots] = {0};
uint16_t potPrevVal[numPots] = {0};
const uint16_t potThreshold = 4;
const uint8_t midiCC[numPots] = {7, 1, 74, 71, 91, 93, 10, 11, 76};

const uint8_t numJoystickAxes = 4;
uint16_t joyBuffer[numJoystickAxes][avgWindow] = {0};
uint8_t joyIndex[numJoystickAxes] = {0};
uint32_t joySum[numJoystickAxes] = {0};
uint16_t joyPrevVal[numJoystickAxes] = {0};
const uint16_t joyThreshold = 4;
const uint8_t joystickCC[numJoystickAxes] = {12, 13, 14, 15};

// ----------------------
// Teclado Matricial
// ----------------------
char keys[4][4] = {
  {'0', '1', '2', '3'},
  {'4', '5', '6', '7'},
  {'8', '9', 'A', 'B'},
  {'C', 'D', 'E', 'F'}
};
byte rowPins[4] = {10, 11, 12, 13};
byte colPins[4] = {A2, A3, A4, A5};
Keypad keypad = Keypad((byte*)keys, rowPins, colPins, 4, 4);

// ----------------------
// Configuración para cambio de rango de notas MIDI (DOS BOTONES)
// ----------------------
const uint8_t rangeUpButtonPin = 0;   // D0
const uint8_t rangeDownButtonPin = 1; // D1
uint8_t noteGroup = 0;
const uint8_t maxGroups = 8;

// ----------------------
// Variables para rastrear notas activas
// ----------------------
bool notasActivas[128] = {false};

// ----------------------
// Funciones MIDI
// ----------------------
void sendControlChange(uint8_t control, uint8_t value) {
  midiEventPacket_t event = {0x0B, 0xB0, control, value};
  MidiUSB.sendMIDI(event);
  MidiUSB.flush();
}

void sendNoteOn(uint8_t note, uint8_t velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90, note, velocity};
  MidiUSB.sendMIDI(noteOn);
  MidiUSB.flush();
}

void sendNoteOff(uint8_t note, uint8_t velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80, note, velocity};
  MidiUSB.sendMIDI(noteOff);
  MidiUSB.flush();
}

// ----------------------
// Función promedio móvil
// ----------------------
uint16_t updateMovingAverage(uint8_t index, uint16_t newReading, uint16_t buffer[][avgWindow], uint32_t sumArr[], uint8_t indexArr[]) {
  sumArr[index] = sumArr[index] - buffer[index][indexArr[index]] + newReading;
  buffer[index][indexArr[index]] = newReading;
  indexArr[index] = (indexArr[index] + 1) % avgWindow;
  return sumArr[index] / avgWindow;
}

// ----------------------
// Función actualizar Display
// ----------------------
void actualizarDisplay() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Rango:");
  uint8_t inicio = noteGroup * 16;
  uint8_t fin = inicio + 15;
  display.setTextSize(2);
  display.setCursor(0, 30);
  display.print(inicio);
  display.print("-");
  display.print(fin);
  display.display();
}

// ----------------------
// Función para obtener el número de nota MIDI de la tecla
// ----------------------
uint8_t obtenerNotaMidi(char tecla) {
  if (tecla == '1') return noteGroup * 16 + 0;
  else if (tecla == '2') return noteGroup * 16 + 2;
  else if (tecla == '3') return noteGroup * 16 + 4;
  else if (tecla == '4') return noteGroup * 16 + 5;
  else if (tecla == '5') return noteGroup * 16 + 7;
  else if (tecla == '6') return noteGroup * 16 + 9;
  else if (tecla == '7') return noteGroup * 16 + 11;
  else if (tecla == '8') return noteGroup * 16 + 12;
  else if (tecla == '9') return noteGroup * 16 + 14;
  else if (tecla == '0') return noteGroup * 16 + 16;
  else if (tecla == 'A') return noteGroup * 16 + 1;
  else if (tecla == 'B') return noteGroup * 16 + 3;
  else if (tecla == 'C') return noteGroup * 16 + 6;
  else if (tecla == 'D') return noteGroup * 16 + 8;
  else if (tecla == 'E') return noteGroup * 16 + 10;
  else if (tecla == 'F') return noteGroup * 16 + 13;
  return 0; // Retorna 0 si la tecla no está mapeada
}

// ----------------------
// Setup
// ----------------------
void setup() {
  // Inicializar pines de los multiplexores
  for (uint8_t i = 0; i < 4; i++) {
    pinMode(mpSelPins[i], OUTPUT);
    pinMode(mux2SelPins[i], OUTPUT);
  }

  // Inicializar botones de cambio de rango con pull-up interna
  pinMode(rangeUpButtonPin, INPUT_PULLUP);
  pinMode(rangeDownButtonPin, INPUT_PULLUP);

  // Inicialización del display OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    for (;;) {}
  }
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Rango:");
  display.display();

  // Inicializar buffers
  for (uint8_t i = 0; i < numPots; i++) {
    uint16_t initVal = leerCanalMultiplexor(i);
    for (uint8_t j = 0; j < avgWindow; j++) {
      potBuffer[i][j] = initVal;
    }
    potSum[i] = initVal * avgWindow;
    potPrevVal[i] = initVal;
  }
  for (uint8_t i = 0; i < numJoystickAxes; i++) {
    uint16_t initJoy = leerCanalMux2(i);
    for (uint8_t j = 0; j < avgWindow; j++) {
      joyBuffer[i][j] = initJoy;
    }
    joySum[i] = initJoy * avgWindow;
    joyPrevVal[i] = initJoy;
  }
}

// ----------------------
// Loop principal
// ----------------------
void loop() {
  // Lectura de potenciómetros
  for (uint8_t i = 0; i < numPots; i++) {
    uint16_t raw = leerCanalMultiplexor(i);
    uint16_t avg = updateMovingAverage(i, raw, potBuffer, potSum, potIndex);
    if (abs((int)avg - (int)potPrevVal[i]) > potThreshold) {
      potPrevVal[i] = avg;
      uint8_t midiVal = map(avg, 0, 1023, 0, 127);
      sendControlChange(midiCC[i], midiVal);
    }
  }

  // Lectura de joysticks
  for (uint8_t i = 0; i < numJoystickAxes; i++) {
    uint16_t raw = leerCanalMux2(i);
    uint16_t avg = updateMovingAverage(i, raw, joyBuffer, joySum, joyIndex);
    if (abs((int)avg - (int)joyPrevVal[i]) > joyThreshold) {
      potPrevVal[i] = avg;
      uint8_t midiVal = map(avg, 0, 1023, 0, 127);
      sendControlChange(joystickCC[i], midiVal);
    }
  }

  // Teclado Matricial
  KeypadEvent* keyEvents = keypad.getKeys();
  if (keyEvents) {
    for (int i = 0; i < LIST_MAX; i++) {
      if (keypad.key[i].stateChanged) {
        char tecla = keypad.key[i].kchar;
        uint8_t nota = obtenerNotaMidi(tecla);

        if (nota != 0) {
          switch (keypad.key[i].state) {
            case PRESSED:
              if (!notasActivas[nota]) {
                sendNoteOn(nota, 100);
                notasActivas[nota] = true;
              }
              break;
            case RELEASED:
              if (notasActivas[nota]) {
                sendNoteOff(nota, 0);
                notasActivas[nota] = false;
              }
              break;
          }
        }
      }
    }
  }

  // Cambio de rango de notas MIDI (DOS BOTONES)
  if (digitalRead(rangeUpButtonPin) == LOW) {
    delay(50);
    if (digitalRead(rangeUpButtonPin) == LOW) {
      noteGroup = (noteGroup + 1) % maxGroups;
      actualizarDisplay();
      delay(200);
      while (digitalRead(rangeUpButtonPin) == LOW) delay(10);
    }
  }

  if (digitalRead(rangeDownButtonPin) == LOW) {
    delay(50);
    if (digitalRead(rangeDownButtonPin) == LOW) {
      if (noteGroup > 0) {
        noteGroup--;
        actualizarDisplay();
        delay(200);
        while (digitalRead(rangeDownButtonPin) == LOW) delay(10);
      }
    }
  }

  delay(10);
}