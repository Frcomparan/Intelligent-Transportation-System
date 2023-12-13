#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ESP32Servo.h>

const char *ssid = "*******";     // Cambia por tu SSID
const char *password = "*******"; // Cambia por tu contraseña

// Creamos el objeto server en el puerto 80
AsyncWebServer server(80);

// ------------------------ Carrito configuración -------------------------------
/**
 17 |||   |||  18
 05 |||   |||  19+


 04 |||   |||  15
 16 |||   |||  16
**/

// Llanta trasceras
#define din1 15 // cafe **der
#define din2 2  // rojo ** der
#define din3 4  // blanco ** izq
#define din4 16 // negro ** izq

// Llanta delanteras
#define tin1 17 // azul ** izq
#define tin2 5  // verde ** izq
#define tin3 18 // morado ** der
#define tin4 19 // blanco ** der
// ------------------------ Carrito configuración -------------------------------

// ======================================================================================================

// ------------------------ Carrito seguidor de linea -------------------------------

/*
  Variables para los seguidores de linea
*/

// Sensores para el avance hacia delante (Del)
int lecturaSensorIzqDel;        // Almacena el valor de la lectura del sensor izquierdo
int lecturaSensorDerDel;        // Almacena el valor de la lectura del sensor derecho
const int sensorIzqPinDel = 35; // Valor de pin del sensor izquierdo
const int sensorDerPinDel = 34; // Valor de pin del sensor derecho

// Sensores para el avance hacia atras (Atr)
int lecturaSensorIzqAtr;        // Almacena el valor de la lectura del sensor izquierdo
int lecturaSensorDerAtr;        // Almacena el valor de la lectura del sensor derecho
const int sensorIzqPinAtr = 32; // Valor de pin del sensor izquierdo
const int sensorDerPinAtr = 33; // Valor de pin del sensor derecho

// Bandera que ayuda a saber si el carro debera avanzar (hacia delante o detras)
bool avance = false;

// ------------------------ Carrito seguidor de linea -------------------------------

// ======================================================================================================

// ------------------------ Brazo configuración -------------------------------

// 0 - BASE, 1 - HOMBRO, 2 - CODO, 3 - MUÑECA 1, 4 - MUÑECA 2, 5 - CODO
// Array con los servos del brazo
Servo servos[6];

// Array con las posiones previas de los servos
int prevPositions[6];

// 15 rapido / 20 medio / 25 lento
int servosVel[] = {15, 20, 15, 15, 15, 15};

// Pines del servos del brazo
#define base 13
#define hombro 12
#define codo 14
#define muneca1 27
#define muneca2 26
#define garra 25

// ------------------------ Brazo configuración -------------------------------

