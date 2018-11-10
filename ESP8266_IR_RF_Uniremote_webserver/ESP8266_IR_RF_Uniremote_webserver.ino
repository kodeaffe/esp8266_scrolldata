/* MURSY NOTE
  adapted from http://www.esp8266.com/viewtopic.php?f=11&t=9293#
  and https://github.com/oguime/Hue_W5100_HT6P20B/blob/master/Hue_W5100_HT6P20B.ino
  and James Bruce (http://www.makeuseof.com/tag/control-philips-hue-lights-arduino-and-motion-sensor/)
  and Gilson Oguime (https://github.com/oguime/Hue_W5100_HT6P20B/blob/master/Hue_W5100_HT6P20B.ino).
  and Rui Santos https://randomnerdtutorials.com/esp8266-web-server/
  http://esp8266-server.de/
*/

#include <IRutils.h> //for serialPrintUint64
#include <IRrecv.h> //to recieve IR commands
#include <IRsend.h> //to resend IR commands
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <RCSwitch.h> //RF library

String scriptname = "ESP8266_IR_RF_Uniremote_2-webserver";
//  Serial.println(scriptname); // insert under setup

//Timer
int count = 0;

//  Hue constants
const char hueHubIP[] = "192.168.1.239";  //Hue bridge IP
const char hueUsername[] = "user";  //ESP8266 username
const int hueHubPort = 80;

//  Hue variable
boolean hueOn;  // on/off
int hueBri;  // brightness value
String hueCmd;  //Hue command

//Set lightNum
//monkeys room: 1 /RF1
//my room: 2      /RF2
//kitchen: 3      /RF3
int lightNum;

//RF
RCSwitch mySwitch = RCSwitch();

////RF1 lightNum 1
#define RF1A 8710306 //toggle
#define RF1B 8710305 //brighter
#define RF1C 8710312 //dimmer
////RF2 lightNum 2
#define RF2A 3064690 //toggle
#define RF2B 3064696 //brighter
#define RF2C 3064692 //dimmer
////RF3 lightNum 3
#define RF3A  //toggle
#define RF3B  //brighter
#define RF3C  //dimmer

// IR
IRrecv irrecv (14); // D5
IRsend irsend(13);  // D7
decode_results results;

////IR remote keys - Amp remote
#define IRKEY1 0x10EA05F //skip back
#define IRKEY2 0x10E20DF //skip forward
#define IRKEY3 0x10ED02F //A-B //lights toggle
#define IRKEY4 0x10EB04F //intro //projecter power
#define IRKEY5 0x10E0AF5 //disc

//  WiFi
WiFiClient client;
IPAddress ip(192, 168, 1, 231);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
const char* ssid     = "ssid";
const char* password = "password";

//   WEBSERVER
ESP8266WebServer server(80); // Set web server port number to 80
String Webdata;  // Just as example

//==============================================================
//                  SETUP
//==============================================================
void setup() {
  //serial ouput
  Serial.begin(115200);
  Serial.println("");
  Serial.println(scriptname);

  // connect WiFi
  Serial.print("Connecting");
  WiFi.mode(WIFI_STA); //Client mode only, no Access Point
  WiFi.config(ip, gateway, subnet);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Serial output when connected
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("Current IP: ");
  Serial.println(WiFi.localIP());
  Serial.println("Ready.");
  Serial.println("");

  //start IR receiver
  irrecv.enableIRIn();
  irsend.begin();

  //start RF receiver
  mySwitch.enableReceive(4);  // Receiver pin #D2 on ESP8266

  //start webserver
  server.on("/", view_index);
  server.on("/data/1", view_set_data_1);
  server.on("/data/2", view_set_data_2);
  server.on("/data/otto", view_set_data_otto);
  server.on("/data", view_get_data);
  server.on("/lights", view_get_lights);
  server.on("/light/1/toggle", view_toggle_light_1);
  server.on("/light/1/brightness", view_set_brightness_light_1);
  server.begin();
}


