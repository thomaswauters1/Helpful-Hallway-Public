# Code for Raspberry Pi
This is the code that will handel the receiving messages and send it to the database.  
InfluxDB will (probably) be used for the database which will be connected to Grafana for the visualisation.  
I think we should be able to run everything on one Raspberry Pi on a remote private network.  
The things running on the server will be:
* This code
* The MQTT broker (Mosquito)
* The database (InfluxDB)
* Visualisation application (Grafana)
* Software for dynamic DNS (Probably DuckDNS)
