#include <Ethernet.h>
#include <SPI.h>


#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>

#define PIN 6
#define NUMPIXELS 40
#define COLS 5
#define ROWS 8

//Matrix Conf
Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(5, 8, PIN, NEO_MATRIX_TOP + NEO_MATRIX_RIGHT + NEO_MATRIX_COLUMNS + NEO_MATRIX_PROGRESSIVE, NEO_GRB + NEO_KHZ800);
const uint32_t textColor = matrix.Color(150, 0, 0);

//Ethernet Conf
byte server[] = { 213,174,47,236 };
String location = "/queo/index.php HTTP/1.0";

//Check MAC on back of Arduino
byte mac[] = { 0x90, 0xA2, 0xDA, 0x0E, 0xD3, 0x5F };    // 3. OG
//byte mac[] = { 0x90, 0xA2, 0xDA, 0x0E, 0x20, 0x78 };  // 2. OG

//First IP belongs to first MAC
IPAddress ip(10,0,51,193);    // 3. OG
//IPAddress ip(10,0,51,194);  // 2. OG

EthernetClient client;

//Total count of projects
int projectCount = 0; 
//count of failed projects
int failedProjectCount = 0;
//delay between loops
int delayval = 30000;
// array of colors
uint16_t colors[40];
// array for failed project names
String failedProjects[40];
//show text of failed projects
boolean showFailedProjects = true;

void setup() {
  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(40);
  matrix.setTextColor(textColor);
  matrix.fillScreen(0);
  
  Ethernet.begin(mac, ip);
  Serial.begin(9600);
}

void loop() {
    doMagic();
    delay(delayval);
}

void doMagic() {
  connectAndRead();
}

void connectAndRead(){
  Serial.println("connecting...");
  if (client.connect(server, 80)) {
    Serial.println("connected");
    client.print("GET ");
    client.println(location);
    client.println();
    //Connected - read data
    readAndShowData();
  } else {
    Serial.println("connection failed");
    matrix.fillScreen(0);
    matrix.setCursor(0, 0);
    matrix.print(("!"));
    matrix.show();
  }
}

int readAndShowData(){
    //read the data, and capture & return everything between '<' and '>'
    // everything between '+' and '#' will be captured as red project, every following number als color index
    
    int projPos = 0;
    int x = 0;
    char project[30];
    projectCount = 0; 
    failedProjectCount = 0;
    boolean readRedProjects = false;
    boolean startRead = false;
    
    memset( &project, 0, 30); //clear project memory
    
    x = matrix.width();
    matrix.fillScreen(0);
    
    while(true){
        if (client.available()) {
            char c = client.read();
            if (c == '<' ) {
                startRead = true; //Ready to start reading the part
            }else if(startRead){
                if(c != '>'){ //'>' is our ending character
                    if( c == '+')  { //'+' is the beginning character for red project names
                        readRedProjects = true;
                    } else if (readRedProjects) {
                        if (c != '#') {//'#' is the ending character for red project names
                            if (c != ',') {//red projects are comma separated
                                //add character to project name
                                project[projPos] = c;
                                projPos ++;
                            } else {
                                //add project name to failedProjects array
                                failedProjects[failedProjectCount] = project;
                                failedProjectCount ++;
                                memset( &project, 0, 20);
                                projPos = 0;
                            }
                        } else {
                            readRedProjects = false;
                        }
                      } else {
                          if (c != ',') {//color indexes are comma separated
                            //add color to colors
                            colors[projectCount] = getColorFromId(c);
                            projectCount ++;
                          }
                      }
                    } else {
                    //got what we need here! We can disconnect now
                    startRead = false;
                    client.stop();
                    client.flush();
                    Serial.println("disconnecting.");
                    //continiue with saved data;
                    printFailedProjects();
                    return true;
                }
            }
        }
    }
}

void printFailedProjects() {
  int x;
   
  //just call this part every second run
  if (showFailedProjects == true) {
    
    //loop trough each project text
    for(int i = 0; i < failedProjectCount; i++) {
      int totalShifts = - ((failedProjects[i].length() + 3) * COLS);   // Total shifts needed
                                  
      x = matrix.width();
      matrix.setCursor(x, 0);
      matrix.fillScreen(0);
      
      Serial.println(failedProjects[i]);
      
      //print text to matrix and shift x-cursor
      for(int j = x; j > totalShifts; j--) {
          matrix.fillScreen(0);
          matrix.setCursor(j, 0);
          matrix.print((failedProjects[i]));
          matrix.show();
          delay(75);
      }
    }
  
    showFailedProjects = false;
  } else {
    showFailedProjects = true;
  }
  
  //show Dots on matrix after text is finisched
  drawProjectsOnMatrix();
}

void drawProjectsOnMatrix() {
  int factor = 1;
  int rest = 0;
  int x = 0;
  int y = 0;
  
  //how many leds per project
  factor = NUMPIXELS / projectCount;
  //unused leds
  rest = NUMPIXELS - projectCount * factor;
  
   matrix.fillScreen(0);
   matrix.setCursor(0, 0);
  
  for(int i = 0; i < projectCount; i++) {
    int actualFactor = factor;
    
    //spare rest to leds until empty
    if (rest > 0) {
       actualFactor ++;
       rest --;
     }
     
    for (int j = 0; j <= actualFactor -1; j++) {
       matrix.drawPixel(x,y,colors[i]);
       
       //vertical alignment
       if (y == ROWS -1) {
         y = 0;
         x ++;
       } else {
         y ++;
       }
    }
    
    matrix.show();
  }
}

uint32_t getColorFromId(char colorId) {
  
      uint32_t color; 
      
      //red - fail
      if (colorId == '1') {
          color = matrix.Color(180,0,0);
      //blue - ok, blue_anime
      } else if (colorId == '2' || colorId == '6') {
          color = matrix.Color(0,80,0);
      //disabled
      } else if (colorId == '3') {
          color = matrix.Color(0,0,150);
      //red_anime, aborted_anime
      } else if (colorId == '4' || colorId == '5') {
          color = matrix.Color(255,165,0);
      //aborted
      } else if (colorId == '7') {
        color = matrix.Color(0,120,160);
      }
      //yellow = unstable
      else if (colorId == '8') {
        color = matrix.Color(155,155,0);
      } else {
          color = matrix.Color(150,0,150);
      }
      
      return color;
}
