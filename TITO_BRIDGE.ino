#include <WiFi.h>
#include <WiFiUDP.h>
#include <ESPmDNS.h>
#include <Wire.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SLAVE_ADDRESS 0x50 // Dirección del esclavo
#define UDP_TX_PACKET_MAX_SIZE 1472
#define MAX_QUEUE_SIZE 10  //Cantidad en FIFO
#define MAX_SIZE  228     //Maximo bytes para i2c

// Define las credenciales de red WiFi
const char* ssid = "CONECTAR1354";
const char* password = "309997609";
IPAddress local_IP(192, 168, 200, 100);  // Dirección IP fija del ESP32
IPAddress gateway(192, 168, 1, 1);   // Dirección IP del gateway de la red
IPAddress subnet(255, 255, 255, 0);  // Máscara de subred de la red
IPAddress ip(192, 168, 1, 100); // Dirección IP del destinatario

// Define los puertos UDP a utilizar
unsigned int port_TC = 51524;       
unsigned int port_TCresponse = 51525;
unsigned int port_TM = 51526;


class CircularQueue {
public:
    CircularQueue() {
        front = rear = -1;
    }

    bool isFull() {
        return ((front==0 && rear == MAX_QUEUE_SIZE - 1) || (rear==front-1));
    }

    bool isEmpty() {
        return front == -1;
    }

    bool enqueue(char packet[], unsigned short size_packet) {
        if (isFull()) {
            return false;
        }
        if (front==-1){
          front=0;
        }

        if (rear==MAX_QUEUE_SIZE - 1){
          rear=0;
        }
        else{
          rear++
        }
        strncpy(packets[rear], packet, MAX_SIZE);
        size_packets[rear]=size_packet;
        return true;
    }

    bool denqueue(char packet[], unsigned short& size_packet) {
        if (isEmpty()) {
            return false;
        }
        strncpy(packet, packets[front], MAX_SIZE);
        size_packet=size_packets[front];

        if (rear==front){
          rear=-1;
          front=-1;
        }
        else{
          front=front+1;          
        }
        return true;
    }

private:
    //Max packet size is 65,535 bytes 
    char packets[MAX_QUEUE_SIZE][MAX_SIZE];
    unsigned short size_packets[MAX_QUEUE_SIZE]; 
    int front, rear;
};

CircularQueue CircularQueue;

// Crea los objetos WiFi y UDP
WiFiUDP udp_TC, udp_TCresponse, udp_TM;

// void receiveEvent(int bytesReceived) {
//   Serial.print("Dato I2C recibido: ");
//   char bufferr[MAX_SIZE]={0};
//   int i=0;
//   while (Wire.available()) { // Si hay datos disponibles en el buffer de recepción
//     char c = Wire.read(); // Lee el byte recibido
//     bufferr[i]= c;
//     i++;
//     Serial.print(c); // Muestra el byte recibido en el puerto serie
//   }
//   Serial.println("");
//   switch (0x00) {
//     case 0x00:        //Telemetria
//     {
//         Serial.print("Enviando TM por UDP: ");
//         Serial.println(bufferr);
        
//         //?
//         int packetSize2 = udp_TM.parsePacket(); 

//         udp_TM.beginPacket(udp_TM.remoteIP(), udp_TM.remotePort());
//         udp_TM.print(bufferr);
//         udp_TM.endPacket();
//         break;
//     }
//     case 0x01:    //responder TC request o ACK de TC por UDP
//     {
//         udp_TCresponse.beginPacket(udp_TCresponse.remoteIP(), udp_TCresponse.remotePort());
//         udp_TCresponse.print("Envio ACK");
//         udp_TCresponse.endPacket();
//         break;
//     }
//     default:
//     {
//       break;
//     }
//   } 
// }


void receiveEvent(int bytesReceived) {
  Serial.print("Dato I2C recibido: ");
  char bufferr[MAX_SIZE]={0};
  int i=0;
  while (Wire.available()) { // Si hay datos disponibles en el buffer de recepción
    char c = Wire.read(); // Lee el byte recibido
    bufferr[i]= c;
    i++;
    Serial.print(c); // Muestra el byte recibido en el puerto serie
  }
  Serial.println("");
}

