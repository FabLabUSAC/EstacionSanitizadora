/*IMPLEMENTAR LA PROGRA DEL DISPENSADOR
  IMPLEMENTAR DIAGRAMA DE ESTADOS*/
#include <pt.h>

#define ENTRA 13
#define SALE 12
#define BANDEJA_1 2
#define BANDEJA_2 3

// LCD
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd (0x27, 16, 2);

//HILOS
struct pt hilo_error;
struct pt hilo_verde1;
struct pt hilo_rojo1;
struct pt hilo_verde2;
struct pt hilo_rojo2;

//ULTRASONICO
int trig = 40;
int echo = 41;

int vpower1 = 12;
// variables
String linea;
int conteo = 0;

// motor
int ida = 38;
int vuelta = 39;

//PERMISOS DE ESTADOS
int permiso_entrada = 0;
int permiso_Bandeja1 = 0;
int permiso_Bandeja2 = 0;
int permiso_buzzer = 0;
int una_vez_buz = 0;

//PINES DE LAS TIRAS LED
int Verde_1 = 8;
int Rojo_1 = 9;
int Verde_2 = 10;
int Rojo_2 = 11;

//BANDERAS PARA CONTROL DE SENSORES
int flag_entra = 0;
int flag_sale = 0;
int flag_ban_1 = 0;
int flag_ban_2 = 0;

//VARIABLES GLOBALES
int num_personas = 0;
int i = 0;
int estado = 0;


//PINES Y VARIABLES PARA EL BUZZER
const int PinBuzzer = 5;
int PermisoBuzzer = 0;

// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurations[] = {
  4, 4, 4, 4,
  4, 4, 4, 4,
  4, 4, 4, 4,
};

void Conteo_Personas() {
  int E = digitalRead(ENTRA);

  if (E == 0 && flag_entra == 0) {
    num_personas = num_personas + 1;
    flag_entra = 1;
    estado = 1;
    permiso_entrada = 1;
    Serial.println("Concede Permiso Entrada");
  } else if (E == 1 && flag_entra == 1) {
    flag_entra = 0;
  }
}

void Deteccion_Bandeja_1(int permiso_entrada) {
  if (permiso_entrada == 1) {
    int Ban_1 = digitalRead(BANDEJA_1);
    if (Ban_1 == 0 && flag_ban_1 == 0) {
      flag_ban_1 = 1;
      estado = 2;
      permiso_Bandeja1 = 1;
      Serial.println("Concede Permiso Bandeja 1");
    } else if (Ban_1 == 1 && flag_ban_1 == 1) {
      flag_ban_1 = 0;
    }
  } else {
    estado = 666;
    Error(&hilo_error);
    Serial.println("Error Bandeja 1");
  }
}

void Deteccion_Bandeja_2(int permiso_Bandeja1) {
  if (permiso_Bandeja1 == 1) {
    int Ban_2 = digitalRead(BANDEJA_2);
    if (Ban_2 == 0 && flag_ban_2 == 0) {
      flag_ban_2 = 1;
      estado = 3;
      permiso_Bandeja2 = 1;
      una_vez_buz = 0;
      Serial.println("Concede Permiso Bandeja 2");
    } else if (Ban_2 == 1 && flag_ban_2 == 1) {
      flag_ban_2 = 0;
    }
  } else {
    estado = 666;
    Error(&hilo_error);
    Serial.println("Error Bandeja 2");
  }
}

void Buzzer(int permiso_Bandeja2) {
  if (una_vez_buz == 0) {
    Serial.println("IF");
    int j = 0;
    while (j < 3) {
      tone(5, 500);
      delay(150);
      noTone(5);
      delay(150);
      j += 1;
    }
    digitalWrite(5, HIGH);
    una_vez_buz = 1;
    estado = 4;
  }
}

void Dispensador() {
  long duracion;
  long distancia;

  digitalWrite(trig, LOW);
  delayMicroseconds(4);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);

  duracion = pulseIn(echo, HIGH); //1000 microsegundos = 1 milisegundo
  duracion = duracion / 2;
  distancia = duracion / 29;


  if (distancia <= 10) {

    Serial.println(distancia);
    conteo = conteo + 1;
    linea = "Utilizado: " + String(conteo) + " =)";
    lcd.setCursor(0, 1);
    lcd.print(linea);

    digitalWrite(ida, HIGH);
    digitalWrite(vuelta, LOW);
    delay(1000);
    digitalWrite(ida, LOW);
    digitalWrite(vuelta, LOW);
    delay(500);
    digitalWrite(ida, LOW);
    digitalWrite(vuelta, HIGH);
    delay(1000);
    digitalWrite(ida, LOW);
    digitalWrite(vuelta, LOW);
    estado = 0;
    una_vez_buz = 0;
  }
}

void Error(struct pt *pt) {
  PT_BEGIN(pt);// Aqui inicia mi protothread
  static long t = 0;
  digitalWrite(Verde_1, LOW);
  digitalWrite(Rojo_1, HIGH);
  digitalWrite(Verde_2, LOW);
  digitalWrite(Rojo_2, HIGH);

  t = millis();
  PT_WAIT_WHILE(pt, (millis() - t) < 1000); //delay(500);

  digitalWrite(Verde_1, LOW);
  digitalWrite(Rojo_1, LOW);
  digitalWrite(Verde_2, LOW);
  digitalWrite(Rojo_2, LOW);

  t = millis();
  PT_WAIT_UNTIL(pt, (millis() - t) > 1000); //delay(500);

  digitalWrite(Verde_1, LOW);
  digitalWrite(Rojo_1, HIGH);
  digitalWrite(Verde_2, LOW);
  digitalWrite(Rojo_2, HIGH);
  PT_END(pt);
}