void setup()
{
  // Inicializa la comunicación serial
  Serial.begin(115200);

  // ======================================================================================================

  // ------------------------ Brazo configuración -------------------------------
  // Inicializamos los servos con la libreria
  servos[0].attach(base);    // BASE
  servos[1].attach(hombro);  // HOMBRO
  servos[2].attach(codo);    // CODO
  servos[3].attach(muneca1); // MUÑECA 1 (IZQ - DER)
  servos[4].attach(muneca2); // MUÑECA 2 (ARR - ABA)
  servos[5].attach(garra);   // GARRA

  // Mandamos los servos a su posición inicial
  prevPositions[0] = 90; // BASE
  servos[0].write(prevPositions[0]);

  prevPositions[1] = 100; // HOMBRO
  servos[1].write(prevPositions[1]);

  prevPositions[2] = 140; // CODO
  servos[2].write(prevPositions[2]);

  prevPositions[3] = 110; // MUÑECA 1
  servos[3].write(prevPositions[3]);

  prevPositions[4] = 80; // MUÑECA 2
  servos[4].write(prevPositions[4]);

  prevPositions[5] = 130; // GRIPPER
  servos[5].write(prevPositions[5]);
  // ------------------------ Brazo configuración -------------------------------

  // ======================================================================================================

  // ------------------------ Carrito seguidor de linea -------------------------------
  // Inicializamos los pines del carrito como pines de entrada
  pinMode(sensorIzqPinDel, INPUT);
  pinMode(sensorDerPinDel, INPUT);
  pinMode(sensorIzqPinAtr, INPUT);
  pinMode(sensorDerPinAtr, INPUT);
  // ------------------------ Carrito seguidor de linea -------------------------------

  // ======================================================================================================

  // ------------------------ Carrito configuración -------------------------------
  // Inicializamos los pines del carrito como pines de salida
  // Ruedas delanteras
  pinMode(tin1, OUTPUT);
  pinMode(tin2, OUTPUT);
  pinMode(tin3, OUTPUT);
  pinMode(tin4, OUTPUT);

  // Ruedas traseras
  pinMode(din1, OUTPUT);
  pinMode(din2, OUTPUT);
  pinMode(din3, OUTPUT);
  pinMode(din4, OUTPUT);
  // Apagamos ruedas despues de inicializarlas (por si acaso)
  ruedasApagadas();
  // ------------------------ Carrito configuración -------------------------------

  // ======================================================================================================

  // ------------------------ Conexión a Wifi configuración -------------------------------
  // Conéctate a la red Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Conectando a WiFi...");
  }

  Serial.println("Conexión exitosa");
  Serial.print("Dirección IP: ");
  Serial.println(WiFi.localIP());
  // ------------------------ Conexión a WiFi configuración -------------------------------

  // ======================================================================================================
  // ------------------------ Servidor configuración -------------------------------

  /*
   Seteo de la ruta del servidor que maneja el movimiento del brazo del carrito
  */
  server.on("/brazo", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    // Obtenemos los valores de los parametros
    String servoString = request->arg("servo");
    String gradosString = request->arg("grados");

    // Convertimos los valores a enteros
    int servoNum = servoString.toInt();
    int gradosNum = gradosString.toInt();
    
    // Validamos que los valores sean correctos
    if ((gradosNum >= 0 && gradosNum <= 180) && (servoNum >= 0 && servoNum <= 5)) {

      // Movemos el servo
      moveServo(servoNum, gradosNum);
      
      // Enviamos la respuesta
      String response = "Servo: " + servoString + " trasladado a " + gradosString + "°";
      request->send(200, "text/plain", response);
    } else {
      request->send(400, "text/plain", "Grados no validos");
    } });

  /*
    Seteo de la ruta del servidor que maneja el movimiento del carrito base
    en el seguidor de linea
  */
  server.on("/seguidor", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              // Obtenemos los valores de los parametros
              String movimiento = request->arg("mov");

              // Validamos que los valores sean correctos
              if (movimiento == "adelante")
              {
                // Movemos el carro
                avance = true;
                moverAdelante(200);
                seguidorAdelante();

                // Enviamos la respuesta
                String response = "Carrito avanza hacia: " + movimiento;
                request->send(200, "text/plain", response);
              }
              else if (movimiento == "atras")
              {
                // Movemos el carro
                avance = true;
                moverAtras(200);
                seguidorAtras();

                // Enviamos la respuesta
                String response = "Carrito avanza hacia: " + movimiento;
                request->send(200, "text/plain", response);
              }
              else
              {
                request->send(400, "text/plain", "Movimiento no validos");
              } });

  // Iniciar el servidor
  server.begin();
  // ------------------------ Servidor configuración -------------------------------
}

void loop()
{
}

void moveServo(int servo, int degrees)
{
  // Mostramos en el serial el servo y los grados enviados
  Serial.print("Servo: ");
  Serial.print(servo);
  Serial.print(", Grados: ");
  Serial.println(degrees);

  // Movimiento si los grados son menores a los actuales
  if (prevPositions[servo] > degrees)
  {
    for (int j = prevPositions[servo]; j >= degrees; j--)
    {
      servos[servo].write(j);
      delay(servosVel[servo]);
    }
  }
  // Movimiento si los grados son mayores a los actuales
  else
  {
    for (int j = prevPositions[servo]; j <= degrees; j++)
    {
      servos[servo].write(j);
      delay(servosVel[servo]);
    }
  }

  // Guardamos los grados actuales
  prevPositions[servo] = degrees;
}

