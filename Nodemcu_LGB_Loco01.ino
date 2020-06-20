/*
  Pilotage locomotive LGB via NodeMCU
  avec : 
     - Nodemcu (ESP8266)
     - 1 pont en H L298N
     - 1 alimentation Lipo 3C 11,1V

  Le programme ecoute sur un topic MQTT et se déclenche des actions en fonction de la valeur reçue
 
  Source :     https://www.sla99.fr
  Date :       20/06/2020

  Changelog : 
  20/06/2020  v1    version initiale

*/

#include <ESP8266WiFi.h> 
const char* ssid = "xxxxx";
const char* password = "xxxxx";


#include <PubSubClient.h>
WiFiClient espClient;
PubSubClient client(espClient);

const char* mqtt_server =        "xxxxx";
const int   mqtt_port =          1883;
const char* mqtt_user =          "xxxxx";
const char* mqtt_password =      "xxxxx";
char* topic =                    "train/loco/loco01";

// définition des pins de l'Arduino qui contrôlent le 1er moteur
#define pinIN1 4 //D2
#define pinIN2 0 //D3
#define pinENA D1 //D1 // doit être une pin PWM

// définition des pins de l'Arduino qui contrôlent les lampes
#define pinIN3 14 //D5
#define pinIN4 12 //D6


void setup() {
  Serial.begin(9600);
  delay(10);
 
  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
 
  
  // Print the IP address
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.println(WiFi.localIP());

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  //déclaration des pin comme sorties
  pinMode(pinIN1, OUTPUT);
  pinMode(pinIN2, OUTPUT);
  pinMode(pinENA, OUTPUT);
  
  pinMode(pinIN3, OUTPUT);
  pinMode(pinIN4, OUTPUT);
  
  //Marche avant par défaut
  digitalWrite(pinIN1, HIGH);
  digitalWrite(pinIN2, LOW);
  //Vitesse 0
  analogWrite(pinENA,  0);
  //Feux éteints
  digitalWrite(pinIN3, LOW);
  digitalWrite(pinIN4, LOW);

  //Modifier la fréquence PWM -> pour ne pas que la loco "siffle", il faut des fréq > 10 kHz. 
  //Il faut faire plusieurs essais.
  //Attention : ne pas monter >20 kHz car le L298H n'aime pas trop
  analogWriteFreq(12000);
}

//Fonction qui récupère le message MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  //Les messages sont envoyés depuis l'interface web via un broker MQTT.
  //Les chiffres et lettres recues sont fixés et interprétés ici

  //Gestion de la Vitesse. La valeur varie de 0 à 1023. Cette loco démarre autour de 650/1023 (environ 5V). 
  //Ensuite, les palliers sont progressifs
  if ((char)payload[0] == '0') analogWrite(pinENA,  0);
  if ((char)payload[0] == '1') analogWrite(pinENA,  650);
  if ((char)payload[0] == '2') analogWrite(pinENA,  720);
  if ((char)payload[0] == '3') analogWrite(pinENA,  770);
  if ((char)payload[0] == '4') analogWrite(pinENA,  820);
  if ((char)payload[0] == '5') analogWrite(pinENA,  870);
  if ((char)payload[0] == '6') analogWrite(pinENA,  920);
  if ((char)payload[0] == '7') analogWrite(pinENA,  970);
  if ((char)payload[0] == '8') analogWrite(pinENA,  1023);
  

  //Gestion du Sens de marche
  //Marche avant
   if ((char)payload[0] == 'F') {
    analogWrite(pinENA,  0);
    digitalWrite(pinIN1, HIGH);
    digitalWrite(pinIN2, LOW);
   }
   //MArche arriere
   if ((char)payload[0] == 'B') {
    analogWrite(pinENA,  0);
    digitalWrite(pinIN1, LOW);
    digitalWrite(pinIN2, HIGH);
   }

   //Gestion des Feux
   //Eteint
   if ((char)payload[0] == 'O') {
    digitalWrite(pinIN3, HIGH);
    digitalWrite(pinIN4, LOW);
   }
   //Allumés
   if ((char)payload[0] == 'N') {
    digitalWrite(pinIN3, LOW);
    digitalWrite(pinIN4, LOW);
   }
     
}


void reconnect(char* topic) {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(),mqtt_user,mqtt_password)) {
      Serial.println("connected");
      client.subscribe(topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
 
void loop() {
    if (!client.connected()) {
    reconnect(topic);
  }
  client.loop();
}