void Breathing(int PIN_SALIDA, struct pt *pt) {
  PT_BEGIN(pt);
  static long t = 0;
  for (i = 15 ; i <= 255; i += 1)
  {
    analogWrite(PIN_SALIDA, i);
    if (i > 254) {
      t = millis();
      PT_WAIT_WHILE(pt, (millis() - t) < 2); //delay(2);
    }
    if ((i > 214) && (i < 255)) {
      t = millis();
      PT_WAIT_WHILE(pt, (millis() - t) < 4); //delay(4);
    }
    if (( i > 171) && (i < 215)) {
      t = millis();
      PT_WAIT_WHILE(pt, (millis() - t) < 5); //delay(5);
    }
    if (( i > 128) && (i < 172)) {
      t = millis();
      PT_WAIT_WHILE(pt, (millis() - t) < 9); //delay(9);
    }
    if (( i > 85) && (i < 129)) {
      t = millis();
      PT_WAIT_WHILE(pt, (millis() - t) < 12); //delay(12);
    }
    if (( i > 42) && (i < 86)) {
      t = millis();
      PT_WAIT_WHILE(pt, (millis() - t) < 14); //delay(14);
    }
    if (( i > 1) && (i < 43)) {
      t = millis();
      PT_WAIT_WHILE(pt, (millis() - t) < 15); //delay(15);
    }
  }
  for (i = 255; i >= 15; i -= 1)
  {
    analogWrite(PIN_SALIDA, i);
    if (i > 254) {
      t = millis();
      PT_WAIT_WHILE(pt, (millis() - t) < 2); //delay(2);
    }
    if ((i > 214) && (i < 255)) {
      t = millis();
      PT_WAIT_WHILE(pt, (millis() - t) < 4); //delay(4);
    }
    if (( i > 171) && (i < 215)) {
      t = millis();
      PT_WAIT_WHILE(pt, (millis() - t) < 5); //delay(5);
    }
    if (( i > 128) && (i < 172)) {
      t = millis();
      PT_WAIT_WHILE(pt, (millis() - t) < 9); //delay(9);
    }
    if (( i > 85) && (i < 129)) {
      t = millis();
      PT_WAIT_WHILE(pt, (millis() - t) < 12); //delay(12);
    }
    if (( i > 42) && (i < 86)) {
      t = millis();
      PT_WAIT_WHILE(pt, (millis() - t) < 14); //delay(14);
    }
    if (( i > 1) && (i < 43)) {
      t = millis();
      PT_WAIT_WHILE(pt, (millis() - t) < 15); //delay(15);
    }
  }
 // delay(1);
  t = millis();
  PT_WAIT_WHILE(pt, (millis() - t) < 2); //delay(2);

  PT_END(pt);
}

void setup() {
  //PINES PARA LOS SENSORES DE PROXIMIDAD
  Serial.begin(9600);
  pinMode(ENTRA, INPUT_PULLUP);
  pinMode(SALE, INPUT_PULLUP);
  pinMode(BANDEJA_1, INPUT_PULLUP);
  pinMode(BANDEJA_2, INPUT_PULLUP);

  //INICIALIZACION DE LOS HILOS PARA EL CONTROL DE LUCES
  PT_INIT(&hilo_error);
  PT_INIT(&hilo_verde1);
  PT_INIT(&hilo_rojo1);
  PT_INIT(&hilo_verde2);
  PT_INIT(&hilo_rojo2);

  //SET DE LAS TIRAS LED
  digitalWrite(Verde_2, LOW);
  digitalWrite(Verde_1, LOW);
  digitalWrite(Rojo_1, HIGH); //Breathing(Rojo_1, &hilo_rojo1);  
  digitalWrite(Rojo_2, HIGH); //Breathing(Rojo_2, &hilo_rojo2);  
  

  //v power
  pinMode(vpower1, OUTPUT);
  digitalWrite(vpower1, HIGH);

  // lcd
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("===hola_USAC===");

  // sensor
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);

  // motor
  pinMode(ida, OUTPUT);
  pinMode(vuelta, OUTPUT);
}

void loop() {

  switch (estado) {

    //ENTRADA
    case 0:
      Conteo_Personas();
      break;

    //BANDEJA_1
    case 1:
      digitalWrite(Verde_1, HIGH);
      digitalWrite(Rojo_1, LOW);
      digitalWrite(Verde_2, LOW);
      digitalWrite(Rojo_2, HIGH);
      Serial.println("Entra al metodo de Bandeja1");
      Deteccion_Bandeja_1(permiso_entrada);
      break;

    //BANDEJA_2
    case 2:
      digitalWrite(Verde_1, LOW);
      digitalWrite(Rojo_1, HIGH);
      digitalWrite(Verde_2, HIGH);
      digitalWrite(Rojo_2, LOW);
      Deteccion_Bandeja_2(permiso_Bandeja1);
      break;

    //BUZZER
    case 3:
      digitalWrite(Verde_1, LOW);
      digitalWrite(Rojo_1, HIGH);
      digitalWrite(Verde_2, LOW);
      digitalWrite(Rojo_2, HIGH);
      Buzzer(permiso_Bandeja2);
      break;

    //DISPENSADOR
    case 4:
      Dispensador();
      break;

    default:
      Error(&hilo_error);
      break;
  }
  delay(10);
}
