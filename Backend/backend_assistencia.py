import json
import time
import paho.mqtt.client as mqtt
import mysql.connector
import ssl # Necessari per a la versió TLS

# ---------------------------------------------------------------------
# 🔑 1. CONFIGURACIÓ DE CONNEXIÓ (AJUSTA AMB LES TEVES DADES)
# ---------------------------------------------------------------------

# Dades d'AWS IoT
AWS_IOT_ENDPOINT = "awo7krzvt5wzv-ats.iot.us-east-1.amazonaws.com" # L'endpoint que has recuperat
THING_NAME = "ESP32_Control_Assist"                             # Utilitza el mateix que a l'Arduino!
AWS_PUB_TOPIC = "itic/projecte1/entrades"                        # Tema on l'Arduino publica
AWS_FEEDBACK_TOPIC = "itic/projecte1/feedback/"                  # Base del tema per enviar la confirmació

# Dades de Certificats (RUTA ON ELS HAS GUARDAT AL TEU SERVIDOR)
# Assegura't que aquests fitxers existeixen al mateix directori que aquest script!
ROOT_CA = "AmazonRootCA1.pem"
CERTIFICATE = "c1ce253bde0d7fb92692fadc5dd36667d735277639455e4e8e2f351c69ac4cee-certificate.pem.crt"
PRIVATE_KEY = "c1ce253bde0d7fb92692fadc5dd36667d735277639455e4e8e2f351c69ac4cee-private.pem.key"

# Dades de Connexió MySQL (HAS D'AJUSTAR AIXÒ!)
MYSQL_HOST = "localhost"  # O la IP interna del teu servidor de BD (p. ex., 172.xx.xx.xx)
MYSQL_USER = "usuari_bd"
MYSQL_PASSWORD = "contrasenya_segura"
MYSQL_DATABASE = "projecte_assistencia"

# ---------------------------------------------------------------------
# ⚙️ 2. FUNCIONS D'MQTT I BASE DE DADES
# ---------------------------------------------------------------------

def db_insert_attendance(card_id, device_id):
    """Insereix la dada d'assistència a la BD i retorna el feedback (OK/ERROR)."""
    try:
        # Connexió a la Base de Dades
        mydb = mysql.connector.connect(
            host=MYSQL_HOST,
            user=MYSQL_USER,
            password=MYSQL_PASSWORD,
            database=MYSQL_DATABASE
        )
        mycursor = mydb.cursor()

        # SQL: Assegura't que el nom de la taula (assistencia) i columnes (card_uid, device_id, timestamp) són correctes
        sql = "INSERT INTO assistencia (card_uid, device_id, timestamp) VALUES (%s, %s, NOW())"
        val = (card_id, device_id)
        
        mycursor.execute(sql, val)
        mydb.commit()
        
        print(f"✅ Assistència registrada amb èxit: Card ID {card_id}")
        return "OK" # Retorna OK si la inserció és correcta

    except mysql.connector.Error as err:
        print(f"❌ ERROR DE BD: {err}")
        return "ERROR" # Retorna ERROR si hi ha un problema a la BD
    except Exception as e:
        print(f"❌ ERROR DESCONEGUT: {e}")
        return "ERROR"

def on_connect(client, userdata, flags, rc):
    """Funció que s'executa quan el client es connecta a AWS."""
    if rc == 0:
        print("🔗 Connectat a AWS IoT amb èxit.")
        # Subscriu-se al tema de les dades de l'Arduino per ESCOLTAR
        client.subscribe(AWS_PUB_TOPIC, qos=1) 
        print(f"👂 Escolant el tema: {AWS_PUB_TOPIC}")
    else:
        print(f"❌ Error de connexió a MQTT, rc={rc}")

def on_message(client, userdata, msg):
    """Funció que s'executa quan rep un missatge MQTT."""
    print(f"\n📦 Missatge rebut de {msg.topic}: {msg.payload.decode()}")
    
    # 1. Processar la dada (espero un JSON)
    try:
        data = json.loads(msg.payload.decode())
        card_id = data['card_id']
        device_id = data['device_id']
        
        # 2. Inserir a la Base de Dades
        feedback = db_insert_attendance(card_id, device_id)
        
        # 3. Enviar el Feedback de tornada a l'Arduino (al seu tema específic)
        topic_feedback = AWS_FEEDBACK_TOPIC + device_id 
        client.publish(topic_feedback, feedback, qos=1)
        print(f"📤 Feedback ({feedback}) enviat a: {topic_feedback}")

    except json.JSONDecodeError:
        print("❌ Error: Missatge rebut no és JSON vàlid.")
    except KeyError:
        print("❌ Error: Missatge rebut no conté 'card_id' o 'device_id'.")
    except Exception as e:
        print(f"❌ Error inesperat en on_message: {e}")


# ---------------------------------------------------------------------
# 🚀 3. EXECUCIÓ PRINCIPAL
# ---------------------------------------------------------------------

# Inicialitza el client MQTT
# Utilitzem client_id=THING_NAME per identificar el backend, igual que l'Arduino
mqtt_client = mqtt.Client(client_id=THING_NAME + "_Backend") 

# Configura les funcions de callback
mqtt_client.on_connect = on_connect
mqtt_client.on_message = on_message

# Configura la connexió segura (TLS) amb els certificats
try:
    mqtt_client.tls_set(
        ca_certs=ROOT_CA,
        certfile=CERTIFICATE,
        keyfile=PRIVATE_KEY,
        tls_version=ssl.PROTOCOL_TLSv1_2
    )

    # Connecta't a AWS IoT
    mqtt_client.connect(AWS_IOT_ENDPOINT, port=8883) # 8883 és el port segur per MQTT

    # Bucle d'escolta infinit (manté el client escoltant i reconectant)
    mqtt_client.loop_forever()
    
except FileNotFoundError:
    print("❌ ERROR: No s'han trobat els fitxers de certificat. Assegura't que estan al directori correcte.")
except Exception as e:
    print(f"❌ ERROR en la connexió TLS o MQTT inicial: {e}")

