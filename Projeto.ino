#include "WiFi.h"
#include <IOXhop_FirebaseESP32.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// Config Firebase -- DADOS DO FIREBASE - CONECTANDO AO BANCO
#define FIREBASE_HOST "kiron-921c2.firebaseio.com" 
#define FIREBASE_AUTH "wB8VfVP8wf4lvk7s6IxCm7fXYoL833kzF1DpO60E"

//****Definindo o formado da dada
#define NTP_OFFSET -3 * 60 * 60 // In seconds
#define NTP_INTERVAL 60 * 1000  // In miliseconds
#define NTP_ADDRESS "0.br.pool.ntp.org"
// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);

// Config connect WiFi -  wifi da rede local
#define WIFI_SSID "lidio"
#define WIFI_PASSWORD "lidio123"

#define DEBUG                   //Se descomentar esta linha vai habilitar a 'impressão' na porta serial

//******************************** variaveis que indicam o núcleo **********************************************
static uint8_t taskCoreZero = 0; // ENVIA OS DADOS
static uint8_t taskCoreOne  = 1;

// Variables to save date and time
String formattedDate;
String dayStamp;
String timeStamp;

//************************************* Pinos Usados do Esp32 **************************************************

#define LED_VERMELHO 2 // PINO DO LED VERMELHO
#define LED_AMARELO 16 // PINO DO LED AMARELO
#define LED_VERDE 18 // PINO DO LED VERDE
#define BTN_VERMELHO 32 // PINO DO BOTÃO REFERENTE AO VERMELHO
#define BTN_AMARELO  35   // PINO DO BOTÃO REFERENTE AO AMARELO
#define BTN_VERDE 34   // PINO DO BOTÃO REFERENTE AO VERDE


//************************************ Setup dos Pinos do Esp32 *************************************************
void setupPins(){

  pinMode(LED_VERMELHO, OUTPUT);
  pinMode(LED_AMARELO, OUTPUT);
  pinMode(LED_VERDE, OUTPUT);
  pinMode(BTN_VERMELHO, INPUT);
  pinMode(BTN_AMARELO, INPUT);
  pinMode(BTN_VERDE, INPUT);
}


//********************************* Nomes das Refencias do Firebase *********************************************
String Dispositivo = "Leito_01";
String data = "01-12-2018";
String status = "";

//############################################################################################################################################################################################
void setupWifi(){
  WiFi.mode(WIFI_STA);
  // connect to wifi.
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Conectando: ");
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }  
  Serial.println();
  Serial.print("Conectado: ");
  Serial.println(WiFi.localIP());
}

//**************************************************************************************************************************************************
void setupFirebase(){
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}


// *** (14) FUNÇÃO ENVIOFIREBASE() ***
void sendFirebase() {  

  
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  //  root["data"] = data;
    root["status"] = status;
    
  Firebase.set(Dispositivo, root);
  
  if (Firebase.failed()) {
    Serial.print(" :( Falhou!: ");
    Serial.println(Firebase.error()); 
    return;
  }       
}


//############################################################################################################################################################################################
/* void blink(byte pin, int duration) {
  digitalWrite(pin, HIGH);
  delay(duration);            CASO QUEIRA FICAR COM O LED PISCANDO
  digitalWrite(pin, LOW);
  delay(duration);
} */

//############################################################################################################################################################################################
//essa função será responsável verificar o status do chamado e acionar o led
void Chamado_Core0_Task1( void * parameter ){
    for (;;){
      
      if(digitalRead(BTN_VERMELHO)){
        status = "Dor Intensa";
        digitalWrite(LED_VERMELHO, HIGH);
        delay(2000); 
      }else if(digitalRead(BTN_AMARELO)){
        status = "Dor Moderada";
        digitalWrite(LED_AMARELO, HIGH);
        delay(2000);
        
      }else if(digitalRead(BTN_VERDE)){
        status = "Chamado";
        digitalWrite(LED_VERDE, HIGH);
        delay(2000);
      }else{
       // status = "";
        digitalWrite(LED_VERMELHO, LOW);
        digitalWrite(LED_AMARELO, LOW);
        digitalWrite(LED_VERDE, LOW);
      } 
      delay(10);
    } 
}

//############################################################################################################################################################################################
void setup() {
  #ifdef DEBUG
    Serial.begin(115200);
  #endif

  setupPins();
  setupWifi();
  setupFirebase();  
  Serial.println("CHAMADO KIRON - ENFERMEIRA - ESP32 ");
  
  delay(500);  // needed to start-up task1
  
  //cria uma tarefa que será executada na função Chamado_Core0_Task1, com prioridade 1 e execução no núcleo 0
  //Chamado_Core0_Task1: piscar LED e contar quantas vezes
  xTaskCreatePinnedToCore(
                    Chamado_Core0_Task1,    /* função que implementa a tarefa */
                    "Chamado_Core0_Task1",  /* nome da tarefa */
                    1000,               /* número de palavras a serem alocadas para uso com a pilha da tarefa */
                    NULL,               /* parâmetro de entrada para a tarefa (pode ser NULL) */
                    5,                  /* prioridade da tarefa (0 a N) */
                    NULL,             /* referência para a tarefa (pode ser NULL) */
                    taskCoreZero);      /* Núcleo que executará a tarefa */
                    
  delay(500); //tempo para a tarefa iniciar

  timeClient.begin();
  
}
//############################################################################################################################################################################################

void loop() {
  timeClient.update();
  
  formattedDate = timeClient.getFormattedTime();
  Serial.println(formattedDate);
  
  sendFirebase();
}