//==============================================================
//                     LOOP
//==============================================================
void loop() {
  if (count == 17) {
    Serial.println("Reset...");
    ESP.restart();
  }

  //IR section////////////////////////////////////////////////////////////////////////////////
  if (irrecv.decode(&results))  {

    switch (results.value) {
      case IRKEY1:
        Serial.println("IRKEY1 - lights down");
        lightNum = 2;
        lightsdown (lightNum);
        break;
    }
    switch (results.value) {
      case IRKEY2:
        Serial.println("IRKEY2 - lights up");
        lightNum = 2;
        lightsup (lightNum);
        break;
    }
    switch (results.value) {
      case IRKEY3: //Toggle lights
        Serial.println("IRKEY3- Toggle lights");
        lightNum = 2;
        lighttoggle (lightNum);
        break;
    }
    switch (results.value) {
      case IRKEY4:
        Serial.print("IRKEY8 - ");
        Serial.println("Projecter power");
        irsend.sendNEC(0x40BFB847, 32);
        break;
    }
    switch (results.value) {
      case IRKEY5:
        Serial.print("IRKEY5 - ");
        Serial.println("Reset..");
        ESP.restart();
        break;
    }
    Serial.println();
    irrecv.resume(); //receive next value
    delay(100);
  }

  //RF Section//////////////////////////////////////////////////////////////////////////////////////////
  if (mySwitch.available()) {

    int value = mySwitch.getReceivedValue();

    //RF1 Monkeys room/////////////////////////////////////////////////////////////////////////////////
    switch ( mySwitch.getReceivedValue() ) {
      case RF1A:
        Serial.println("RF1A - Toggle lights");
        lightNum = 1;
        lighttoggle (lightNum);
        break;
    }
    switch ( mySwitch.getReceivedValue() ) {
      case RF1B:
        Serial.println("RF1B - lights up");
        lightNum = 1;
        lightsup (lightNum);
        break;
    }
    switch ( mySwitch.getReceivedValue() ) {
      case RF1C:
        Serial.println("RF1C - lights down");
        lightNum = 1;
        lightsdown (lightNum);
        break;
    }

    //RF2 My Room////////////////////////////////////////////////////////////////////
    switch ( mySwitch.getReceivedValue() ) {
      case RF2A:
        Serial.println("RF2A - Toggle lights");
        lightNum = 2;
        lighttoggle (lightNum);
        break;
    }
    switch ( mySwitch.getReceivedValue() ) {
      case RF2B:
        Serial.println("RF2B - lights up");
        lightNum = 2;
        lightsup (lightNum);
        break;
    }
    switch ( mySwitch.getReceivedValue() ) {
      case RF2C:
        Serial.println("RF2C - lights down");
        lightNum = 2;
        lightsdown (lightNum);
        break;
    }

    //    Serial.print("Received RF Code: ");
    //    Serial.print( mySwitch.getReceivedValue() );
    //    Serial.print(" / ");
    //    Serial.print( mySwitch.getReceivedBitlength() );
    //    Serial.print("bit ");
    //    Serial.print("Protocol: ");
    //    Serial.println( mySwitch.getReceivedProtocol() );
    Serial.println();
    mySwitch.resetAvailable();
    delay (200);
  }
  
  //WEBSRVER section///////////////////////////////////////////////////////////////////////////////
  server.handleClient();
}

// HUE Functions////////////////////////////////////////////
boolean setHue(int lightNum, String command)
{
  if (client.connect(hueHubIP, hueHubPort))
  {
    client.print("PUT /api/");
    client.print(hueUsername);
    client.print("/lights/");
    client.print(lightNum);  // hueLight zero based, add 1
    client.println("/state HTTP/1.1");
    client.println("keep-alive");
    client.print("Host: ");
    client.println(hueHubIP);
    client.print("Content-Length: ");
    client.println(command.length());
    client.println("Content-Type: text/plain;charset=UTF-8");
    client.println();  // blank line before body
    client.println(command);  // Hue command
    delay (100);
    client.stop();
    return true;  //command executed
  }
  else
    return false;  //command failed
}

