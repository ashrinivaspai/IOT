#include <SPI.h>
#include <Ethernet.h>
#include "DHT.h"


#define DHTPIN 5
#define DHTOUT 6
#define DHTTYPE DHT11
#define PUMP 2
#define LIGHT 3
#define LDR A0
#define RAIN 4
#define SOIL A1
#define SOIL_TH 50//add
#define LIGHT_TH 50//add
#define HUMID 7
#define DEHUMID 8
#define HEATER 9
#define COOLER 10


DHT dht(DHTPIN,DHTOUT,DHTTYPE);

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192,168,1,177);
EthernetServer server(8080);  

String HTTP_req;  
String temperature;
String humidity;
int TEMPERATURE_TH=25;
int HUMIDITY_TH=50;    
boolean PUMP_STATUS = 0;
boolean LIGHT_STATUS=0;
int light,rain,soil;
boolean flag=0;
boolean man_pump=0;
boolean man_light=0;
boolean override_pump=0;
boolean override_light=0;

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
    pinMode(HEATER,OUTPUT);
    pinMode(COOLER,OUTPUT);
    pinMode(DEHUMID,OUTPUT);
    pinMode(HUMID,OUTPUT);
    digitalWrite(PUMP,LOW);
    digitalWrite(LIGHT,LOW);
    digitalWrite(HEATER,LOW);
    digitalWrite(COOLER,LOW);
    digitalWrite(HUMID,LOW);
    digitalWrite(DEHUMID,LOW);
}

void loop()
{
    delay(2000);
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    Serial.println(h);
    Serial.println(t);
    float dp=t-(100-h)/5;
    if (isnan(h) || isnan(t)) 
    {
      flag=1;
      Serial.println("failed to read DHT");
    }
    
    soil=analogRead(SOIL);
    soil=map(soil,0,1023,0,100);
    boolean soil_t=soil>SOIL_TH;
    rain=digitalRead(RAIN);
    PUMP_STATUS=soil_t&(rain);//turn on pump only if both are true

    light=analogRead(A0);//reading sensor value
    light=map(light,609,1015,0,100);//converting it to %
    LIGHT_STATUS=light>LIGHT_TH;
    
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
                    
                    client.println();
                    // send web page
                    client.println("<!DOCTYPE html>");
                    client.println("<html>");
                    client.println("<head>");
                    client.println("<style>");
                    //client.println("background: rgba(76,175,80,0.1);");
                    client.println("body{background-image:");
                    client.println("url('http://www.cerkezkoydogalgaz.net/wp-content/uploads/2013/04/BEYAZ.jpg');");
                    client.println("background-repeat:repeat-x; background-attachment:fixed; background-position:bottom left;}");
                    client.println(".txt_block{border: 1px solid black; width:380px; height: 240px; padding-left: 10px;padding-right: 10px; float: left; margin-right: 20px;margin bottom: 20px;");
                    client.println("}</style>");
                    client.println("<title>Smart Agriculture</title>");
                    client.println("</head>");
                    client.println("<body>");
                    client.println("<div align=\"center\">");
                    client.println("<h1>SMART AGRICULTURE</h1>");
                    client.println("</div>");
                    client.println("<div class=\"txt_block\">");
                    client.println("<p><h2>Click to control water pump and light.</h2></p>");
                    client.println("<Br>");
                    client.println("<form method=\"get\">");
                    ProcessCheckbox(client);
                    client.println("</form>");
                    if(1)
                    {
                      client.println("<div class=\"txt_block\">");
                      client.println("<h3>Weather Report</h3>");
                      client.println("<Br>");
                      client.println("Relative Humidity: ");
                      client.println(h);
                      client.println("%");
                      client.println("<Br>");
                      client.println("Temperature: ");
                      client.println(t);
                      client.println("<sup>O</sup>C<Br>");
                      flag=0;
                      if(t>TEMPERATURE_TH)
                      {
                        client.println("Cooler is ON<Br>");
                        digitalWrite(HEATER,LOW);
                        digitalWrite(COOLER,HIGH);
                      }
                      else if(t<TEMPERATURE_TH)
                      {
                        client.println("Heater is ON<Br>");
                        digitalWrite(COOLER,LOW);
                        digitalWrite(HEATER,HIGH);
                      }
                      else
                      {
                        client.println("Temperature is Maintained<Br>");
                        digitalWrite(COOLER,LOW);
                        digitalWrite(HEATER,LOW);
                      }

                      if(h>HUMIDITY_TH)
                      {
                        client.println("Dehumidifier is ON<Br>");
                        digitalWrite(DEHUMID,HIGH);
                        digitalWrite(HUMID,LOW);
                      }
                      else if(h<HUMIDITY_TH)
                      {
                        client.println("Humidifier is on<Br>");
                        digitalWrite(HUMID,HIGH);
                        digitalWrite(DEHUMID,LOW);
                      }
                      else
                      {
                        client.println("Humidity is maintained<Br>");
                        digitalWrite(HUMID,LOW);
                        digitalWrite(DEHUMID,LOW);
                      }
                    }
                    else
                    {
                      client.println("Failed to read DHT sensor");
                    }
                    client.println("<Br>");
                    if(soil_t)
                    {
                      client.println("SOIL MOISTURE IS LOW");
                    }
                    else
                    {
                      client.println("SOIL MOISTURE IS NORMAL");
                    }
                   
                    if(!rain)
                    {
                      client.println("<img src=\"http://www.moneycontrol.com/news_image_files/Rain-Clouds-190.jpg\"/>");
                      client.println("<p>It's Raining!</p>");
                    }
                    else
                    {
                      client.println("<img src=\"http://png-1.vector.me/files/images/6/8/688600/weather_icon_sunny_thumb\"/>");
                    }
                    client.println("</div>");
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
    }// end if (client)
    
    if(!override_light)
    {
      digitalWrite(LIGHT,LIGHT_STATUS);
    }
    
    if(!override_pump)
    {
      digitalWrite(PUMP,PUMP_STATUS);
    }
    
}


