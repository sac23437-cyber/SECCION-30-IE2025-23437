/*
Control de coche Bluetooth con HC-05
Incluye monitorización por Serial
*/

// Pines para el control de motores (puente H L298N)
const int IN1 = 3;  // Motor A (Izquierdo)
const int IN2 = 4;
const int IN3 = 5;  // Motor B (Derecho)
const int IN4 = 6;

// Variables
char comando;
int velocidad = 255;  // Velocidad PWM (0-255)

void setup() {
  // Configurar pines de motor como salidas
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  
  // Iniciar comunicación serial para Bluetooth (HC-05)
  Serial.begin(38400);  // Baudrate estándar para HC-05
  
  // Mensaje inicial
  Serial.println("Sistema de control de coche Bluetooth iniciado");
  Serial.println("Esperando comandos...");
  Serial.println("F: Adelante | B: Atrás | L: Izquierda | R: Derecha | S: Detener");
}

void loop() {
  // Verificar si hay datos disponibles desde Bluetooth
  if (Serial.available() > 0) {
    comando = toupper(Serial.read());  // Convertir a mayúscula
    
    // Mostrar comando recibido en el monitor serial
    Serial.print("Comando recibido: ");
    Serial.println(comando);
    
    // Ejecutar acción según comando
    switch (comando) {
      case 'F':  // Adelante
        motorA_adelante();
        motorB_adelante();
        Serial.println("Movimiento: Adelante");
        break;
      case 'B':  // Atrás
        motorA_atras();
        motorB_atras();
        Serial.println("Movimiento: Atrás");
        break;
      case 'L':  // Izquierda
        motorA_parar();
        motorB_adelante();
        Serial.println("Movimiento: Izquierda");
        break;
      case 'R':  // Derecha
        motorA_adelante();
        motorB_parar();
        Serial.println("Movimiento: Derecha");
        break;
      case 'S':  // Detener
        motorA_parar();
        motorB_parar();
        Serial.println("Movimiento: Detenido");
        break;
      default:
        Serial.println("Comando no reconocido");
    }
  }
  delay(20);  // Pequeña pausa para estabilidad
}

// Funciones de control para Motor A (Izquierdo)
void motorA_adelante() {
  analogWrite(IN1, velocidad);
  digitalWrite(IN2, LOW);
}

void motorA_atras() {
  digitalWrite(IN1, LOW);
  analogWrite(IN2, velocidad);
}

void motorA_parar() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
}

// Funciones de control para Motor B (Derecho)
void motorB_adelante() {
  digitalWrite(IN3, LOW);
  analogWrite(IN4, velocidad);
}

void motorB_atras() {
  analogWrite(IN3, velocidad);
  digitalWrite(IN4, LOW);
}

void motorB_parar() {
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}
