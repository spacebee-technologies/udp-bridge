#include <WiFi.h>
#include <ESPmDNS.h>
#include <Wire.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "circular_queue.h"

#define SLAVE_ADDRESS 0x50  // Dirección del esclavo
#define UDP_TX_PACKET_MAX_SIZE 1472

// Define las credenciales de red WiFi
const char* ssid = "Personal-21C-2.4GHz";
const char* password = "4BD447221C";
IPAddress local_IP(192, 168, 200, 100);  // Dirección IP fija del ESP32
IPAddress gateway(192, 168, 1, 1);   // Dirección IP del gateway de la red
IPAddress subnet(255, 255, 255, 0);  // Máscara de subred de la red
IPAddress ip(192, 168, 1, 100); // Dirección IP del destinatario

// Define los puertos UDP a utilizar
unsigned int port_TC = 51524;
unsigned int port_TCresponse = 51525;
unsigned int port_TM = 51526;

typedef enum StartingByte {
  STARTING_BYTE_RECEIVE = 'P',
  STARTING_BYTE_SEND = 'S'
} StartingByte_t;

CircularQueue circularQueue;

// Crea los objetos WiFi y UDP
WiFiUDP udp_TC, udp_TCresponse, udp_TM;

// State machine variables
bool enviarSize;  // Variable que determina si se tiene que enviar el size del paquete.
bool enviado_size=false;  // Variable que indica si ya se envio el size.

void receiveEvent(int bytesReceived) {
  // Esta funcion se ejecuta siempre que el maestro envia un dato (master write)
  Serial.print("Dato I2C recibido: ");
  char buffer[CIRCULAR_QUEUE_ELEMENT_MAX_SIZE] = {0};
  int i = 0;
  while (Wire.available()) { // Si hay datos disponibles en el buffer de recepción
    char c = Wire.read(); // Lee el byte recibido
    buffer[i]= c;
    i++;
    Serial.print(c); // Muestra el byte recibido en el puerto serie
  }

  Serial.println("");

  if (buffer[0] == STARTING_BYTE_RECEIVE) {
    // El master solicita leer el mensaje, por lo tanto se envia el size del paquete primero.
    // Serial.println("Pedido Leer Dato!");
    enviarSize=true;

  } else if (buffer[0] == STARTING_BYTE_SEND) {
    // Se envia el paquete recivido por el maestro a traves de WiFi
    Serial.print("Enviando TM por UDP: ");
    Serial.println(buffer);

    int packetSize2 = udp_TM.parsePacket();

    udp_TM.beginPacket(udp_TM.remoteIP(), udp_TM.remotePort());
    udp_TM.print(buffer);
    udp_TM.endPacket();
  }
}

void requestEvent() {
  // Esta funcion se ejecuta siempre que el maestro solicita un dato (master read)
  if (!circularQueue.isEmpty()) {           //Busco en FIFO

    char packet[CIRCULAR_QUEUE_ELEMENT_MAX_SIZE];
    int packet_size;

    if (enviado_size){
      // El paquete ya esta cargado en el buffer del I2C. Por lo que se lee.
      //Serial.println("Paquete enviado ");
      enviado_size=false;
    }

    if (enviarSize){
      circularQueue.denqueue(packet, packet_size);

      // Serial.println("Enviando largo.... ");
      // Serial.print("Enviando por I2C el tamaño del paquete: ");
      // Serial.println(packet_size);
      // Serial.print("Enviando por I2C el paquete: ");
      // Serial.println(packet);

      enviarSize=false;
      enviado_size=true;

      Wire.write(packet_size); // Envía del mensaje al maestro
      Wire.write(reinterpret_cast<byte*>(packet),packet_size); // Send the array to the master

      // Se escribe en el buffer del I2C, en el primer byte el size del paquete, luego se escribre el paquete.
    }

        Serial.println("");
        Serial.println("");

  } else {  // If queue is empty we should check also if there was a request and answer length = 0
    if (enviarSize) {
      // Serial.println("Enviando largo.... ");
      // Serial.println("Enviando por I2C el tamaño del paquete: 0 (queue empty)");
      enviarSize=false;
      enviado_size=true;
      Wire.write(0); // Envía del mensaje al maestro
      // Serial.println(WiFi.localIP());
    }
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin(SLAVE_ADDRESS); // Inicializa la comunicación I2C como esclavo
  Wire.onReceive(receiveEvent); // Función que se llamará cuando se reciba un mensaje
  Wire.onRequest(requestEvent); // Función que se llamará cuando se soliciten datos

  // Inicializa la conexión WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando a la red WiFi...");
  }
  Serial.println("Conectado a la red WiFi.");
  Serial.print("Dirección IP asignada: ");
  Serial.println(WiFi.localIP());

  // Inicializa el servicio mDNS
  if (!MDNS.begin("TITO")) {
    Serial.println("Error al iniciar el servicio mDNS");
  } else {
    Serial.println("Servicio mDNS iniciado con éxito");
    MDNS.addService("udp", "port_TC", port_TC);
    MDNS.addService("udp", "port_TCresponse", port_TCresponse);
    MDNS.addService("udp", "port_TM", port_TM);
  }

  // Inicializa los objetos UDP
  udp_TC.begin(port_TC);
  udp_TCresponse.begin(port_TCresponse);
  udp_TM.begin(port_TM);

  Serial.print("Servidor UDP iniciado en los puertos: ");
  Serial.print(port_TC);
  Serial.print(", ");
  Serial.print(port_TCresponse);
  Serial.print(", ");
  Serial.println(port_TM);
  circularQueue = CircularQueue();
}

void loop() {
  // Espera a recibir datos en el puerto 3 para recibir TC
  int packetSize = udp_TC.parsePacket();
  if (packetSize) {
    char packetBuffer[UDP_TX_PACKET_MAX_SIZE];
    udp_TC.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);  // Lee el mensaje recibido
    Serial.print("TC recibido: ");
    Serial.println(packetBuffer);
    packetBuffer[packetSize]=0;
    circularQueue.enqueue(packetBuffer, packetSize);  // Agrego dato a FIFO
  }
  int packetSize3 = udp_TM.parsePacket();
  int packetSize2 = udp_TCresponse.parsePacket();
}
