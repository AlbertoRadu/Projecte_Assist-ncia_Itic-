#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

// --------------------
// 🔑 1. WIFI Y AWS
// --------------------
const char* ssid = "Pixel_1987";
const char* password = "123456789";

#define AWS_IOT_ENDPOINT "awo7krzvt5wzv-ats.iot.us-east-1.amazonaws.com"
#define THING_NAME "ESP32_Control_Assist"

#define AWS_PUB_TOPIC "itic/projecte1/entradas"
#define AWS_SUB_TOPIC "itic/projecte1/feedback/" THING_NAME 

// --------------------
// 🔐 2. CERTIFICADOS AWS
// --------------------
const char* aws_root_ca =
"-----BEGIN CERTIFICATE-----\n"
"MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\n"
"ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\n"
"b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\n"
"MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\n"
"b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\n"
"ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\n"
"9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\n"
"IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\n"
"VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\n"
"93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\n"
"jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\n"
"AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\n"
"A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\n"
"U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\n"
"N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\n"
"o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\n"
"5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\n"
"rqXRfboQnoZsG4q5WTP468SQvvG5\n"
"-----END CERTIFICATE-----\n";

const char* certificate_pem =
"-----BEGIN CERTIFICATE-----\n"
"MIIDWTCCAkGgAwIBAgIUUazfKNUnXJAJd6W2IOhy3flB+gkwDQYJKoZIhvcNAQEL\n"
"BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g\n"
"SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTI1MTExMzE1MzMz\n"
"MFoXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0\n"
"ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAKwSlpxAVIrd/150yfoK\n"
"O74shwVCbMwprF5OHZIUic6YhVfaoVYfrJht3PCfV6QhgW6Sg3F8QErPd9ZLADgP\n"
"P+lRdnCsPi+QXg67A0WXJl8NRZdWp7InlQVKJOkg9LBBb6Ugxr4TLEDQY3pmjlv7\n"
"1pxA3a0XAeGkxOSJwrYiRUMBr/kxMvD3Drq7Onjy0po+haHA8q+yEKFbDjQmGdRx\n"
"fyupbgUBT9DbA+/oKUyQlsYIJxs4xYMD3xpVpsOu3mTO+hx6EgWUGdoK/CPjK03+\n"
"fPN2+hvpY9ymwry3T5MeO77dOll/KFZHJZWwUISFGzJ0BK1z8el4pMidbWSmjE8G\n"
"p3cCAwEAAaNgMF4wHwYDVR0jBBgwFoAUL6UHhuQ0Gh4DCpYRCkpk8xip5PYwHQYD\n"
"VR0OBBYEFAVslVA5g+hwRYK03RfiGE5EstPDMAwGA1UdEwEB/wQCMAAwDgYDVR0P\n"
"AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQAFJg3db1rfYWNQhfg1BSGi6CX3\n"
"re4dmq7k8aVXyuHUmB4TD4UXzakOB8AHYXO8YM0CjwRy//vB08CyNQcNClqhjI5g\n"
"HYMMxvcV0hH9TWplxXvBgo9l24mH3DSZvIW02cjCMotdU1TEGNsMpGcRfBVWkARU\n"
"exiTUR9d14isk2Hw+H+W+4Nq1y0zHgL1sDh8m6l1zutsyoxZvHsB6fSAwbrMBGIq\n"
"nbH3Sf0vzKsLOTWWs0n/98Y/pwsMn91Sw6wvqeZQS7Yy22OiUCy40I04VQBAHkkL\n"
"r9TXS94nZTOgNcVaVAn+3cwK5Qi5WVJi/P+Nut1rAgXrpxcygPE0f5dNWdEj\n"
"-----END CERTIFICATE-----\n";