// switch PUMP and LIGHT and send back for checkbox
void ProcessCheckbox(EthernetClient cl)
{
    
    man_pump=HTTP_req.indexOf("PUMP=2")>-1;
    man_light=HTTP_req.indexOf("LIGHT=2")>-1;
    int index=HTTP_req.indexOf("temperature=");
    int indexa=0;
     
    
    if(HTTP_req.indexOf("PUMP_OVERRIDE=2")>-1)
    {
      PUMP_STATUS=man_pump;
      override_pump=1;
      cl.println("<input type=\"checkbox\" name=\"PUMP_OVERRIDE\" value=\"2\" \onclick=\"submit();\" checked> OVERRIDE PUMP");
    }
    else
    {
      PUMP_STATUS=PUMP_STATUS||man_pump;
      override_pump=0;
      cl.println("<input type=\"checkbox\" name=\"PUMP_OVERRIDE\" value=\"2\" \onclick=\"submit();\"> OVERRIDE PUMP");
    }
    cl.println("<Br>");
    
    if (PUMP_STATUS) 
    {    // switch PUMP on
        digitalWrite(PUMP, HIGH);
        // checkbox is checked
        cl.println("<input type=\"checkbox\" name=\"PUMP\" value=\"2\" \onclick=\"submit();\" checked>PUMP");
    }
    else
    {    // switch PUMP off
        digitalWrite(PUMP, LOW);
        // checkbox is unchecked
        cl.println("<input type=\"checkbox\" name=\"PUMP\" value=\"2\" \onclick=\"submit();\">PUMP");
    }
    cl.println("<Br>");
    

   if(HTTP_req.indexOf("LIGHT_OVERRIDE=2")>-1)
    {
      LIGHT_STATUS=man_light;
      override_light=1;
      cl.println("<input type=\"checkbox\" name=\"LIGHT_OVERRIDE\" value=\"2\" \onclick=\"submit();\" checked> OVERRIDE LIGHT");
    }
    else
    {
      LIGHT_STATUS=LIGHT_STATUS||man_light;
      override_light=0;
      cl.println("<input type=\"checkbox\" name=\"LIGHT_OVERRIDE\" value=\"2\" \onclick=\"submit();\"> OVERRIDE LIGHT");
    }
    cl.println("<Br>");
    
    if (LIGHT_STATUS) 
    {    // switch LIGHT on
        digitalWrite(LIGHT, HIGH);
        // checkbox is checked
        cl.println("<input type=\"checkbox\" name=\"LIGHT\" value=\"2\" \onclick=\"submit();\" checked>LIGHT");
    }
    else
    {    // switch LIGHT off
        digitalWrite(LIGHT, LOW);
        // checkbox is unchecked
        cl.println("<input type=\"checkbox\" name=\"LIGHT\" value=\"2\" \onclick=\"submit();\">LIGHT");
    }
    cl.println("</div>");


    if(1)//only if DHT sensor is active
    {
       if(HTTP_req.indexOf("temperature=")>-1)
     {
        indexa=index+12;
        temperature+=HTTP_req[indexa];
        indexa++;
        temperature+=HTTP_req[indexa];
        indexa=0;
        index=0;
        TEMPERATURE_TH=temperature.toInt();
        temperature="";
        
      }
      
      index=HTTP_req.indexOf("humidity=");
      if(index>-1)
      {
        indexa=index+9;
        humidity+=HTTP_req[indexa];
        indexa++;
        humidity+=HTTP_req[indexa];
        indexa=0;
        index=0;
        HUMIDITY_TH=humidity.toInt();
        humidity="";
      }
      cl.println("<div class=\"txt_block\">");
      
      cl.println("Enter the required temperature in <sup>0</sup>C<Br>");
 
      cl.println("<input type=\"text\" name=\"temperature\"><Br>");
      cl.println("Current setting for Temperature is ");
      cl.println(TEMPERATURE_TH);
      cl.println("<sup>0</sup>C<Br>");

 
      cl.println("Enter the required relative humidity level in %<Br>");
      cl.println("<input type=\"text\" name=\"humidity\"value=\"50\"><Br>");
      cl.println("Current setting for humidity is ");
      cl.println(HUMIDITY_TH);
      cl.println("%<Br>");
      cl.println("<input type=\"submit\" value=\"Submit\">");
    
      cl.println("</div>");
    }
        
}



