#include <WiFi.h>

const char *ssid = "Wifitest";
const char *password = "Lance113";

void SetupWifi() {
  Serial.println("Conectando a WiFi...");
  WiFi.begin(ssid, password);

  int intentos = 0;
  while (WiFi.status() != WL_CONNECTED && intentos < 20) {
    delay(500);
    Serial.print(".");
    intentos++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConectado al WiFi");
    Serial.print("IP asignada: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nNo se pudo conectar al WiFi.");
  }
}

void CheckWifi() {
  static unsigned long lastCheck = 0;
  const unsigned long checkInterval = 10000;
  unsigned long now = millis();

  if (now - lastCheck < checkInterval) return;
  lastCheck = now;

  wl_status_t status = WiFi.status();

  if (status == WL_CONNECTED) {
    // OK
    return;
  }

  if (status == WL_DISCONNECTED || status == WL_CONNECTION_LOST) {
    Serial.println("WiFi desconectado, intentando reconectar...");
    WiFi.disconnect(true);
    delay(1000);
    WiFi.begin(ssid, password);
  } else if (status == WL_IDLE_STATUS) {
    Serial.println("WiFi está intentando conectar...");
  }
}

