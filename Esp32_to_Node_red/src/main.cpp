#include <Arduino.h>
#include "WiFi.h"
#include <HTTPClient.h>

#define MON_TELEPHONE
// #define MA_FREEBOX

const char *name_card_elec = "esp32_test_node_red_v2"; // Nom d'hôte de la carte ESP32
// BESOIN DE ME SIMPLIFIER MA VIE
#ifdef MON_TELEPHONE
const char *ssid = "Me voici";      // SSID du réseau WiFi
const char *password = "youssef13"; // Mot de passe du réseau WiFi
String serverName = "http://192.168.238.171:1880/compteur";
#endif

void setup()
{

  Serial.begin(115200);
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
  delay(1000);
  compteur++;

  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;

    http.begin(serverName.c_str());

    // Indique au serveur le type de données envoyées
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    // Données envoyées dans le corps (body) de la requête POST
    String httpRequestData = "compteur=" + String(compteur);

    // Exécution de la requête POST
    int httpResponseCode = http.POST(httpRequestData);

    if (httpResponseCode > 0)
    {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();
      Serial.println(payload);
    }
    else
    {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  }
  else
  {
    Serial.println("Wi-Fi déconnecté");
  }
}
