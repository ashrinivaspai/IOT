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
boolean PUMP_STATUS = 0;
boolean LIGHT_STATUS=0;
int light,rain,soil;
boolean override_pump=0;
boolean override_light=0;
boolean flag;
boolean man_pump=0;
boolean man_light=0;

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
      Serial.println("Failed to read from DHT sensor!\n");
      //return;
      flag=1;
    }
    
    Serial.print("Relative humidity: ");
    Serial.print(h);
    Serial.print(" %\t");
    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.println("*C\n");
    
    soil=analogRead(SOIL);
    boolean soil_t=soil>SOIL_TH;
    Serial.println(soil);
    rain=digitalRead(RAIN);
    PUMP_STATUS=soil_t|rain;//turn on pump if any is true
   /* if(!override_pump)
    {
      digitalWrite(PUMP,PUMP_STATUS);
      Serial.print("PUMP is ");
      Serial.println(PUMP_STATUS);
    }*/
    
    light=analogRead(A0);//reading sensor value
    light=map(light,609,1015,0,100);//converting it to %
    light=light>LIGHT_TH;
    if(!override_light)
    {
      digitalWrite(LIGHT,light);
      Serial.print("Light is ");
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
                    client.println("</head>");
                    client.println("<body>");
                    client.println("<div align=\"center\">");
                    client.println("<h1>SMART AGRICULTURE</h1>");
                    client.println("</div>");
                    client.println("<p>Click to control water pump and light.</p>");
                    client.println("<Br>");
                    client.println("<form method=\"get\">");
                    ProcessCheckbox(client);
                    client.println("<Br>");
                    client.println("</form>");
                    client.println("Weather Report");
                    client.println("Relative Humidity: ");
                    client.println(h);
                    client.println("%");
                    client.println("Temperature: ");
                    client.println(t);
                    client.println("<sup>O</sup>C");
                    client.println("<Br>");
                    if(soil_t)
                    {
                      client.println("SOIL MOISTURE IS LOW");
                    }
                    else
                    {
                      client.println("SOIL MOISTURE IS NORMAL");
                    }
                    client.println("<Br>");
                    if(!rain)
                    {
                      client.println("It's Raining");
                    }
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
    
    man_pump=HTTP_req.indexOf("PUMP=2")>-1;
     
    if(HTTP_req.indexOf("PUMP_OVERRIDE=2")>-1)
    {
      PUMP_STATUS=man_pump;
      cl.println("<input type=\"checkbox\" name=\"PUMP_OVERRIDE\" value=\"2\" \onclick=\"submit();\" checked> OVERRIDE PUMP");
      cl.println("<Br>");
    }
    else
    {
      PUMP_STATUS=PUMP_STATUS||man_pump;
      cl.println("<input type=\"checkbox\" name=\"PUMP_OVERRIDE\" value=\"2\" \onclick=\"submit();\"> OVERRIDE PUMP");
      cl.println("<Br>");
    }

    
    
    if (PUMP_STATUS) 
    {    // switch LED on
        digitalWrite(PUMP, HIGH);
        // checkbox is checked
        cl.println("<input type=\"checkbox\" name=\"PUMP\" value=\"2\" \onclick=\"submit();\" checked>PUMP");
        cl.println("on<Br>");
    }
    else
    {    // switch PUMP off
        digitalWrite(PUMP, LOW);
        // checkbox is unchecked
        cl.println("<input type=\"checkbox\" name=\"PUMP\" value=\"2\" \onclick=\"submit();\">PUMP");
        cl.println("off<Br>");
    }
   // Serial.print("pump is ");
   // Serial.println(PUMP_STATUS);

   /* if(HTTP_req.indexOf("LIGHT_OVERRIDE=2")>-1)
    {
      override_light=1;
      cl.println("<input type=\"checkbox\" name=\"LIGHT_OVERRIDE\" value=\"2\" \onclick=\"submit();\" checked> OVERRIDE LIGHT");
      cl.println("<Br>");
    }
    else
    {
      override_light=0;
      cl.println("<input type=\"checkbox\" name=\"LIGHT_OVERRIDE\" value=\"2\" \onclick=\"submit();\"> OVERRIDE LIGHT");
      cl.println("<Br>");
    }
    if (HTTP_req.indexOf("LIGHT=2") > -1) 
    {  // see if checkbox was clicked
        // the checkbox was clicked, toggle the Light
        LIGHT_STATUS=1;
    }
      // see if checkbox was clicked
        // the checkbox was clicked, toggle the Light
    else
        LIGHT_STATUS=0;
    
    if (LIGHT_STATUS)
    {    // switch Light on
        digitalWrite(LIGHT, HIGH);
        // checkbox is checked
        cl.println("<input type=\"checkbox\" name=\"LIGHT\" value=\"2\" onclick=\"submit();\" checked>LIGHT");
        cl.println("on\n");
    }
    else
    {    // switch Light off
        digitalWrite(LIGHT, LOW);
        // checkbox is unchecked
        cl.println("<input type=\"checkbox\" name=\"Light\" value=\"2\" onclick=\"submit();\">LIGHT");
        cl.println("off\n");
    }*/
}



