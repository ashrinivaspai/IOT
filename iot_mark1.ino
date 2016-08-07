#include <SPI.h>
#include <Ethernet.h>
#include "DHT.h"

#define DHTPIN 5
#define DHTTYPE DHT11
#define PUMP 2
#define LIGHT 3
#define LDR A0
#define RAIN 4
#define SOIL A1
#define SOIL_TH 50//add
#define LIGHT_TH 50//add

DHT dht(DHTPIN,DHTTYPE);

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192,168,1,177);
EthernetServer server(80);  

String HTTP_req;          
boolean PUMP_status = 0;
boolean LED_status=0;
int light,rain,soil;
boolean override_pump=0;
boolean override_light=0;
int man_pump=0;
int man_light=0;

void setup()
{
    Ethernet.begin(mac, ip); 
    server.begin();
    dht.begin();        
    Serial.begin(9600);       
    pinMode(PUMP, OUTPUT);
    pinMode(LIGHT,OUTPUT);      
    pinMode(LDR,INPUT);
    pinMode(SOIL,INPUT);
    pinMode(RAIN,INPUT);
    digitalWrite(PUMP,LOW);
    digitalWrite(LIGHT,LOW);
}

void loop()
{
    delay(2000);
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    float dp=t-(100-h)/5;
    if (isnan(h) || isnan(t)) 
    {
      Serial.println("Failed to read from DHT sensor!");
      //return;
      int flag=1;
    }
    Serial.print("Relative humidity: ");
    Serial.print(h);
    Serial.print(" %/t");
    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.println("*C");
    
    soil=analogRead(SOIL)<SOIL_TH;//high if water level is low
    rain=digitalRead(RAIN);
    int water=soil|rain;//turn on pump if any is true
    if(!override_pump)
    {
      digitalWrite(PUMP,water);
      Serial.print("PUMP is ");
      Serial.println(water);
    }
    
    light=analogRead(A0);//reading sensor value
    light=map(light,609,1015,0,100);//converting it to %
    light=light>light;
    if(!override_light)
    {
      digitalWrite(LIGHT,light);
      Serial.print("lamp is ");
      Serial.println(light);
    } 
    
    EthernetClient client = server.available();
    if (client) 
    {  // got client?
        boolean currentLineIsBlank = true;
        while (client.connected()) 
        {
            if (client.available()) 
            {   // client data available to read
                char c = client.read(); // read 1 byte (character) from client
                HTTP_req += c;  // save the HTTP request 1 char at a time
                // last line of client request is blank and ends with \n
                // respond to client only after last line received
                if (c == '\n' && currentLineIsBlank)
                {
                    // send a standard http response header
                    client.println("HTTP/1.1 200 OK");
                    client.println("Content-Type: text/html");
                    client.println("Connection: close");
                    client.println("Refresh: 5");
                    client.println();
                    // send web page
                    client.println("<!DOCTYPE html>");
                    client.println("<html>");
                    client.println("<head>");
                    client.println("<title>Smart Agricultue</title>");
                    client.println("</head>");
                    client.println("<body>");
                    client.println("<h1></h1>");
                    client.println("<p>Click to control water pump and light.</p>");
                    client.println("<form method=\"get\">");
                    ProcessCheckbox(client);
                    client.println("<Br>");
                    client.println("</form>");
                    client.println("Weather Report");
                    client.print("Relative Humidity: ");
                    client.print(h);
                    client.println("%");
                    client.print("Temperature: ");
                    client.println("</body>");
                    client.println("</html>");
                    Serial.print(HTTP_req);
                    HTTP_req = "";    // finished with request, empty string
                    break;
                }
                // every line of text received from the client ends with \r\n
                if (c == '\n')
                {
                    // last character on line of received text
                    // starting new line with next character read
                    currentLineIsBlank = true;
                } 
                else if (c != '\r')
                {
                    // a text character was received from client
                    currentLineIsBlank = false;
                }
            } // end if (client.available())
        } // end while (client.connected())
        delay(1);      // give the web browser time to receive the data
        client.stop(); // close the connection
    } // end if (client)
}

// switch PUMP and LIGHT and send back for checkbox
void ProcessCheckbox(EthernetClient cl)
{
    if (HTTP_req.indexOf("PUMP=2") > -1)
    {  // see if checkbox was clicked
        // the checkbox was clicked, toggle the PUMP
        PUMP_status=1;
    }
      // see if checkbox was clicked
        // the checkbox was clicked, toggle the PUMP
        else
        PUMP_status=0;
    
    if (PUMP_status) 
    {    // switch LED on
        digitalWrite(PUMP, HIGH);
        // checkbox is checked
        cl.println("<input type=\"checkbox\" name=\"PUMP\" value=\"2\" \onclick=\"submit();\" checked>PUMP");
        cl.println("on<Br>");
    }
    else
    {              // switch LED off
        digitalWrite(PUMP, LOW);
        // checkbox is unchecked
        cl.println("<input type=\"checkbox\" name=\"PUMP\" value=\"2\" \onclick=\"submit();\">PUMP");
        cl.println("off<Br>");
    }
    if (HTTP_req.indexOf("LAMP=2") > -1) 
    {  // see if checkbox was clicked
        // the checkbox was clicked, toggle the LED
        LED_status=1;
    }
      // see if checkbox was clicked
        // the checkbox was clicked, toggle the LED
        else
        LED_status=0;
    
    if (LED_status)
    {    // switch LED on
        digitalWrite(LIGHT, HIGH);
        // checkbox is checked
        cl.println("<input type=\"checkbox\" name=\"LAMP\" value=\"2\" \onclick=\"submit();\" checked>LAMP");
        cl.println("on\n");
    }
    else
    {              // switch LED off
        digitalWrite(LIGHT, LOW);
        // checkbox is unchecked
        cl.println("<input type=\"checkbox\" name=\"LAMP\" value=\"2\" \onclick=\"submit();\">LAMP");
        cl.println("off\n");
    }
}



