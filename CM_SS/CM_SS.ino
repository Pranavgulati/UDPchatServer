/*
 Name:		CM_SS.ino
 Created:	10/1/2016 2:36:40 PM
 Author:	Pranav
*/


#include <ESP8266WiFi.h>
#include <WiFiUDP.h>
#define DEBUG 1
#define NUM_MAX_CLIENT 4 
#define CLIENT_PORT 4444	  // port to send UDP packets to clients
#define SERVER_PORT 8888      // local port to listen for UDP packets
#define SEPARATORbeg "."
#define SEPARATORend "."
/*
	format for chat string
	.name.message
*/

const char* serverSSID = "myServer";			//  your network SSID (name)
const char* password = "passphrase";       // your network password
WiFiUDP Udp;
byte clientCount = 0;

struct clientIdentity{
	String name;
	IPAddress ip;
}clientList[NUM_MAX_CLIENT];

void printWifiStatus() {
	// print the SSID of the network you're attached to:
	// print your WiFi shield's IP address:
	IPAddress ip = WiFi.softAPIP();
	Serial.print("IP Address: ");
	Serial.println(ip);
}

void sendMessage(String data, IPAddress sendIP) {
	Udp.beginPacket(sendIP, CLIENT_PORT);
	Udp.write(data.c_str());
	Udp.endPacket();
}
byte isClientList(String name){
	for (byte i = 0; i < NUM_MAX_CLIENT; i++){
		if (clientList[i].name.equals(name)){
			return 1;
		}
	}
	return 0;
}
byte addClient(String name, IPAddress clientIP){
	clientList[(clientCount) % NUM_MAX_CLIENT].name = name;
	clientList[(clientCount++) % NUM_MAX_CLIENT].ip = clientIP;
}
const char* removeName(String &test, String input){
	if (input.indexOf(SEPARATORbeg) != -1){

		byte start = input.indexOf(SEPARATORbeg), end = input.indexOf(SEPARATORend) + 1;
		test = input.substring(start, end);
		if (test.length() > 2){
			//valid name 
			input.remove(start, end);
			return input.c_str();
		}
		else{
			//invlaid name
			//invalid packet 
			return "0";
		}
		
	}
	else{
		//invalid packet 
		return "0";
	}

}
IPAddress getIP(String name){

	for (byte i = 0; i < NUM_MAX_CLIENT; i++){
		if (clientList[i].name.equals(name)){
			//found it here therefore we have an IP
			return clientList[i].ip;
		}
	}
	return IPAddress(0, 0, 0, 0);
}
void printClientNames(){
	for (byte i = 0; i < NUM_MAX_CLIENT; i++){
		if (clientList[i].name.length()>2){
			Serial.println(clientList[i].name);
		}
	}
}

void setup(){
	
	Serial.setDebugOutput(true);
	Serial.begin(115200);
	printWifiStatus();
	Serial.print("Udp server started at port ");
	Serial.println(SERVER_PORT);
	Serial.print("begin =");
	Serial.println(Udp.begin(SERVER_PORT));

	WiFi.mode(WIFI_AP);
	WiFi.softAP(serverSSID, password,11);
	delay(1000);

}

void loop(){	

	char inBuffer[32] = {}; //buffer to hold incoming and outgoing packets
	String outBuffer;
	int noBytes = Udp.parsePacket();
	if (noBytes) {
		String inputName;
		//parse the input name from the UDP packet
		Udp.read(inBuffer, noBytes); // read the packet into the buffer
		String temp;	
		temp = (String)removeName(inputName, String(inBuffer));
		if (temp.equals("0")){
			//invalid packet
		#ifdef DEBUG
			Serial.println("Invalid Packet Recvd");
		#endif
		}
		else{
		//valid packet
			strcpy(inBuffer, temp.c_str());
			if (isClientList(inputName)){
				Serial.print(inputName);
				Serial.println(inBuffer);
			}
			else{
#ifdef DEBUG
				Serial.print("\nnot found add to list ");
				Serial.println(Udp.remoteIP());
#endif
				addClient(inputName, Udp.remoteIP());
			}
		}
	}
	if (Serial.available()){
		//parse the serial input to send
		outBuffer = Serial.readString();
		String outName;
		outBuffer=removeName(outName, outBuffer);
		if (getIP(outName) != IPAddress(0, 0, 0, 0)){
			sendMessage(outBuffer, getIP(outName));
		}
		else{
			//no IP found 
			Serial.println("NO client found try the following names: ");
			printClientNames();
		}
	}
}
