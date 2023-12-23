import paho.mqtt.client as mqtt
# import RPi.GPIO as GPIO
# from gpiozero import AngularServo
# import time
# 
# GPIO.setwarnings(False)
# GPIO.setmode(GPIO.BCM)
# 
# R_LED = 11
# G_LED = 12
# B_LED = 13
# 
# servoPin = 24
# 
# buzzer = 6
# 
# LED_1 = 2
# LED_2 = 3
# LED_3 = 8
# LED_4 = 9
# 
# GPIO.setup(buzzer, GPIO.OUT)
# 
# for x in range(11, 14):
#     GPIO.setup(x, GPIO.OUT)
# 
# for x in range(2,4):
#     GPIO.setup(x, GPIO.OUT)
# 
# for x in range(8,10):
#     GPIO.setup(x, GPIO.OUT)
#     
# 
# servo = AngularServo(servoPin, min_angle=-90, max_angle=90)
# 
# def map(x, in_min, in_max, out_min, out_max):
#     return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min

def on_connect(client, userdata, flags, rc):
    print("Connected with result code", str(rc))
    client.subscribe("MQTT/ESP32/LED")

def on_message(client, userdata, message):
    data = str(message.payload.decode("utf-8")).strip()
    print("Data received on topic:", message.topic, " | Message:", data)

#     if message.topic == "IOTIF/RPI/BurglarAlarm":
#         red = GPIO.HIGH if int(data[1]) == 1 else GPIO.LOW
#         green = GPIO.HIGH if int(data[0]) == 1 else GPIO.LOW
#         blue = GPIO.HIGH if int(data[2]) == 1 else GPIO.LOW
#         GPIO.output(R_LED, red)
#         GPIO.output(G_LED, green)
#         GPIO.output(B_LED, blue)
# 
#     elif message.topic == "IOTIF/RPI/Servo":
#         servo.angle = map(float(data), 0.0, 90, -90, 90)
#         
#     elif message.topic == "IOTIF/RPI/LED_1":
#         GPIO.output(LED_1, GPIO.HIGH if int(data == "ON") else GPIO.LOW)
#         
#     elif message.topic == "IOTIF/RPI/LED_2":
#         GPIO.output(LED_2, GPIO.HIGH if int(data == "ON") else GPIO.LOW)
#         
#     elif message.topic == "IOTIF/RPI/LED_3":
#         GPIO.output(LED_3, GPIO.HIGH if int(data == "ON") else GPIO.LOW)
#         
#     elif message.topic == "IOTIF/RPI/LED_4":
#         GPIO.output(LED_4, GPIO.HIGH if int(data == "ON") else GPIO.LOW)
#     
#     elif message.topic == "IOTIF/RPI/Buzzer":
#         GPIO.output(buzzer, GPIO.HIGH if int(data == "ON") else GPIO.LOW)

client = mqtt.Client()
client.connect("broker.hivemq.com")

client.on_connect = on_connect
client.on_message = on_message

try:
    client.loop_forever()
except KeyboardInterrupt:
    print("\nExiting the program\n")
    # GPIO.cleanup()
    exit()

