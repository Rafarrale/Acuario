
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

MDNSResponder mdns;

#define LED_BUILTIN 2
#define IN1  D0
#define IN2  D1
#define IN3  D2
#define IN4  D3

char temp[1000]; //Pagina Web

String Boton1 = "OFF"; //Variable ON_Off boton1
String Boton2 = "OFF"; //Variable ON_Off boton2

const char* host = "esp8266-webupdate";
const char *ssid     = "Livebox-FF5C";
const char *password = "xwaAJLC7";

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

//  variables que cambiarán de valor.
int ledState = LOW;
int Paso [ 8 ][ 4 ] =
{ 
  {1, 0, 0, 0},
  {1, 1, 0, 0},
  {0, 1, 0, 0},
  {0, 1, 1, 0},
  {0, 0, 1, 0},
  {0, 0, 1, 1},
  {0, 0, 0, 1},
  {1, 0, 0, 1}
};

int steps_left = 4095;
boolean Direction = true;
int Steps = 0;                       // Define el paso actual de la secuencia


WiFiUDP ntpUDP;

int16_t utc = +2; //UTC +2:00 España
uint32_t currentMillis = 0;
uint32_t previousMillis = 0;
uint32_t previousMillis2 = 0;
uint32_t hora = 0;

NTPClient timeClient(ntpUDP, "a.st1.ntp.br", utc*3600, 60000);

void setup(void){

  Serial.begin(115200);
  Serial.println();
  Serial.println("Booting Sketch...");
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);

  while(WiFi.waitForConnectResult() != WL_CONNECTED){
    WiFi.begin(ssid, password);
    Serial.println("WiFi failed, retrying.");
  }

  MDNS.begin(host);
  if (mdns.begin("esp8266", WiFi.localIP())) {
    Serial.println("MDNS responder started");
  }
  
  // Start the server
  httpUpdater.setup(&httpServer);
  httpServer.on ( "/", handleRoot );
  httpServer.on("/socket1On", [](){
    handleRoot();
    digitalWrite(IN1, HIGH);
    Boton1 = "";
    Boton1 += "ON";
    delay(1000);
  });
  httpServer.on("/socket1Off", [](){
    handleRoot();
    digitalWrite(IN1, LOW);
    Boton1 = "";
    Boton1 += "OFF";
    delay(1000); 
  });
  httpServer.on("/socket2On", [](){
    handleRoot();
    digitalWrite(IN2, HIGH);
    Boton2 = "";
    Boton2 += "ON";
    delay(1000);
  });
  httpServer.on("/socket2Off", [](){
    handleRoot();
    digitalWrite(IN2, LOW);
    Boton2 = "";
    Boton2 += "OFF";
    delay(1000); 
  });
  httpServer.onNotFound ( handleNotFound );
  httpServer.begin();
  MDNS.addService("http", "tcp", 80);
  Serial.println ( "HTTP server started" );
  
  //--------------
  // El LED integrado está conectado al pin 2.
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  timeClient.begin();
  timeClient.update();
  hora = (timeClient.getHours() + 12) % 24; 
  //Serial.println(hora);
  
}

void loop(void){
  httpServer.handleClient();
  //Chama a verificacao de tempo
  checkOST();
/*  
//Metodo para probar el paso al Loop completo
if (currentMillis - previousMillis2 > 1000){
  // Si se cumple la condición se guarda el nuevo tiempo
  // en el que el LED cambia de estado
  previousMillis2 = currentMillis;
  // Y ahora cambiamos de estado el LED, si está encendido a
  // apagado o viceversa.
  if (ledState == LOW)
    ledState = HIGH;
  else
    ledState = LOW;
  // Hacemos que el contenido de la variable llegue al LED
  digitalWrite(LED_BUILTIN, ledState);
  }
 
*/
//Activo el alimentador cada 12 horas

 if (timeClient.getHours() == hora) {
    hora = (hora + 12) % 24;
    activaMotor();
    forceUpdate();
    Serial.println(hora);
  }
}

void forceUpdate(void) {
  timeClient.forceUpdate();
}

void checkOST(void) {
  currentMillis = millis();//Tiempo actual en ms
  //Lógica de verificación del tiempo
  if (currentMillis - previousMillis > 10000) {
    previousMillis = currentMillis;    // Salva el tiempo actual
//    printf("Time Epoch: %d: ", timeClient.getEpochTime());
    Serial.println(timeClient.getFormattedTime());
    
  }
}

void stepper()           //Avanza un paso
{
  digitalWrite( IN1, Paso[Steps][0] );
  digitalWrite( IN2, Paso[Steps][1] );
  digitalWrite( IN3, Paso[Steps][2] );
  digitalWrite( IN4, Paso[Steps][3] );
  SetDirection();
}

void SetDirection(){
  if (Direction)
    Steps++;
  else
    Steps--;
    
  Steps = ( Steps + 8 ) % 8 ;
}

void activaMotor(){
  int minutos = (timeClient.getMinutes() + 1) % 60;
  while(timeClient.getMinutes() <= minutos) {
    stepper() ;     // Avanza un paso
    delay (1) ; //Velocidad de giro del motor
  }
}

void handleRoot() {
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;

  snprintf ( temp, 1000,

"<html lang=\"en\"><head><meta http-equiv='refresh' content='5'\ name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/>\r\n\
  <title>Acuario ESP8266 Web Server</title>\
    <style>.c{text-align: center;} div,input{padding:5px;font-size:1em;} input{width:80%;} body{text-align: center;font-family:verdana;} button{border:0;border-radius:0.3rem;background-color:#1fa3ec;color:#fff;line-height:2.4rem;font-size:1.2rem;width:100%;} .q{float: right;width: 64px;text-align: right;}</style>\
     </head>\
      <body>\
        <h1>Hola desde tu acuario!</h1>\
        <p>Uptime: %02d:%02d:%02d</p>\
        <p>GPIO4 esta %s <a href=\"socket1On\"><button>ON</button></a><p><a href=\"socket1Off\"><button>OFF</button></a></p>\
        <p>GPIO5 esta %s <a href=\"socket2On\"><button>ON</button></a><p><a href=\"socket2Off\"><button>OFF</button></a></p>\
      </body>\
</html>",

    hr, min % 60, sec % 60, Boton1.c_str(),Boton2.c_str()
  );
  httpServer.send ( 200, "text/html", temp );
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += httpServer.uri();
  message += "\nMethod: ";
  message += ( httpServer.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += httpServer.args();
  message += "\n";

  for ( uint8_t i = 0; i < httpServer.args(); i++ ) {
    message += " " + httpServer.argName ( i ) + ": " + httpServer.arg ( i ) + "\n";
  }

  httpServer.send ( 404, "text/plain", message );
}
