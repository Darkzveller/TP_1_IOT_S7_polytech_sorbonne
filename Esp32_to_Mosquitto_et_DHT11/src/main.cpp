#include <Arduino.h>
#include "WiFi.h"
#include <PubSubClient.h>
/*
cd /d "C:\Program Files\mosquitto"
mosquitto.exe -c "C:\Program Files\mosquitto\mosquitto.conf" -v
mosquitto_sub.exe -h localhost -p 1883 -t "esp32/#" -v 
*/
#include <HTTPClient.h>
#include "DHT.h"

#define LED_PIN 2

// Capteur de temperature et d'humidite DHT11
// https://tutoduino.fr/
// Copyleft 2020
#include "DHT.h"
// Definit la broche de l'Arduino sur laquelle la
// broche DATA du capteur est reliee
#define DHTPIN 4
// Definit le type de capteur utilise
#define DHTTYPE DHT11
// Declare un objet de type DHT
// Il faut passer en parametre du constructeur
// de l'objet la broche et le type de capteur
DHT dht(DHTPIN, DHTTYPE);

#define MON_TELEPHONE
// #define MA_FREEBOX

const char *name_card_elec = "esp32_test_node_red_v2"; // Nom d'hôte de la carte ESP32
// BESOIN DE ME SIMPLIFIER MA VIE
#ifdef MON_TELEPHONE
const char *ssid = "Me voici";      // SSID du réseau WiFi
const char *password = "youssef13"; // Mot de passe du réseau WiFi
const char *mqtt_server = "192.168.233.171";

#endif

WiFiClient espClient;
PubSubClient client(espClient);
void callback(char *topic, byte *message, unsigned int length);
void reconnect();

void setup()
{
  Serial.begin(9600);
  dht.begin();
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  delay(1000);
  digitalWrite(LED_PIN, LOW);
  delay(1000);
  digitalWrite(LED_PIN, HIGH);
  delay(1000);
  digitalWrite(LED_PIN, LOW);
  delay(1000);

  // Définit le nom d'hôte pour la carte ESP32 sur le réseau
  WiFi.setHostname(name_card_elec);
  // Passe le WiFi en mode station (client du réseau WiFi)
  WiFi.mode(WIFI_STA);

  // Démarre la connexion WiFi avec les identifiants donnés
  WiFi.begin(ssid, password);
  Serial.println();
  Serial.print("SSID : ");
  Serial.println(ssid);

  Serial.println("Connexion au WiFi en cours...");

  // Boucle jusqu'à ce que la connexion au WiFi soit réussie
  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.println("Connection Failed! Rebooting...");
    delay(1000);
    // Redémarrage si la connexion échoue (commenté pour l'instant)
    ESP.restart();
  }

  Serial.print("Adresse IP ESP32 : ");
  Serial.println(WiFi.localIP());

  // Affiche le succès de la connexion
  Serial.println("");
  Serial.println("Connexion établie !");

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop()
{
  static long lastMsg = 0;
  float temperature = dht.readTemperature();
  float humidite = dht.readHumidity();
  // Serial.println();
  // Serial.println("----- SERIAL -----");

  // Serial.println("Temperature = " + String(temperature) + " °C");
  // Serial.println("Humidite = " + String(humidite) + " %");

  if (WiFi.status() == WL_CONNECTED)
  {
    if (!client.connected())
    {
      reconnect();
    }
    client.loop();

    long now = millis();
    if (now - lastMsg > 500)
    {
      lastMsg = now;

      // Convert the value to a char array
      char tempString[8];
      dtostrf(temperature, 1, 2, tempString);
      // Serial.print("Temperature: ");
      // Serial.println(tempString);
      client.publish("esp32/temperature", tempString);

      // Convert the value to a char array
      char humString[8];
      dtostrf(humidite, 1, 2, humString);
      // Serial.print("Humidity: ");
      // Serial.println(humString);
      client.publish("esp32/humidity", humString);
    }
  }
  else
  {
    Serial.println("Wi-Fi deconnecte");
  }

  delay(500);
}

void callback(char *topic, byte *message, unsigned int length)
{
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++)
  {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off".
  // Changes the output state according to the message
  if (String(topic) == "esp32/output")
  {
    Serial.print("Changing output to ");
    if (messageTemp == "true")
    {
      Serial.println("true");
      digitalWrite(LED_PIN, HIGH);
    }
    else if (messageTemp == "false")
    {
      Serial.println("false");
      digitalWrite(LED_PIN, LOW);
    }
  }
}

void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP32Client_Youssef"))
    {
      Serial.println("connected");
      // Subscribe
      client.subscribe("esp32/output");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