/* A helper function in case your logic depends on the current state of the light.
   This sets a number of global variables which you can check to find out if a light is currently on or not
   and the hue etc. Not needed just to send out commands
*/
boolean getHue(int lightNum)
{
  if (client.connect(hueHubIP, hueHubPort))
  {
    //    Serial.println("getting status...");
    client.print("GET /api/");
    client.print(hueUsername);
    client.print("/lights/");
    client.print(lightNum);
    client.println(" HTTP/1.1");
    client.print("Host: ");
    client.println(hueHubIP);
    client.println("Content-type: application/json");
    client.println("keep-alive");
    client.println();
    while (client.connected())
    {
      if (client.available())
      {
        client.findUntil("\"on\":", "\0");
        hueOn = (client.readStringUntil(',') == "true");  // if light is on, set variable to true
        client.findUntil("\"bri\":", "\0");
        hueBri = client.readStringUntil(',').toInt();  // set variable to brightness value
        //        client.findUntil("\"hue\":", "\0");
        //        hueHue = client.readStringUntil(',').toInt();  // set variable to hue value

        break;
      }
    }
    client.stop();
    return true;  // captured on,bri
  }
  else
    return false;  // error reading on,bri
}

//Light Functions///////////////////////////////////////////////////////

boolean lightsdown(int lightNum) {
  Serial.print("Hue lightNum:");
  Serial.println(lightNum);
  getHue (lightNum);
  displayHue (lightNum, "Old");
  if (hueOn == HIGH) {
    if (hueBri <= 10) {
      Serial.println("Toggling: off");
      hueBri = 1; // set brightness to 1
      String command = "{\"on\": false,\"bri\":1}";
      setHue(lightNum, command);
    }
    if (hueBri >= 11 && hueBri <= 70) {
      Serial.println("Toggling: Dim");
      hueBri = 1;
      String command = "{\"bri\": " + String(hueBri) + "}";
      setHue(lightNum, command);
    }
    if (hueBri >= 71 && hueBri <= 100) {
      Serial.println("Toggling: Dimmer");
      hueBri = 70;
      String command = "{\"bri\": " + String(hueBri) + "}";
      setHue(lightNum, command);
    }
    if (hueBri >= 101 && hueBri <= 170) {
      Serial.println("Toggling: Dimmer");
      hueBri = 100;
      String command = "{\"bri\": " + String(hueBri) + "}";
      setHue(lightNum, command);
    }
    if (hueBri >= 171) {
      Serial.println("Toggling: Dimmer");
      hueBri = 170;
      String command = "{\"bri\": " + String(hueBri) + "}";
      setHue(lightNum, command);
    }
  } else {
    Serial.println("Already Off");
  }
  getHue (lightNum);
  displayHue (lightNum, "New");
  count++;
  Serial.print("Count: ");
  Serial.println(count);
  delay (200);
}

boolean lightsup(int lightNum) {
  Serial.print("Hue lightNum:");
  Serial.println(lightNum);
  getHue (lightNum);
  displayHue (lightNum, "Old");
  if (hueOn == LOW) {
    Serial.println("Toggling: On");
    hueBri = 1; // set brightness to 1
    String command = "{\"on\": true,\"bri\": " + String(hueBri) + "}";
    setHue(lightNum, command);
  }
  if (hueOn == HIGH)  {
    if (hueBri >= 254) {
      Serial.println("At Max Brightness");
    }
    if (hueBri >= 170 && hueBri <= 253) {
      Serial.println("Toggling: Full");
      hueBri = 254;
      String command = "{\"bri\": " + String(hueBri) + "}";
      setHue(lightNum, command);
    }
    if (hueBri >= 100 && hueBri <= 169) {
      Serial.println("Toggling: Brighter");
      hueBri = 170;
      String command = "{\"bri\": " + String(hueBri) + "}";
      setHue(lightNum, command);
    }
    if (hueBri >= 70 && hueBri <= 99) {
      Serial.println("Toggling: Brighter");
      hueBri = 100;
      String command = "{\"bri\": " + String(hueBri) + "}";
      setHue(lightNum, command);
    }
    if (hueBri <= 10) {
      Serial.println("Toggling: Brighter");
      hueBri = 70;
      String command = "{\"bri\": " + String(hueBri) + "}";
      setHue(lightNum, command);
    }
  }
  getHue (lightNum);
  displayHue (lightNum, "New");
  count++;
  Serial.print("Count: ");
  Serial.println(count);
  delay (200);
}