void requestEvent() {  o
  if (!CircularQueue.isEmpty()) {           //Busco en FIFO
        char packet[MAX_SIZE];
        unsigned short packet_size;
          
        CircularQueue.denqueue(packet,packet_size);
        Serial.print("Enviando por I2C el tamaño del paquete: ");
        Serial.println(packet_size);
        Wire.write(packet_size); // Envía el tamaño del mensaje al maestro
          
        Serial.print("Enviando por I2C el paquete: ");
        Serial.println(packet);
        Wire.write(packet); // Envía del mensaje al maestro
  }
}

// void setup() {
//   Serial.begin(115200);
//   Wire.begin(SLAVE_ADDRESS); // Inicializa la comunicación I2C como esclavo
//   Wire.onReceive(receiveEvent); // Función que se llamará cuando se reciba un mensaje
//   Wire.onRequest(requestEvent); // Función que se llamará cuando se soliciten datos
  
//   //WiFi.config(local_IP, gateway, subnet);
//   // Inicializa la conexión WiFi
//   WiFi.begin(ssid, password);
//   while (WiFi.status() != WL_CONNECTED) {
//     delay(1000);
//     Serial.println("Conectando a la red WiFi...");
//   }
//   Serial.println("Conectado a la red WiFi.");
//   Serial.print("Dirección IP asignada: ");
//   Serial.println(WiFi.localIP());
  
//   // Inicializa el servicio mDNS
//   if (!MDNS.begin("TITO")) {
//     Serial.println("Error al iniciar el servicio mDNS");
//   } else {
//     Serial.println("Servicio mDNS iniciado con éxito");
//     MDNS.addService("udp", "port_TC", port_TC);
//     MDNS.addService("udp", "port_TCresponse", port_TCresponse);
//     MDNS.addService("udp", "port_TM", port_TM);
//   }

  
//   // Inicializa los objetos UDP
//   udp_TC.begin(port_TC);
//   udp_TCresponse.begin(port_TCresponse);
//   udp_TM.begin(port_TM);

//   Serial.print("Servidor UDP iniciado en los puertos: ");
//   Serial.print(port_TC);
//   Serial.print(", ");
//   Serial.print(port_TCresponse);
//   Serial.print(", ");
//   Serial.println(port_TM);
//   CircularQueue = CircularQueue();
// }


// void loop() {
//   // Espera a recibir datos en el puerto 3 para recibir TC
//   int packetSize = udp_TC.parsePacket();
//   if (packetSize) {
//     char packetBuffer[UDP_TX_PACKET_MAX_SIZE];
//     udp_TC.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);// Lee el mensaje recibido
//     Serial.print("TC recibido: ");
//     Serial.println(packetBuffer);
//     CircularQueue.enqueue(packetSize);      //Agrega tamaño del dato o comando a mandar
//     CircularQueue.enqueue(packetBuffer);   //Agrego dato o comando al rover
//   }
//   int packetSize3 = udp_TM.parsePacket();
//   int packetSize2 = udp_TCresponse.parsePacket();
// }

void setup() {
  Serial.begin(115200);
  Wire.begin(SLAVE_ADDRESS); // Inicializa la comunicación I2C como esclavo
  Wire.onReceive(receiveEvent); // Función que se llamará cuando se reciba un mensaje
  Wire.onRequest(requestEvent); // Función que se llamará cuando se soliciten datos
  


  Serial.print("Esp32 prendido....");

  CircularQueue = CircularQueue();

  char packetBuffer[UDP_TX_PACKET_MAX_SIZE];
  unsigned short int size;

  Serial.print("Agregar a la Cola");
  packetBuffer="Primer paquete";
  size=static_cast<unsigned short int>(strlen(packetBuffer));

  Serial.println(packetBuffer);
  CircularQueue.enqueue(packetSize,size); 

  Serial.print("Agregar a la Cola");
  packetBuffer="Segundo paquete";
  size=static_cast<unsigned short int>(strlen(packetBuffer));

  Serial.println(packetBuffer);
  CircularQueue.enqueue(packetSize,size); 

    Serial.print("Agregar a la Cola");
  packetBuffer="Tercer paquete";
  size=static_cast<unsigned short int>(strlen(packetBuffer));

  Serial.println(packetBuffer);
  CircularQueue.enqueue(packetSize,size); 

}