/*
  Metodo que desactiva los pines (coloca en LOW) de las ruedas
  para detener el carro
*/
void ruedasApagadas()
{
  // Rueda delantera izquierda
  digitalWrite(tin1, LOW);
  digitalWrite(tin2, LOW);

  // Rueda delantera derecha
  digitalWrite(tin3, LOW);
  digitalWrite(tin4, LOW);

  // Rueda traseara derecha
  digitalWrite(din1, LOW);
  digitalWrite(din2, LOW);

  // Rueda trasera izquierda
  digitalWrite(din3, LOW);
  digitalWrite(din4, LOW);
}

/*
  Metodo que activa los pines de las ruedas
  para mover el carro hacia delante
*/
void ruedasAdelante()
{
  // Rueda delantera izquierda
  digitalWrite(tin1, LOW);
  digitalWrite(tin2, HIGH);

  // Rueda delantera derecha
  digitalWrite(tin3, LOW);
  digitalWrite(tin4, HIGH);

  // Rueda traseara derecha
  digitalWrite(din1, LOW);
  digitalWrite(din2, HIGH);

  // Rueda trasera izquierda
  digitalWrite(din3, LOW);
  digitalWrite(din4, HIGH);
}

/*
  Metodo que activa los pines de las ruedas
  para mover el carro hacia atras
*/
void reudasDetras()
{
  // Rueda delantera izquierda
  digitalWrite(tin1, HIGH);
  digitalWrite(tin2, LOW);

  // Rueda delantera derecha
  digitalWrite(tin3, HIGH);
  digitalWrite(tin4, LOW);

  // Rueda traseara derecha
  digitalWrite(din1, HIGH);
  digitalWrite(din2, LOW);

  // Rueda trasera izquierda
  digitalWrite(din3, HIGH);
  digitalWrite(din4, LOW);
}

/*
  Metodo que activa los pines de las ruedas
  para mover el carro hacia la izquerda
*/
void reudasIzquierda()
{
  // Rueda delantera izquierda
  digitalWrite(tin1, HIGH);
  digitalWrite(tin2, LOW);

  // Rueda delantera derecha
  digitalWrite(tin3, LOW);
  digitalWrite(tin4, HIGH);

  // Rueda traseara derecha
  digitalWrite(din1, LOW);
  digitalWrite(din2, HIGH);

  // Rueda trasera izquierda
  digitalWrite(din3, HIGH);
  digitalWrite(din4, LOW);
}

/*
  Metodo que activa los pines de las ruedas
  para mover el carro hacia la deracha
*/
void reudasDerecha()
{
  // Rueda delantera izquierda
  digitalWrite(tin1, LOW);
  digitalWrite(tin2, HIGH);

  // Rueda delantera derecha
  digitalWrite(tin3, HIGH);
  digitalWrite(tin4, LOW);

  // Rueda traseara derecha
  digitalWrite(din1, HIGH);
  digitalWrite(din2, LOW);

  // Rueda trasera izquierda
  digitalWrite(din3, LOW);
  digitalWrite(din4, HIGH);
}

/*
  Función para mover el carro hacia delante
  "x" cantidad de tiempo
*/
void moverAdelante(int time)
{
  ruedasAdelante();

  delay(time);

  ruedasApagadas();
}

/*
  Función para mover el carro hacia atras
  "x" cantidad de tiempo
*/
void moverAtras(int time)
{
  reudasDetras();

  delay(time);

  ruedasApagadas();
}

/*
  Función para mover el carro hacia la derecha
  "x" cantidad de tiempo
*/
void moverDerecha(int time)
{
  reudasDerecha();

  delay(time);

  ruedasApagadas();
}

