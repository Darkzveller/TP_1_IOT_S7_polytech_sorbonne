#include <Arduino.h>
#include "WiFi.h"
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
String serverPOST = "http://192.168.66.171:1880/donnes";
String serverGET =  "http://192.168.66.171:1880/button";

#endif

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
}

void loop()
{

  static int compteur = 0;
  compteur++;

  float humidite = dht.readTemperature();
  float temperature = dht.readHumidity();
  Serial.println();
  Serial.println("----- SERIAL -----");

  Serial.println("Temperature = " + String(temperature) + " °C");
  Serial.println("Humidite = " + String(humidite) + " %");

  Serial.print("Compteur : ");
  Serial.println(compteur);

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println();
    Serial.println("----- POST -----");

    HTTPClient httpPOST;

    httpPOST.begin(serverPOST.c_str());
    httpPOST.addHeader(
        "Content-Type",
        "application/x-www-form-urlencoded");

    // Toutes les données sont envoyées dans une seule requête
    String httpRequestData =
        "compteur=" + String(compteur) +
        "&hum=" + String(humidite, 1) +
        "&temp=" + String(temperature, 1);

    Serial.print("Donnees envoyees : ");
    Serial.println(httpRequestData);

    int httpResponseCode = httpPOST.POST(httpRequestData);

    if (httpResponseCode > 0)
    {
      Serial.print("Code HTTP : ");
      Serial.println(httpResponseCode);

      String reponse = httpPOST.getString();
      Serial.print("Reponse Node-RED : ");
      Serial.println(reponse);
    }
    else
    {
      Serial.print("Erreur HTTP : ");
      Serial.println(httpResponseCode);
    }

    httpPOST.end();

    HTTPClient httpGET;

    httpGET.begin(serverGET.c_str());

    Serial.println();
    Serial.println("----- GET -----");

    int getResponseCode = httpGET.GET();

    if (getResponseCode > 0)
    {
      Serial.print("Code HTTP GET : ");
      Serial.println(getResponseCode);

      // Récupération de la réponse de Node-RED
      String etatLED = httpGET.getString();

      Serial.print("Etat LED recu : ");
      Serial.println(etatLED);

        if (etatLED == "1" )
      {
        digitalWrite(LED_PIN, HIGH);

        Serial.println("LED : ON");
      }
      else if (etatLED == "0")
      {
        digitalWrite(LED_PIN, LOW);

        Serial.println("LED : OFF");
      }
      else
      {
        Serial.println("Etat LED inconnu !");
      }
    }
    else
    {
      Serial.print("Erreur HTTP GET : ");
      Serial.println(getResponseCode);
    }

    httpGET.end();
  }
  else
  {
    Serial.println("Wi-Fi deconnecte");
  }

  delay(500);
}