boolean lighttoggle(int lightNum) {
  Serial.print("Hue lightNum:");
  Serial.println(lightNum);
  getHue (lightNum);
  displayHue (lightNum, "Old");
  if (hueOn == LOW) {
    Serial.println("Toggling: On");
    hueBri = 254;
    String command = "{\"on\": true,\"bri\": " + String(hueBri) + "}";
    setHue(lightNum, command);
  }
  if (hueOn == HIGH) {
    Serial.println("Toggling: Off");
    String command = "{\"on\": false}";
    setHue(lightNum, command);
  }
  getHue (lightNum);
  displayHue (lightNum, "New");
  count++;
  Serial.print("Count: ");
  Serial.println(count);
  delay (200);
}

boolean displayHue (int lightNum, String zeit) {
  Serial.print(zeit);
  Serial.print(" light status: ");
  Serial.println(hueOn);
  Serial.print(zeit);
  Serial.print(" HueBri:");
  Serial.println(hueBri);
}


//==============================================================
//                     WEB VIEWS
//==============================================================

void view_index()
{
  String html = "<!DOCTYPE html>"
"<html>"
"<head>"
  "<title>ESP8266 IR RF Uniremote</title>"
  "<script src=\"https://cdnjs.cloudflare.com/ajax/libs/jquery/3.3.1/jquery.min.js\"></script>"
  "<script src=\"https://cdnjs.cloudflare.com/ajax/libs/semantic-ui/2.4.1/semantic.min.js\"></script>"
  "<link rel=\stylesheet\" href=\"https://cdnjs.cloudflare.com/ajax/libs/semantic-ui/2.4.1/semantic.min.css\">"
  "<script type=\"text/javascript\">"
  "var TIMEOUT = 1000;"

  "function pullData() {"
    "$.get('/data', function(data, status) {"
      "var item = $('<div class=\"item\">' + data['datetime'] + ': ' + data['state'] + '</div>');"
      "var data = $('.data');"
      "data.prepend(item);"
      "window.setTimeout(pullData, TIMEOUT);"
    "});"
  "};"

  "function setState(light, state) {"
    "var icon = '<i class=\"lightbulb outline icon\"></i>';"
    "if (state) {"
      "icon = '<i class=\"lightbulb icon\"></i>';"
    "}"
    "light.find('.state').html(icon);"
  "};"

  "function setBrightness(light, value) {"
    "light.find('.brightness').val(value);"
  "};"

  "function pullLights() {"
    "$.get('/lights', function(data, status) {"
      "$.each(data, function(light_id, val) {"
        "var light = $('.card .light-' + light_id);"
        "setState(light, val['state']);"
        "setBrightness(light, val['brightness']);"
      "});"
      "window.setTimeout(pullLights, TIMEOUT);"
    "});"
  "};"

  "$(function(){"
    "window.setTimeout(pullData, TIMEOUT);"
    "window.setTimeout(pullLights, TIMEOUT);"
    "$('.menu .button').click(function(){"
      "var state = $(this).data('state');"
      "$.post('/control/' + state);"
      "return false;"
    "});"
    "$('.card .toggle').click(function(){"
      "var light_id = $(this).data('light_id');"
      "var url = '/light/' + light_id + '/toggle';"
      "$.post(url, function(data) {"
        "var light = $('.card .light-' + light_id);"
        "setState(light, data['state']);"
      "});"
      "return false;"
    "});"
    "$('.card .brightness').change(function(){"
      "var light_id = $(this).data('light_id');"
      "var brightness = $(this).val()"
      "var url = '/light/' + light_id + '/brightness/' + brightness;"
      "$.post(url, function(data) {"
        "var light = $('.card .light-' + light_id);"
        "setBrightness(light, data['brightness']);"
      "});"
      "return true;"
    "});"
  "});"
  "</script>"
"</head>"
"<body>"
"<div class=\"ui container\">"
  "<h1>ESP8266 Scrolldata</h1>"

  "<div class=\"ui cards\">"
    "<div class=\"card\">"
      "<div class=\"content light-1\">"
        "<div class=\"header\">Light 1</div>"
        "<div class=\"description\">"
          "<div class=\"ui relaxed divided list\">"
            "<div class=\"item\">"
              "State: <span class=\"state\"><i class=\"lightbulb outline icon\"></i></span>"
              "<button class=\"ui tiny button toggle\" data-light_id=\"1\">Toggle</button>"
            "</div>"
            "<div class=\"item\">"
              "HueBri: <input type=\"range\" min=\"0\" max=\"255\" value=\"0\" class=\"brightness\" data-light_id=\"1\" />"
            "</div>"
          "</div>"
        "</div>"
      "</div>"
    "</div>"
    "<div class=\"card\">"
      "<div class=\"content light-2\">"
        "<div class=\"header\">Light 2</div>"
        "<div class=\"description\">"
          "<div class=\"ui relaxed divided list\">"
            "<div class=\"item\">"
              "State: <span class=\"state\"><i class=\"lightbulb outline icon\"></i></span>"
              "<button class=\"ui tiny button toggle\" data-light_id=\"2\">Toggle</button>"
            "</div>"
            "<div class=\"item\">"
              "HueBri: <input type=\"range\" min=\"0\" max=\"255\" value=\"0\" class=\"brightness\" data-light_id=\"2\" />"
            "</div>"
          "</div>"
        "</div>"
      "</div>"
    "</div>"
  "</div>"

  "<div class=\"ui stackable menu\">"
    "<a class=\"item button\" data-state=\"1\">Set 1</a>"
    "<a class=\"item button\" data-state=\"2\">Set 2</a>"
    "<a class=\"item button\" data-state=\"otto\">Set otto</a>"
  "</div>"
  "<div class=\"ui segment\">"
    "<h2>Data</h2>"
    "<div class=\"ui list data\"></div>"
  "</div>"
"</div>"
"</body>"
"</html>";
  server.send(200, "text/html", html);
}


