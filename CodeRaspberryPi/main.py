import paho.mqtt.client as cl
import influxdb as db
import json as json 
import os

################################################################################################################################
#
#   This is the main file that has to run on the raspberry pi on the edge of the network where we measure.
#   Besides this python program, the Mosquitto mqtt broker should also run on this raspberri pi.
#
################################################################################################################################


def makeJsonW(bakID, koeID, volume, timestampESP32):
    data =  {
        "measurement": "meting",
        "tags": {
            "koeID": "5",
            "bakID": "1",
            "timestampEsp32": "2020-02-18T16:12:34Z"
        },
        "fields": {
            "volume": 32.0
        }
    }
    data["tags"]["bakID"] = bakID
    data["tags"]["koeID"] = koeID
    data["tags"]["timestampEsp32"] = timestampESP32
    data["fields"]["volume"] = volume
    return data

def makeJsonHH(HallwayID, MicValue, TrafficValue):
    data = {
        "measurement": "meting",
        "tags": {
            "HallwayID":"1",    #Or follow the structure from university like B230
        },
        "fields": {
            "MicValue": 32.0,
            "TrafficValue": 5
        }
    }
    data["tags"]["HallwayID"] = HallwayID
    data["fields"]["MicValue"] = MicValue
    data["fields"]["TrafficValue"] = TrafficValue
    return data

def on_message(clientMqtt, userdata, msg):
    print("Topic: ", msg.topic + "\nMessage: " + str(msg.payload.decode("utf-8")))

def on_connect(clientMqtt, userdata, flags, rc):
    # rc is the error code returned when connecting to the broker
    print("Connected!", str(rc))
    # Once the client has connected to the broker, subscribe to the topic
    clientMqtt.subscribe(mqtt_topicW)
    clientMqtt.subscribe(mqtt_topicHH)

    
def on_messageW(clientMqtt, userdata, msg):
    client.switch_database(databaseW)
    print("Topic: ", msg.topic + "\nMessage: " + str(msg.payload.decode("utf-8")))

    try:
        #Received packet to json format
        data = msg.payload.decode("utf-8").split(";")               #vb:  3;0;30.0;2020-02-18T16:12:34Z
        bakID = data[0]
        koeID =  data[1]
        volume = float(data[2])                                    #Has to be type float
        timestampESP32 = data[3]
        packet = makeJsonW(bakID, koeID, volume, timestampESP32)
        
        path = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'data.json')
        #JSON file now still useless but code more or less ready to send multiple datapoints in at once
        #get saved data from data.json
        if os.stat(path).st_size == 0:
            #if data.json empty, load in file
            d = "["+json.dumps(packet)+"]"
            data1 = json.loads(d)
        else:
            with open(path, 'r') as outfile:
                #if data in data.json load and append data
                data1 = json.load(outfile)
            data1.append(packet)

        #data to file
        with open(path, 'w') as outfile:
            json.dump(data1, outfile, sort_keys=True, indent=4)

    except ValueError:
        print("Er is een fout opgetreden: ValueError")
    except IndexError:
        print("Er is een fout opgetreden: IndexError")

    #send data
    client.write_points(data1)                      

    #clear data.json after send
    with open(path, 'r+') as outfile:
        outfile.truncate(0)

def on_messageHH(clientMqtt, userdata, msg):
    # This function is called everytime the topic is published to.
    # If you want to check each message, and do something depending on
    # the content, the code to do this should be run in this function
    client.switch_database(databaseHH)
    print("Topic: ", msg.topic + "\nMessage: " + str(msg.payload.decode("utf-8")))
    #Handling of arrived messages happens here.
    data = msg.payload.decode("utf-8").split(";")  		#Format:  HallwayID;MicValue;TrafficID (vb. 5;250;2)    

    HallwayID = data[0]     			
    MicValue = float(data[1])      
    # CameraID = data[2]    we can just put this in HallwayID
    TrafficValue= float(data[2])      

    #put data in json format
    entry = makeJsonHH(HallwayID, MicValue, TrafficValue)
    #post to database
    e = "["+json.dumps(entry)+"]"	#makes complete json from dictionary
    ent = json.loads(e)
    client.write_points(ent)

#Mqtt data, change to settings mosquitto
# mqtt_username = "iujtbwbp"
# mqtt_password = "UyBkMSx2TYnv"
# mqtt_topic = "HelpfulHallway"
# mqtt_broker_ip = "farmer.cloudmqtt.com"
# mqtt_port = 15383
#For Mosquitto
mqtt_broker_ip = "wateropname.duckdns.org"
mqtt_topicW = "Wateropname"
mqtt_topicHH = "HelpfulHallway"
mqtt_port = 1883

#Mqtt client
clientMqtt = cl.Client()
#clientMqtt.username_pw_set(mqtt_username, mqtt_password)

databaseW = "wateropnameKoeien"
databaseHH = "HelpfulHallway"

#Database client
client = db.InfluxDBClient(host='localhost', port=8086)

clientMqtt.message_callback_add(mqtt_topicW, on_messageW)
clientMqtt.message_callback_add(mqtt_topicHH, on_messageHH)

clientMqtt.on_connect = on_connect
clientMqtt.on_message = on_message
# Once everything has been set up, we can (finally) connect to the broker
clientMqtt.connect(mqtt_broker_ip, mqtt_port)

#Send Hello world!
clientMqtt.publish(mqtt_topicW, payload="Hello World!", qos=0, retain=False)
clientMqtt.publish(mqtt_topicHH, payload="Hello World!", qos=0, retain=False)

# Once we have told the client to connect, let the client object run itself
#client.loop_forever()

while True:
    #check for incoming messages
    clientMqtt.loop_start()

    #extra code excuteded while running has to be placed here (I think)

#Dissconnect client, code never comes here but I'm not 100% sure if I can delete this
clientMqtt.disconnect()
