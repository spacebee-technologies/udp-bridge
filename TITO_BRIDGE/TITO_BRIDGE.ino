#include <WiFi.h>
#include <Wire.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <AsyncUDP.h>

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

const char* udpServerIP = "192. 168.200.100";
const int udpServerPort = 51524;
const int port=51524;
// This application listens for UDP packets from the network in the following port


CircularQueue circularQueue;
CommunicationSequence communicationSequence;

// Crea los objetos WiFi y UDP
WiFiUDP udpHandler;
AsyncUDP udp;

void receiveEvent(int bytesReceived) {
  // Esta funcion se ejecuta siempre que el maestro envia un dato (master write)
  // Serial.print("Dato I2C recibido: 0x");
  char buffer[CIRCULAR_QUEUE_ELEMENT_MAX_SIZE] = {0};
  int i = 0;
  while (Wire.available()) { // Si hay datos disponibles en el buffer de recepción
    char c = Wire.read(); // Lee el byte recibido
    buffer[i] = c;
    i++;
    // Serial.print(c, HEX);  // Muestra el byte recibido en el puerto serie
  }
  // Serial.println("");
  communicationSequence.handleReceive(buffer, i);
}

void requestEvent() {
  communicationSequence.handleRequest(&circularQueue);
}

void processPacket(char* packet, int packetSize){
    if (packetSize) {
      //packetBuffer[packetSize]=0;
      Serial.print("TC recibido: ");
      for (int i = 0; i < packetSize; i++) {
        Serial.print(packet[i], HEX);  // Print each byte in hexadecimal format
        Serial.print(" ");
      }
      Serial.println();  // Print a new line after the hexadecimal output
      circularQueue.enqueue(packet, packetSize);  // Agrego dato a FIFO
    }
  }

void setup() {
  delay(4000);
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
  Serial.print("Signal Strength: ");
  Serial.println(WiFi.RSSI());


   if(udp.listen(port)) {
      Serial.println("Escucho...");
        udp.onPacket([](AsyncUDPPacket packet) {
            int packetSize = packet.length();
            char packetBuffer[packetSize];
            memcpy(packetBuffer, (char*)packet.data(), packet.length());
            packetBuffer[packetSize]=0;
            Serial.print("TC recibido: ");
            for (int i = 0; i < packetSize; i++) {
              Serial.print(packetBuffer[i], HEX);  // Print each byte in hexadecimal format
              Serial.print(" ");
            }
            circularQueue.enqueue(packetBuffer, packetSize); 
            Serial.println(); 
        });
    }



  // Init queue
  circularQueue = CircularQueue();

  // Init communication sequence
  communicationSequence = CommunicationSequence();
}

void loop(){

}

