#include "DFRobot_DF2301Q.h"

const int in1 = 9, in2 = 10, in3 = 11, in4 = 12;
const int velocidadMotor = 80;
const int trigPin = 5, echoPin = 6;
const int distanciaMinima = 20, distanciaMaxima = 150;

DFRobot_DF2301Q_I2C asr; // Se usa I2C
long duration;
int distance;
bool modoSeguimiento = false;
unsigned long lastWakeTime = 0;
const unsigned long wakeInterval = 5000; // Intentar reactivar cada 5 segundos

void setup(){
  Serial.begin(115200);
  pinMode(in1, OUTPUT); pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT); pinMode(in4, OUTPUT);
  pinMode(trigPin, OUTPUT); pinMode(echoPin, INPUT);
  
  while (!asr.begin()) {
    Serial.println("Fallo de comunicación I2C con el módulo ASR");
    delay(3000);
  }

  asr.setVolume(7);  
  asr.setWakeTime(255);  // Mantener el módulo siempre activo
  Serial.println("ASR listo en modo I2C.");
}

int medirDistancia(){
  digitalWrite(trigPin, LOW); delayMicroseconds(2);
  digitalWrite(trigPin, HIGH); delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  return duration * 0.034 / 2;
}

void avanzarLento(){
  analogWrite(in1, velocidadMotor); analogWrite(in2, 0);
  analogWrite(in3, velocidadMotor); analogWrite(in4, 0);
}

void detener(){
  analogWrite(in1, 0); analogWrite(in2, 0);
  analogWrite(in3, 0); analogWrite(in4, 0);
}

void girarIzquierda(){
  analogWrite(in1, velocidadMotor); analogWrite(in2, 0);
  analogWrite(in3, 0); analogWrite(in4, velocidadMotor); // Un motor hacia adelante y otro hacia atrás
}

void loop(){
  unsigned long currentMillis = millis();

  // Intentar reactivar el módulo si ha pasado mucho tiempo
  if (currentMillis - lastWakeTime > wakeInterval) {
    Serial.println("Intentando reactivar ASR...");
    asr.begin();
    asr.setWakeTime(255);
    lastWakeTime = currentMillis;
  }

  uint8_t cmd = asr.getCMDID();
  
  if (cmd) {
    Serial.print("Comando recibido: ");
    Serial.println(cmd);
    
    if (cmd == 22) modoSeguimiento = true;  // Activar seguimiento
    else if (cmd == 93) modoSeguimiento = false;  // Desactivar seguimiento
    else if (cmd == 80) { // TRUCO: Girar a la izquierda por 4.5 segundos
      Serial.println("Girando a la izquierda...");
      girarIzquierda();
      delay(4500); // Espera 4.5 segundos
      detener();
      Serial.println("Giro completado.");
    }
  }

  if (modoSeguimiento) {
    distance = medirDistancia();
    Serial.print("Distancia: ");
    Serial.print(distance);
    Serial.println(" cm");

    if (distance > distanciaMinima && distance < distanciaMaxima)
      avanzarLento();
    else
      detener();
  } else {
    detener();
  }

  delay(100);
}
