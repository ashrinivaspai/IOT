#include <SPI.h>
#include <Ethernet.h>

// MAC address from Ethernet shield sticker under board
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192,168,1,177); // IP address, may need to change depending on network
EthernetServer server(80);  // create a server at port 80

String HTTP_req;          // stores the HTTP request
boolean LED_status = 0;
boolean LED_status1=0;// state of LED, off by default
int x,light;// to store sensor value
void setup()
{
    Ethernet.begin(mac, ip);  // initialize Ethernet device
    server.begin();           // start to listen for clients
    Serial.begin(9600);       // for diagnostics
    pinMode(2, OUTPUT);
    pinMode(3,OUTPUT);      // LED on pin 2
    pinMode(4,INPUT);// to connect sensor like LDR
}

void loop()
{
    EthernetClient client = server.available();  // try to get client
    x=analogRead(A0);//reading sensor value
    light=map(x,609,1015,0,100);//converting it to %
    if (client) {  // got client?
        boolean currentLineIsBlank = true;
        while (client.connected()) {
            if (client.available()) {   // client data available to read
                char c = client.read(); // read 1 byte (character) from client
                HTTP_req += c;  // save the HTTP request 1 char at a time
                // last line of client request is blank and ends with \n
                // respond to client only after last line received
                if (c == '\n' && currentLineIsBlank) {
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
                    client.println("<title>Arduino LED Control</title>");
                    client.println("</head>");
                    client.println("<body>");
                    client.println("<h1>LED</h1>");
                    client.println("<p>Click to switch LED on and off.</p>");
                    client.println("<form method=\"get\">");
                    ProcessCheckbox(client);
                    client.println("<Br>");
                    client.println(light);//printing sensor value after using map() function
                    client.println(x);//printing sensor value
                    client.println("</form>");
                    client.println("</body>");
                    client.println("</html>");
                    Serial.print(HTTP_req);
                    HTTP_req = "";    // finished with request, empty string
                    break;
                }
                // every line of text received from the client ends with \r\n
                if (c == '\n') {
                    // last character on line of received text
                    // starting new line with next character read
                    currentLineIsBlank = true;
                } 
                else if (c != '\r') {
                    // a text character was received from client
                    currentLineIsBlank = false;
                }
            } // end if (client.available())
        } // end while (client.connected())
        delay(1);      // give the web browser time to receive the data
        client.stop(); // close the connection
    } // end if (client)
}

// switch LED and send back HTML for LED checkbox
void ProcessCheckbox(EthernetClient cl)
{
    if (HTTP_req.indexOf("LED2=2") > -1) {  // see if checkbox was clicked
        // the checkbox was clicked, toggle the LED
        LED_status=1;
    }
      // see if checkbox was clicked
        // the checkbox was clicked, toggle the LED
        else
        LED_status=0;
    
    if (LED_status) {    // switch LED on
        digitalWrite(2, HIGH);
        // checkbox is checked
        cl.println("<input type=\"checkbox\" name=\"LED2\" value=\"2\" \onclick=\"submit();\" checked>LED2");
        cl.println("on<Br>");
    }
    else {              // switch LED off
        digitalWrite(2, LOW);
        // checkbox is unchecked
        cl.println("<input type=\"checkbox\" name=\"LED2\" value=\"2\" \onclick=\"submit();\">LED2");
        cl.println("off<Br>");
    }
    if (HTTP_req.indexOf("LED1=2") > -1) {  // see if checkbox was clicked
        // the checkbox was clicked, toggle the LED
        LED_status1=1;
    }
      // see if checkbox was clicked
        // the checkbox was clicked, toggle the LED
        else
        LED_status1=0;
    
    if (LED_status1) {    // switch LED on
        digitalWrite(3, HIGH);
        // checkbox is checked
        cl.println("<input type=\"checkbox\" name=\"LED1\" value=\"2\" \onclick=\"submit();\" checked>LED1");
        cl.println("on\n");
    }
    else {              // switch LED off
        digitalWrite(3, LOW);
        // checkbox is unchecked
        cl.println("<input type=\"checkbox\" name=\"LED1\" value=\"2\" \onclick=\"submit();\">LED1");
        cl.println("off\n");
    }
}



