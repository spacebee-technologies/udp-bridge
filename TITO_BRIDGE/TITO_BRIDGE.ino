#include <WiFi.h>
#include <ESPmDNS.h>
#include <Wire.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "circular_queue.h"
#include "communication_sequence.h"

#define SLAVE_ADDRESS 0x50  // Dirección del esclavo
#define UDP_TX_PACKET_MAX_SIZE 1472

// Define las credenciales de red WiFi
const char* ssid = "Personal-21C-2.4GHz";
const char* password = "4BD447221C";
IPAddress local_IP(192, 168, 200, 100);  // Dirección IP fija del ESP32
IPAddress gateway(192, 168, 1, 1);   // Dirección IP del gateway de la red
IPAddress subnet(255, 255, 255, 0);  // Máscara de subred de la red
IPAddress ip(192, 168, 1, 100); // Dirección IP del destinatario

// This application listens for UDP packets from the network in the following port
unsigned int port = 51524;

CircularQueue circularQueue;
CommunicationSequence communicationSequence;

// Crea los objetos WiFi y UDP
WiFiUDP udpHandler;

void receiveEvent(int bytesReceived) {
  // Esta funcion se ejecuta siempre que el maestro envia un dato (master write)
  Serial.print("Dato I2C recibido: 0x");
  char buffer[CIRCULAR_QUEUE_ELEMENT_MAX_SIZE] = {0};
  int i = 0;
  while (Wire.available()) { // Si hay datos disponibles en el buffer de recepción
    char c = Wire.read(); // Lee el byte recibido
    buffer[i] = c;
    i++;
    Serial.print(c, HEX);  // Muestra el byte recibido en el puerto serie
  }
  Serial.println("");
  communicationSequence.handleReceive(buffer, i);
}

void requestEvent() {
  communicationSequence.handleRequest(&circularQueue);
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
    MDNS.addService("udp", "port", port);
  }

  // Inicializa los objetos UDP
  udpHandler.begin(port);
  Serial.print("UDP server started at port: ");
  Serial.print(port);

  // Init queue
  circularQueue = CircularQueue();

  // Init communication sequence
  communicationSequence = CommunicationSequence();
}

void loop() {
  // Espera a recibir datos en el puerto 3 para recibir TC
  int packetSize = udpHandler.parsePacket();
  if (packetSize) {
    char packetBuffer[UDP_TX_PACKET_MAX_SIZE];
    udpHandler.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);  // Lee el mensaje recibido
    packetBuffer[packetSize]=0;
    Serial.print("TC recibido: ");
    Serial.println(packetBuffer);
    circularQueue.enqueue(packetBuffer, packetSize);  // Agrego dato a FIFO
  }
}
