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

void setup(){
  Ethernet.begin(mac, ip);
  Serial.begin(9600);
  pixels.begin();
}

void loop(){
  String responseString = connectAndRead(); //connect to the server and read the output
  Serial.println(responseString);
  checkResponse(responseString);
  delay(10000); //wait 5 seconds before connecting again
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
  
  int commaIndex = -1;
  int secondCommaIndex = 0;
  
  for(int i=0; i < NUMPIXELS; i++){
    secondCommaIndex = commaIndex + 1;
    secondCommaIndex = response.indexOf(',',secondCommaIndex);
    if (secondCommaIndex > 0 ) {
        String color = response.substring(commaIndex + 1, secondCommaIndex);
        commaIndex = secondCommaIndex;
      
        Serial.println(secondCommaIndex);
        if (color == "red") {
            red = 50;
            green = 0;
            blue = 0;
        } else if (color == "blue" || color == "blue_anime") {
            red = 0;
            green = 50;
            blue = 0;
        } else if (color == "disabled") {
            red = 50;
            green = 50;
            blue = 50;
        } else {
            red = 10;
            green = 10;
            blue = 0;
        }
    } else {
      red = 10;
      green = 10;
      blue = 10;
    }
    
    
    pixels.setPixelColor(i, pixels.Color(red, green, blue));
    pixels.show();
  }
}
 
