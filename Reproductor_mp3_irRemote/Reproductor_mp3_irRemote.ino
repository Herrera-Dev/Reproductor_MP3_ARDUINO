
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

byte gvol[8] = { B00010, B00110, B11110, B11110, B11110, B00110, B00010, B0000};
byte grepetir[8] = {B00010, B11111, B10010, B10000, B00001, B01001, B11111, B01000};
byte galeatoriot[8] = {B00011, B00001, B10100, B01000, B10100, B00001, B00011, B00000};
byte galeatoriof[8] = {B00000, B00000, B00010, B11111, B00010, B00000, B00000, B00000};
byte gpause[8] = {B10010, B10010, B10010, B10010, B10010, B10010, B10010, B00000};
byte gplay[8] = {B10000, B11000, B11100, B11110, B11100, B11000, B10000, B00000};

#include <IRremote.h>
#define irPrevious 0xF708FF00
#define irPlay_pause 0xE31CFF00
#define irNext 0xA55AFF00
#define irVolumenUp 0xE718FF00
#define irVolumenDown 0xAD52FF00
#define irRepetir 0xE916FF00 // El *
#define irAleatorio 0xF20DFF00 // El #
#define irFuente 0xE619FF00 // El 0 
#define irFolder1 0xBA45FF00 // 1 
#define irFolder2 0xB946FF00 // 2
#define irFolder3 0xB847FF00 // 3

// DFPLAYER MINI MP3 - PARLANTE
#include <SoftwareSerial.h>
#include "DFRobotDFPlayerMini.h"
SoftwareSerial mySoftwareSerial(10, 11); // RX, TX
DFRobotDFPlayerMini myDFPlayer;

#define control 2
#define led 3

int vVolumen = 20;
int pistaActual;
boolean staInicio = true;
boolean staPause = true;

String fuente = "USB";
boolean aleatorio = true;
boolean repetir = false;
int folder = 1;

void setup() {
  Serial.begin(9600);
  while (!Serial);
  IrReceiver.begin(control, DISABLE_LED_FEEDBACK);

  pinMode(led, OUTPUT);

  lcd.init();
  lcd.backlight();
  lcd.createChar(0, galeatoriot);
  lcd.createChar(1, galeatoriof);
  lcd.createChar(2, grepetir);
  lcd.createChar(3, gvol);
  lcd.createChar(4, gpause);
  lcd.createChar(5, gplay);
  lcd.begin(16, 2);

  mySoftwareSerial.begin(9600);
  if (!myDFPlayer.begin(mySoftwareSerial)) {  //Use softwareSerial to communicate with mp3.
    Serial.println(F("Incapaz de empezar"));
    Serial.println(F("1. ¡Vuelva a comprobar la conexión!"));
    Serial.println(F("2. ¡Inserte la tarjeta SD!"));
    while (true) {
      digitalWrite(led, HIGH);
      delay(1000);
      digitalWrite(led, LOW);
      delay(1000);
    }
  }

  lcd.clear();      // limpia pantalla
  lcd.setCursor(1, 0);
  lcd.print("HERRERA MUSIC");
  lcd.display();
  delay(2000);

  myDFPlayer.setTimeOut(500);
  myDFPlayer.outputDevice(DFPLAYER_DEVICE_FLASH);
}

void loop() {

  Serial.println(F("-------------------"));

  if (myDFPlayer.available()) {
    printDetail(myDFPlayer.readType(), myDFPlayer.read());
    if (myDFPlayer.readType() > 6) {
      staPause = true;
    }
    digitalWrite(led, HIGH);
  } else {
    digitalWrite(led, LOW);
  }

  //---------------
  if (staInicio) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Presionar Play..");
    lcd.display();
    delay(500);
  }

  if (IrReceiver.decode()) {
    //---------------
    Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX);

    // INICIO
    if (IrReceiver.decodedIRData.decodedRawData == irPlay_pause && staInicio) {
      randomSeed(analogRead(A0));
      staInicio = false;
      myDFPlayer.volume(vVolumen); //0 - 30

      if (aleatorio) {
        pistaActual = fileAleatorio();
        myDFPlayer.playFolder(folder, pistaActual);;

      } else {
        pistaActual = 0;
        myDFPlayer.loopFolder(folder);
      }
    }

    // PLAY Y PAUSE
    if (IrReceiver.decodedIRData.decodedRawData == irPlay_pause && !staInicio) {
      if (staPause) {  // Esta en pause entonces hay que reanudar.
        staPause = false;
        myDFPlayer.start();

      } else {
        staPause = true;
        myDFPlayer.pause();
      }
    }

    // PREVIOUS
    if (IrReceiver.decodedIRData.decodedRawData == irPrevious && !staInicio) {

      repetir = false;
      staPause = false;

      if (pistaActual == 1) {
        pistaActual += myDFPlayer.readFileCountsInFolder(folder);
      }
      pistaActual -= 1;
      myDFPlayer.playFolder(folder, pistaActual);
    }

    // NEXT
    if (IrReceiver.decodedIRData.decodedRawData == irNext && !staInicio) {
      repetir = false;
      staPause = false;

      if (aleatorio) {
        pistaActual = fileAleatorio();
        myDFPlayer.playFolder(folder, pistaActual);

      } else {
        if (pistaActual == myDFPlayer.readFileCountsInFolder(folder)) {
          pistaActual = 0;
        }
        pistaActual += 1;
        myDFPlayer.playFolder(folder, pistaActual);
      }
    }


    // VOLUMEN
    if (IrReceiver.decodedIRData.decodedRawData == irVolumenUp && !staInicio)
    {
      if (vVolumen < 30 ) {
        vVolumen += 1;
        myDFPlayer.volume(vVolumen);
      }
    }

    if (IrReceiver.decodedIRData.decodedRawData == irVolumenDown && !staInicio)
    {
      if (vVolumen > 0 ) {
        vVolumen -= 1;
        myDFPlayer.volume(vVolumen);
      }
    }

    // REP. LOOP
    if (IrReceiver.decodedIRData.decodedRawData == irRepetir && !staInicio)
    {
      if (repetir) {
        repetir = false;
      } else {
        repetir = true;
      }
    }
    // REP. ALETORIO
    if (IrReceiver.decodedIRData.decodedRawData == irAleatorio && !staInicio)
    {
      if (aleatorio) {
        aleatorio = false;
      } else {
        aleatorio = true;
      }
    }

    // FUENTE
    if (IrReceiver.decodedIRData.decodedRawData == irFuente && !staInicio)
    {
      myDFPlayer.reset();
      if (fuente == "USB") {
        myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);
        fuente = "SD";
        staPause = true;
      } else {
        myDFPlayer.outputDevice(DFPLAYER_DEVICE_FLASH);
        fuente = "USB";
        staPause = true;
      }
      staInicio = true;
    }

    // FUENTE
    if (IrReceiver.decodedIRData.decodedRawData == irFolder1 && !staInicio)
    {
      folder = 1;
    }
    if (IrReceiver.decodedIRData.decodedRawData == irFolder2 && !staInicio)
    {
      folder = 2;
    }
    if (IrReceiver.decodedIRData.decodedRawData == irFolder3 && !staInicio)
    {
      folder = 3;
    }


    // MOSTRAR PANTALLA
    if (!staInicio) {

      lcd.clear();
      mostrarPantalla();
      lcd.display();
    }

    IrReceiver.resume();
  }
  delay (500);
}