void view_set_data_1() {
  Webdata = "1";
  String json = "{\"state\": \"" + Webdata + "\"}";
  server.send(200, "text/json", json);
}


void view_set_data_2() {
  Webdata = "2";
  String json = "{\"state\": \"" + Webdata + "\"}";
  server.send(200, "text/json", json);
}


void view_set_data_otto() {
  Webdata = "otto";
  String json = "{\"state\": \"" + Webdata + "\"}";
  server.send(200, "text/json", json);
}


void view_get_data() {
  String json = "{\"datetime\": \"constant datetime\", \"state\": \"" + Webdata + "\"}";
  server.send(200, "text/json", json);
}


void view_get_lights() {
  getHue(1);
  String light_1 = "\"1\": {\"state\": ";
  if (hueOn) light_1 += String("true"); else light_1 += String("false");
  light_1 += String(", \"brightness\": ") + String(hueBri) + String("}");
  getHue(2);
  String light_2 = "\"1\": {\"state\": ";
  if (hueOn) light_2 += String("true"); else light_2 += String("false");
  light_2 += String(", \"brightness\": ") + String(hueBri) + String("}");
  String json = String("{") + light_1 + String(", ") + light_2 + String("}");
  server.send(200, "text/json", json);
}


void view_toggle_light_1() {
  lighttoggle(1);
  getHue(1);
  String json = "{\"state\": ";
  if (hueOn) json += String("true"); else json += String("false");
  json += String(", \"brightness\": ") + String(hueBri) + String("}");
  server.send(200, "text/json", json);
}


void view_set_brightness_light_1() {
  // something with lightsup or lightsdown
  getHue(1);
  String json = "{\"brightness\": \"" + String(hueBri) + "\"}";
  server.send(200, "text/json", json);
}
