#include <WiFi.h>

//Add own ssid and password
const char* ssid = "";
const char* password = "";

WiFiServer server(80);

String header;

String led = "off";

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);

  //Conectarse al wifi
  Serial.print("Connecting to...");
  Serial.print(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  //Ip del esp y empieza el servidor
  Serial.print("");
  Serial.print("WiFi connected.");
  Serial.print("IP address: ");
  Serial.print(WiFi.localIP());
  server.begin();

}

void loop() {
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            //Prender led del esp
            if (header.indexOf("GET /LED_BUILTIN/on") >= 0) {
              Serial.println("LED_BUILTIN on");
              led = "on";
              digitalWrite(LED_BUILTIN, HIGH);
            } else if (header.indexOf("GET /LED_BUILTIN/off") >= 0) {
              Serial.println("LED_BUILTIN off");
              led = "off";
              digitalWrite(LED_BUILTIN, LOW);
            }

            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");

            //CSS
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");

            client.println("<body><h1>ESP32 Web Server</h1>");

            // Display current state, and ON/OFF buttons for LED_BUILTIN  
            client.println("<p>LED_BUILTIN - State " + led + "</p>");
            // If the output26State is off, it displays the ON button       
            if (led=="off") {
              client.println("<p><a href=\"/LED_BUILTIN/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/LED_BUILTIN/off\"><button class=\"button button2\">OFF</button></a></p>");
            }

            client.println("</body></html>");

            } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }

    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
