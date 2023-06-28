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

// Define los puertos UDP a utilizar
unsigned int port_TC = 51524;
unsigned int port_TCresponse = 51525;
unsigned int port_TM = 51526;

CircularQueue circularQueue;
CommunicationSequence communicationSequence;

// Crea los objetos WiFi y UDP
WiFiUDP udp_TC, udp_TCresponse, udp_TM;

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
  communicationSequence.handleReceive(buffer, i, &udp_TM);
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

  // Init queue
  circularQueue = CircularQueue();

  // Init communication sequence
  communicationSequence = CommunicationSequence();
}

void loop() {
  // Espera a recibir datos en el puerto 3 para recibir TC
  int packetSize = udp_TC.parsePacket();
  if (packetSize) {
    char packetBuffer[UDP_TX_PACKET_MAX_SIZE];
    udp_TC.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);  // Lee el mensaje recibido
    packetBuffer[packetSize]=0;
    Serial.print("TC recibido: ");
    Serial.println(packetBuffer);
    circularQueue.enqueue(packetBuffer, packetSize);  // Agrego dato a FIFO
  }
  int packetSize3 = udp_TM.parsePacket();
  int packetSize2 = udp_TCresponse.parsePacket();
}
