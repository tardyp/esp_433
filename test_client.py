import paho.mqtt.client as mqtt
import ssl
import logging
logging.basicConfig(format='%(levelname)s:%(message)s', level=logging.DEBUG)

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe("#")
    client.publish("projects/homeassistant-161315/topics/hass", 'coucou')

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    print(msg.topic+" "+str(msg.payload))

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.enable_logger()

client.tls_set(ca_certs=None, certfile='keys/cert.crt', keyfile='keys/rsa_private.pem', cert_reqs=ssl.CERT_REQUIRED,
    tls_version=ssl.PROTOCOL_TLS, ciphers=None)

client.connect("mqtt.googleapis.com", 8883, 60)

# Blocking call that processes network traffic, dispatches callbacks and
# handles reconnecting.
# Other loop*() functions are available that give a threaded interface and a
# manual interface.
client.loop_forever()