/*
  Función para mover el carro hacia la izquierda
  "x" cantidad de tiempo
*/
void moverIzquierda(int time)
{

  reudasIzquierda();

  delay(time);

  ruedasApagadas();
}

/*
  Función para dar el seguimiento de linea hacie delante
*/
void seguidorAdelante()
{
  bool avance = true;
  while (avance)
  {
    // Se analiza el resultado de los sensores para hacer que el robot siga la línea negra
    lecturaSensorIRDel(); // Se lee el valor de los sensores IR

    // Si el resultado de ambos sensores es 0 (zona blanca) el robot sigue se para
    if (lecturaSensorIzqDel == 0 && lecturaSensorDerDel == 0)
    {
      ruedasApagadas(); // El robot para
      avance = false;
    }
    // Si el izquierdo retorna 0 (zona blanca) y el derecho 1 (negra) el robot gira derecha
    if (lecturaSensorIzqDel == 0 && lecturaSensorDerDel == 1)
    {
      // El robot gira a la derecha
      reudasDerecha();
    }
    // Si el izquierdo retorna 1 (zona negra) y el derecho 0 (blanca) el robot gira izquierda
    if (lecturaSensorIzqDel == 1 && lecturaSensorDerDel == 0)
    {
      reudasIzquierda();
    }
    // Si ambos sensores retornan 0 (zona negra) el robot sigue recto
    if (lecturaSensorIzqDel == 1 && lecturaSensorDerDel == 1)
    {
      ruedasAdelante(); // El robot avanza
      Serial.println("robot avanza");
    }
  }
}

/*
  Función para dar el seguimiento de linea en reversa (para atras)
*/
void seguidorAtras()
{
  while (avance)
  {
    // Se analiza el resultado de los sensores para hacer que el robot siga la línea negra
    lecturaSensorIRAtr(); // Se lee el valor de los sensores IR

    // Si el resultado de ambos sensores es 0 (zona blanca) el robot sigue se para
    if (lecturaSensorIzqAtr == 0 && lecturaSensorDerAtr == 0)
    {
      ruedasApagadas(); // El robot para
      avance = false;
    }
    // Si el izquierdo retorna 0 (zona blanca) y el derecho 1 (negra) el robot gira derecha
    if (lecturaSensorIzqAtr == 0 && lecturaSensorDerAtr == 1)
    {
      // El robot gira a la derecha
      reudasDerecha();
    }
    // Si el izquierdo retorna 1 (zona negra) y el derecho 0 (blanca) el robot gira izquierda
    if (lecturaSensorIzqAtr == 1 && lecturaSensorDerAtr == 0)
    {
      reudasIzquierda();
    }
    // Si ambos sensores retornan 0 (zona negra) el robot sigue recto
    if (lecturaSensorIzqAtr == 1 && lecturaSensorDerAtr == 1)
    {
      reudasDetras(); // El robot avanza
      Serial.println("robot avanza");
    }
  }
}

/*
  Función lecturaSensorIRDel: leerá el valor de los sensores delanteros
  y lo almacena en una variable. Dicho sensor retornará el valor 0 (LOW) si
  el sensor está en zona blanca y el valor 1 (HIGH) si el sensor está en zona
  negra.
*/
void lecturaSensorIRDel()
{
  lecturaSensorIzqDel = digitalRead(sensorIzqPinDel); // Almacena la lectura del sensor izquierdo
  lecturaSensorDerDel = digitalRead(sensorDerPinDel); // Almacena la lectura del sensor derecho
}

/*
  Función lecturaSensorIRAtr: leerá el valor de los sensores traseros
  y lo almacena en una variable. Dicho sensor retornará el valor 0 (LOW) si
  el sensor está en zona blanca y el valor 1 (HIGH) si el sensor está en zona
  negra.
*/
void lecturaSensorIRAtr()
{
  lecturaSensorIzqAtr = digitalRead(sensorIzqPinAtr); // Almacena la lectura del sensor izquierdo
  lecturaSensorDerAtr = digitalRead(sensorDerPinAtr); // Almacena la lectura del sensor derecho
}