// FUNCIONES ================================================

// AUTO NEXT
void autoNext() {
  if (repetir) {
    myDFPlayer.playFolder(folder, pistaActual);
  } else {
    if (aleatorio) {
      pistaActual = fileAleatorio();
      myDFPlayer.playFolder(folder, pistaActual);

    } else {
      if (pistaActual == myDFPlayer.readFileCountsInFolder(folder)) {
        pistaActual = 0;
      }
      pistaActual += 1;
      myDFPlayer.playFolder(folder, pistaActual);
    }
  }
}


void mostrar(int col, int fila, int dat) {
  lcd.setCursor(col, fila); // ubica cursor en inicio de coordenadas X,Y
  lcd.print(dat);
}

void mostrar(int col, int fila, String dat) {
  lcd.setCursor( col, fila); // ubica cursor en inicio de coordenadas X,Y
  lcd.print(dat);
}

// ----------------------------
void mostrarPantalla()
{
  mostrar(0, 0, "Track:");
  mostrar(6, 0, pistaActual);

  if (staPause && !staInicio) {
    lcd.setCursor(15, 0);
    lcd.write(byte(5));
  } else {
    lcd.setCursor(15, 0);
    lcd.write(byte(4));
  }

  mostrar(9, 1, "D:");
  mostrar(11, 1, folder);
  mostrar(4, 1, fuente);

  lcd.setCursor(0, 1);
  lcd.write(byte(3));
  mostrar(1, 1, vVolumen);

  if (aleatorio) {
    lcd.setCursor(15, 1);
    lcd.write(byte(0));
  }
  else {
    lcd.setCursor(15, 1);
    lcd.write(byte(1));
  }

  if (repetir) {
    lcd.setCursor(14, 1);
    lcd.write(byte(2));
  }
}

// ----------------------------
int fileAleatorio() {
  return  random(1, myDFPlayer.readFileCountsInFolder(folder));
}

// ----------------------------
void printDetail(uint8_t type, int value) {
  switch (type) {
    case TimeOut:
      Serial.println(F("¡Se acabó el tiempo!"));
      break;
    case WrongStack:
      Serial.println(F("¡Pila mal!"));
      break;
    case DFPlayerCardInserted:
      Serial.println(F("¡Tarjeta insertada!"));
      staPause = true;
      break;
    case DFPlayerCardRemoved:
      Serial.println(F("¡Tarjeta removida!"));
      if (fuente == "SD") {
        myDFPlayer.outputDevice(DFPLAYER_DEVICE_FLASH);
        myDFPlayer.volume(vVolumen);
        fuente = "USB";
        staInicio = true;
        staPause = true;
      }
      break;
    case DFPlayerCardOnline:
      Serial.println(F("¡Tarjeta en linea!"));
      staPause = true;
      break;
    case DFPlayerPlayFinished:
      Serial.println(F("¡Reproducion terminada!"));
      autoNext();
      break;
    case DFPlayerError:
      Serial.print(F("DFPlayerError:"));
      switch (value) {
        case Busy:
          Serial.println(F("Tarjeta no encontrada"));
          if (fuente == "SD") {
            fuente = "USB";
            staInicio = true;
          }
          break;
        case Sleeping:
          Serial.println(F("Durmiendo"));
          break;
        case SerialWrongStack:
          Serial.println(F("Obtener pila incorrecta"));
          break;
        case CheckSumNotMatch:
          Serial.println(F("Compruebe que la suma no coincide"));
          break;
        case FileIndexOut:
          Serial.println(F("Indice de archivo fuera de limites"));
          break;
        case FileMismatch:
          Serial.println(F("No se puede encontrar el archivo"));
          pistaActual = fileAleatorio();
          break;
        case Advertise:
          Serial.println(F("En Anunciar"));
          break;
        default:
          Serial.println(F("Error"));
          break;
      }
      break;
    default:
      staPause = true;
      break;
  }
}
