#include <Ethernet.h>
#include <SPI.h>
#include <Adafruit_NeoPixel.h>

////////////////////////////////////////////////////////////////////////
//CONFIGURE
////////////////////////////////////////////////////////////////////////
byte server[] = { 213,174,47,236 }; //ip Address of the server you will connect to

//The location to go to on the server
//make sure to keep HTTP/1.0 at the end, this is telling it what type of file it is
String location = "/queo/index.php HTTP/1.0";

// if need to change the MAC address (Very Rare)
byte mac[] = { 0x90, 0xA2, 0xDA, 0x0E, 0x20, 0x78 };
////////////////////////////////////////////////////////////////////////
IPAddress ip(10,0,51,194);

#define PIN            6
#define NUMPIXELS      40

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
EthernetClient client;

char inString[255]; // string for incoming serial data
int stringPos = 0; // string index counter
boolean startRead = false; // is reading?
int delayval = 10000;
String colors[40];

void setup(){
  Ethernet.begin(mac, ip);
  Serial.begin(9600);
  pixels.begin();
}

void loop(){
  readAndShowHudsonData();
  delay(delayval); //wait 5 seconds before connecting again
}

void readAndShowHudsonData() {
  String responseString = connectAndRead(); //connect to the server and read the output
  checkResponse(responseString);
}

String connectAndRead(){
  //connect to the server

  Serial.println("connecting...");

  //port 80 is typical of a www page
  if (client.connect(server, 80)) {
    Serial.println("connected");
    client.print("GET ");
    client.println(location);
    client.println();

    //Connected - Read the page
    return readPage(); //go and read the output

  }else{
    return "connection failed";
  }

}

String readPage(){
  //read the page, and capture & return everything between '<' and '>'

  stringPos = 0;
  memset( &inString, 0, 255); //clear inString memory

  while(true){

    if (client.available()) {
      char c = client.read();

      if (c == '<' ) { //'<' is our begining character
        startRead = true; //Ready to start reading the part 
      }else if(startRead){
        if(c != '>'){ //'>' is our ending character
          inString[stringPos] = c;
          stringPos ++;
        }else{
          //got what we need here! We can disconnect now
          startRead = false;
          client.stop();
          client.flush();
          Serial.println("disconnecting.");
          return inString;
        }
      }
    }
  }
}

void checkResponse(String response) {
  int red = 0;
  int green = 0;
  int blue = 0;
  int factor = 1;
  int pixelRegister = 0;
  
  int commaIndex = -1;
  int secondCommaIndex = 0;
  int projectCount = 0;
  
  while(secondCommaIndex >= 0) {
    projectCount ++;
    
    secondCommaIndex = commaIndex + 1;
    secondCommaIndex = response.indexOf(',',secondCommaIndex);
    String color = response.substring(commaIndex + 1, secondCommaIndex);
    commaIndex = secondCommaIndex;
    
    colors[projectCount - 1] = color;
  }
  
  factor = NUMPIXELS / projectCount;
  
  for(int i = 0; i < projectCount; i++) {
    String colorString = colors[i];
    for (int j = 0; j <= factor -1; j++) {
       uint32_t color = getColorFromString(colorString);
       pixels.setPixelColor(pixelRegister, color);
       pixelRegister ++;
       pixels.show();
    }
  }
}

uint32_t getColorFromString(String colorString) {
      uint32_t color; 
      
      if (colorString == "red") {
          color = pixels.Color(50,0,0);
      } else if (colorString == "blue") {
          color = pixels.Color(0,50,0);
      } else if (colorString == "disabled") {
          color = pixels.Color(0,0,50);
      } else if (colorString == "red_anime" || colorString == "aborted_anime") {
          color = pixels.Color(100,40,0);
      } else if (colorString == "blue_anime") {
          color = pixels.Color(50,60,0);
      } else if (colorString == "aborted") {
        color = pixels.Color(50,0,50);
      } else {
          color = pixels.Color(10,10,0);
      }
      
      return color;
}

 
