#include <Wire.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SLAVE_ADDRESS 0x50 // Dirección del esclavo
#define UDP_TX_PACKET_MAX_SIZE 15
#define MAX_QUEUE_SIZE 10  //Cantidad en FIFO
#define MAX_SIZE  228     //Maximo bytes para i2c



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

    bool enqueue(char packet[MAX_SIZE], int size_packet) {
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
          rear++;
        }
        strncpy(packets[rear], packet, MAX_SIZE);
        size_packets[rear]=size_packet;
        return true;
    }

    bool denqueue(char packet[], int& size_packet) {
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
    int size_packets[MAX_QUEUE_SIZE]; 
    int front, rear;
};

CircularQueue CircularQueue;

bool enviarSize;

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

  if (bufferr[0]=='P'){
    // Serial.println("Pedido Leer Dato!");
    enviarSize=true;
  }
  Serial.println("");
}


bool enviado_size=false;

 

void requestEvent() {  
  if (!CircularQueue.isEmpty()) {           //Busco en FIFO
     
    char packet[MAX_SIZE];
    int packet_size;

    if (enviado_size){
      Serial.println("Paquete enviado ");


      enviado_size=false;
    
    }
    if (enviarSize){  
      CircularQueue.denqueue(packet,packet_size);

      Serial.println("Enviando largo.... ");
      Serial.print("Enviando por I2C el tamaño del paquete: ");
      Serial.println(packet_size);
      Serial.print("Enviando por I2C el paquete: ");
      Serial.println(packet,packet_size);
      
      enviarSize=false;
      enviado_size=true;
      

      
      Wire.write(packet_size); // Envía del mensaje al maestro
      Wire.write(reinterpret_cast<byte*>(packet),packet_size); // Send the array to the master
    }



        Serial.println("");
        Serial.println("");

  }
}







void setup() {
  Serial.begin(115200);
  Wire.begin(SLAVE_ADDRESS); // Inicializa la comunicación I2C como esclavo
  Wire.onReceive(receiveEvent); // Función que se llamará cuando se reciba un mensaje
  Wire.onRequest(requestEvent); // Función que se llamará cuando se soliciten datos
  


  Serial.print("Esp32 prendido....");




}
char packetBuffer[UDP_TX_PACKET_MAX_SIZE];
char size;
int i;

void loop(){
 
  i++;
  Serial.print("Agregar a la Cola");
  Serial.println("");
//  strncpy("packet", packetBuffer,UDP_TX_PACKET_MAX_SIZE);
    int numDigits = floor(log10(abs(i))) + 1;
    snprintf(packetBuffer, sizeof(packetBuffer), "paquete %d", i);
    int len = sizeof(packetBuffer);
//      int len = 9 + numDigits;
//    size = strtol(std::to_string(len).c_str(), nullptr, 10);
    Serial.println(packetBuffer);
//    Serial.println(len);
    CircularQueue.enqueue(packetBuffer,len); 
    delay(2000);
  }