import sys
import time
import serial
from Adafruit_IO import MQTTClient 

# Configuración de Adafruit IO
ADAFRUIT_IO_USERNAME = "..." #por seguridad no hay clave
ADAFRUIT_IO_KEY = "..." #por seguridad no hay clave

# Configuración de los feeds
FEED_ID_receiveS1 = 'Counter_TX_S1'
FEED_ID_SendS1 = 'Counter_RX_S1'
FEED_ID_receiveS2 = 'Counter_TX_S2'
FEED_ID_SendS2 = 'Counter_RX_S2'
FEED_ID_receiveS3 = 'Counter_TX_S3'
FEED_ID_SendS3 = 'Counter_RX_S3'
FEED_ID_receiveS4 = 'Counter_TX_S4'
FEED_ID_SendS4 = 'Counter_RX_S4'

# Mapeo de feeds TX a RX
FEED_MAPPING = {
    FEED_ID_receiveS1: FEED_ID_SendS1,
    FEED_ID_receiveS2: FEED_ID_SendS2,
    FEED_ID_receiveS3: FEED_ID_SendS3,
    FEED_ID_receiveS4: FEED_ID_SendS4
}

def connected(client):
    """Función llamada cuando se conecta a Adafruit IO"""
    print('Subscribing to Feed {0}'.format(FEED_ID_receiveS1))
    client.subscribe(FEED_ID_receiveS1)
    print('Subscribing to Feed {0}'.format(FEED_ID_receiveS2))
    client.subscribe(FEED_ID_receiveS2)
    print('Subscribing to Feed {0}'.format(FEED_ID_receiveS3))
    client.subscribe(FEED_ID_receiveS3)
    print('Subscribing to Feed {0}'.format(FEED_ID_receiveS4))
    client.subscribe(FEED_ID_receiveS4)
    print('Waiting for feed data...')

def disconnected(client):
    """Función llamada cuando se desconecta"""
    sys.exit(1)

def message(client, feed_id, payload):
    """Maneja los mensajes recibidos de Adafruit IO"""
    print('Feed {0} received new value: {1}'.format(feed_id, payload))
    
    # Encontrar el feed RX correspondiente
    rx_feed = FEED_MAPPING.get(feed_id)
    if rx_feed:
        try:
            # Enviar comando al Arduino (formato "S1=90")
            servo_cmd = f"S{feed_id[-1]}={payload}\n"
            miarduino.write(servo_cmd.encode())
            print(f"Sent to Arduino: {servo_cmd.strip()}")
            
            # Esperar confirmación con timeout de 1 segundo
            start_time = time.time()
            while time.time() - start_time < 1:
                if miarduino.in_waiting:
                    try:
                        # Leer respuesta con manejo de errores de decodificación
                        response = miarduino.readline().decode('utf-8', errors='ignore').strip()
                        if response.startswith("OK"):
                            print(f"Arduino confirmed: {response}")
                            client.publish(rx_feed, payload)
                            break
                    except Exception as e:
                        print(f"Error reading response: {str(e)}")
                        break
            else:
                print("Error: Timeout waiting for Arduino response")
        except Exception as e:
            print(f"Error sending command: {str(e)}")

# Inicialización con manejo de errores
try:
    client = MQTTClient(ADAFRUIT_IO_USERNAME, ADAFRUIT_IO_KEY)
    
    # Configurar conexión serial con timeout
    miarduino = serial.Serial(
        port='COM9', 
        baudrate=9600,
        timeout=1  # Timeout de 1 segundo para las operaciones de lectura
    )
    
    # Configurar callbacks
    client.on_connect = connected
    client.on_disconnect = disconnected
    client.on_message = message

    # Conectar
    client.connect()
    client.loop_background()

    print("Sistema listo. Esperando movimientos...")

    # Bucle principal con manejo de excepciones
    while True:
        try:
            # Leer respuestas del Arduino si las hay
            if miarduino.in_waiting:
                try:
                    response = miarduino.readline().decode('utf-8', errors='ignore').strip()
                    if response:  # Solo mostrar si hay contenido
                        print(f"Received from Arduino: {response}")
                except Exception as e:
                    print(f"Error reading serial data: {str(e)}")
            
            time.sleep(0.1)  # Pequeña pausa para no saturar la CPU
            
        except KeyboardInterrupt:
            print("\nCerrando conexiones...")
            miarduino.close()
            client.disconnect()
            sys.exit(0)
            
        except Exception as e:
            print(f"Error in main loop: {str(e)}")
            time.sleep(1)  # Esperar antes de reintentar

except serial.SerialException as e:
    print(f"Error al abrir el puerto serial: {str(e)}")
    sys.exit(1)

except Exception as e:
    print(f"Error de inicialización: {str(e)}")
    sys.exit(1)