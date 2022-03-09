#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>

byte buf[255];
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

// set arduino ip
IPAddress ip(192, 168, 0, 110);

// set server to listen for http requests
EthernetServer server(80);

// allocate file memory
File webFile;

char HTTP_req[20];

//order of requests
char titles[3][13] = {
  " H",
  "favicon.ico",
  "RiskGame.iso"
};
char headers [3][6] = {
  "a.txt",
  "b.txt",
  "c.txt"
};


void setup() {
  //shield is on pin ten
  Ethernet.init(10);
  Serial.begin(9600);
  while (!Serial);
  Serial.println("Risk II Netplay");
  if(!SD.begin(4)){
    Serial.println("Failed to initiate SD card.");
  }
  //ethernet connection has mac and ip
  Ethernet.begin(mac, ip);
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
  }
  //start server
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
}


void loop() {
  EthernetClient client = server.available();
  if (client) {
    boolean currentLineIsBlank = true;
    boolean unreadLine = true;
    while (client.connected()) {
      if (client.available()) {
        char c;
        if(unreadLine){
          int len = client.readBytes(HTTP_req, 20);
          Serial.write(HTTP_req, len);
          c = HTTP_req[len - 1];
          unreadLine = false;
        }else{
          c = client.read();
          Serial.write(c);
        }
        if (c == '\n' && currentLineIsBlank) {
          char *loc = strstr(HTTP_req, "GET /");
          if(loc != NULL && int(loc - HTTP_req) == 0){
            for(byte i = 0; i < 3; i ++){
              char *loc = strstr(HTTP_req, titles[i]);
              if(loc != NULL && int(loc - HTTP_req) == 5){
                webFile = SD.open(headers[i]);
                while(webFile.available()){
                  client.write(webFile.read());
                }
                webFile.close();
                
                if(i==0){
                  webFile = SD.open("index.htm");
                }else{
                  webFile = SD.open(titles[i]);
                }
                client.print("Content-Length: ");
                client.println(webFile.size());
                client.println();
                
                while(webFile.available()){
                  client.write(buf, webFile.read(buf, 255));
                }
                webFile.close();
                break;
              }
            }
          }
          unreadLine = true;
          break;
        }
        if (c == '\n') {
          currentLineIsBlank = true;
        } else if (c != '\r') {
          currentLineIsBlank = false;
        }
      }
    }
    delay(1);
    client.stop();
  }
}