const char* private_key_pem =
"-----BEGIN RSA PRIVATE KEY-----\n"
"MIIEpAIBAAKCAQEArBKWnEBUit3/XnTJ+go7viyHBUJszCmsXk4dkhSJzpiFV9qh\n"
"Vh+smG3c8J9XpCGBbpKDcXxASs931ksAOA8/6VF2cKw+L5BeDrsDRZcmXw1Fl1an\n"
"sieVBUok6SD0sEFvpSDGvhMsQNBjemaOW/vWnEDdrRcB4aTE5InCtiJFQwGv+TEy\n"
"8PcOurs6ePLSmj6FocDyr7IQoVsONCYZ1HF/K6luBQFP0NsD7+gpTJCWxggnGzjF\n"
"gwPfGlWmw67eZM76HHoSBZQZ2gr8I+MrTf5883b6G+lj3KbCvLdPkx47vt06WX8o\n"
"VkcllbBQhIUbMnQErXPx6XikyJ1tZKaMTwandwIDAQABAoIBAHBuENR1QAggN0w7\n"
"WQE0EIcYxvwLw9S+Ye6Ycb/SBZzkkTOvGX2RYo+SMStTocsPTWe2+YV8HvICjxWI\n"
"mdMHO8YyQQYrtNIVLS/Ix8F/mkW/ATp6F0ZTkI4NPQLKUrabetxVByaIloCe6HDA\n"
"c8HsUfR0eaMwBj2uECwXSw8AY2r8qvdnzbp4EkJv6tLUHWo0SuqcAND4XCp+XfOm\n"
"futHkReKjNLjV97h1vCwpoZG0SI9IoujgzwnPhO9FDIKLLHlhYwstsrWgQvSTsDA\n"
"3pcKLtvconQjeAV9lOxzWXj7mIRqYQTQ59cclAheNARA6bc9MnkNQA1kDIZUYszS\n"
"YgdQBCECgYEA5NEacJpXKhsr4HhjSaDiUepcP9fizGKDjF7En6ypaxtvieCyIURL\n"
"udOUw6snAzu/0Rb2FD74J6U5pukg2jHpvaeSLQiWWWu+yxC+OHw2OXOUDgwxuYsJ\n"
"OyjJ9xtyRuE2ukNPEBe3oFH7U2liIZ962P59FzSHNirOlDefqaelBfECgYEAwIPA\n"
"L3vjlPPRcOIJFHcwU3TG3GkqsvQy2RyZNczMu7nsqQyIRVHAju+/WmvnZpajZNgo\n"
"XtN1SZGt2Set92eX8fAi8vZfEwg4geh0e5ZYvNktLXjfHZslUF+Sp3IflM4jGuIx\n"
"w8UT2d1ivGSx6XxDEjCvSVzZ48dDOP10s/n7++cCgYEAjWNoIww7sBj6E8pQD78K\n"
"GfPm7MxJqGF3R9WHSeM3DKf3VAs0Brpc0Iac9gtcH/Nbo6e8huYiEN1Xsnt7gVg2\n"
"Rqq0+H7F6JtbWb8Oy3h1SSrR4ZxHXY88NQIPSlH7WzDu0EmnIRqGrNn7op8LtBsH\n"
"ne4i/aPHqQDqhUuZrQuaPQECgYAWOyCmWxgcy0sUTgXeZyUdfg64xSw5HhoBGFxY\n"
"6h16UC4UF25lEtu5pdXjCzLdleeobY275Y9Vv4zj1sEwILZbKe/fPrQb7ocX+U/6\n"
"NZJpvGqLJboeIRxd+6tjyrn65RMIt7YndUqljfso5jflToQwRY74WdCPjSMmIZ6o\n"
"aeW/YQKBgQC1U4i98/w2SlYETClhMUC1IhrnfWg1jcnqcTeUYzVrwOyG8t25Bywk\n"
"PhizHZwGM9lbkVKDUb82hjdSJCBRkE7X8JBEZJkCj7osam1IFMJZ3B3DLtSxrw5L\n"
"UDCXzUOyaINs9ha/fwZIb2VeMSxZ/CpaSKWUYW+SOIoKtalH/Lb9EA==\n"
"-----END RSA PRIVATE KEY-----\n";

// --------------------
// ⚙️ 3. PINS RC522
// --------------------
#define SS_PIN 5
#define RST_PIN 22

MFRC522 rfid(SS_PIN, RST_PIN);

// --------------------
// 🔗 4. MQTT
// --------------------
WiFiClientSecure net;
PubSubClient client(net);

// --------------------
// 5. FUNCIONES WIFI Y AWS
// --------------------
void SetupWifi() {
  Serial.print("Conectando a WiFi...");
  WiFi.begin(ssid, password);
  int intentos = 0;
  while (WiFi.status() != WL_CONNECTED && intentos < 20) {
    delay(500);
    Serial.print(".");
    intentos++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi conectado!");
    Serial.print("IP: "); Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nNo se pudo conectar a WiFi.");
  }
}

void CheckWifi() {
  static unsigned long lastCheck = 0;
  const unsigned long checkInterval = 10000;
  unsigned long now = millis();
  if (now - lastCheck < checkInterval) return;
  lastCheck = now;

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi desconectado, reconectando...");
    WiFi.disconnect(true);
    delay(1000);
    WiFi.begin(ssid, password);
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Feedback recibido ["); Serial.print(topic); Serial.print("] ");
  String message;
  for (int i=0; i<length; i++) message += (char)payload[i];
  Serial.println(message);

  if (message.indexOf("OK") != -1) {
    Serial.println("-> Dato guardado correctamente (LED VERDE)");
  } else if (message.indexOf("ERROR") != -1) {
    Serial.println("-> ERROR al guardar dato (LED ROJO)");
  } else {
    Serial.println("-> Feedback desconocido");
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Conectando MQTT...");
    if (client.connect(THING_NAME)) {
      Serial.println("Conectado a AWS IoT!");
      client.subscribe(AWS_SUB_TOPIC);
    } else {
      Serial.print("Falló, rc=");
      Serial.print(client.state());
      Serial.println(" intentando en 5s");
      delay(5000);
    }
  }
}

void SetupAWS() {
  net.setCACert(aws_root_ca);
  net.setCertificate(certificate_pem);
  net.setPrivateKey(private_key_pem);

  client.setServer(AWS_IOT_ENDPOINT, 8883);
  client.setCallback(callback);
}

// --------------------
// 6. SETUP Y LOOP
// --------------------
void setup() {
  Serial.begin(115200);
  SPI.begin(18, 19, 23, 5);
  rfid.PCD_Init();
  Serial.println("RC522 listo");

  SetupWifi();
  SetupAWS();
}

void loop() {
  CheckWifi();
  if (!client.connected()) reconnect();
  client.loop();

  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial()) return;

  String uidString;
  for (byte i=0; i<rfid.uid.size; i++) {
    char temp[3];
    sprintf(temp, "%02X", rfid.uid.uidByte[i]);
    uidString += temp;
  }

  String payload = "{\"device_id\":\"" THING_NAME "\", \"card_id\":\"" + uidString + "\", \"timestamp\":\"" + String(millis()) + "\"}";
  Serial.print("Enviando: "); Serial.println(payload);

  if (client.publish(AWS_PUB_TOPIC, payload.c_str())) {
    Serial.println("-> Publicación MQTT OK");
  } else {
    Serial.println("-> ERROR al publicar MQTT");
  }

  rfid.PICC_HaltA();
  delay(1000);
}

