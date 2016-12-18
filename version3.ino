#include "I2Cdev.h"
#include "MPU6050.h"
#include "Wire.h"
#include <ESP8266WiFi.h>
#include "Pushover.h"
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include "FS.h"
MPU6050 accelgyro;
int state = 0 ;
int16_t ax, ay, az, gx, gy, gz;
float acx, acy, acz;
float cx = 0, cy = 0 , cz = 0;//calibration
unsigned long timer, preTime , timeOut;
int tmp[2];
int m;
float Raw_AM;
int i;
int check = 1;
const int buttonPin = 16;     // D0
const int ledPin =  12;      // D7
const int vibration = 2 ;
const int ledwifi = 13 ;
int buttonState = 0;
int program_mode= 0;
//
//const char* ssid     = "chatsada";
//const char* password = "159357123";

/*
   NodeMCU/ESP8266 act as AP (Access Point) and simplest Web Server
   Connect to AP "arduino-er", password = "password"
   Open browser, visit 192.168.4.1
*/
int start_ap = 1 ;
String ssid_list[4];
String password_list[4];
String form =                                             // String form to sent to the client-browser
  //<form action='msg'><p>Type something nice... <input type='text' name='msg' size=50 autofocus> <input type='submit' value='Submit'></form>
  "<head>"
  "<style>"
  "table {"
  " font-family: arial, sans-serif;"
  "border-collapse: collapse;"
  "width: 50%;"
  "}"

  "td, th {"
  "border: 1px solid #dddddd;"
  " text-align: left;"
  " padding: 8px;"
  "}"

  "tr:nth-child(even) {"
  " background-color: #dddddd;"
  "}"
  "</style>"
  "</head>"
  "<p>"
  "<center>"
  " <h1>Talk to me! </h1>"
  "<img src='http://yt3.ggpht.com/-vOfCsox2nxk/AAAAAAAAAAI/AAAAAAAAAAA/1JXT_EWqap4/s100-c-k-no/photo.jpg'>"
  "<table>"
  "<tr>"
  " <th>SSID</th>"
  "<th>PASSWORD</th>"
  "</tr>"
  ;
long period;
const char *ssid = "smart helper";
const char *password = "password";
/*
  handles the messages coming from the webbrowser, restores a few special characters and
  constructs the strings that can be sent to the oled display
*/String Edit_form =  "<fieldset >"
                      " <legend>SSID information:</legend><form action='msg'>"
                      "<table style='width:50%'>"
                      "<tr>"
                      " <th>SSID</th>"
                      "<th>PASSWORD</th>"
                      "</tr>";


ESP8266WebServer server(80);


/*char* htmlBody_help = "<h1>Help</h1><br/>\n"
  "Visit http://192.168.4.1/ to access web server.<br/>\n"
  "Visit http://192.168.4.1/help to access this page.<br/>\n";

  void handleHelp(){
  server.send(200, "text/html", htmlBody_help);
  }*/

void web_page() {
  String line;
  int round_read = 0;
  File file = SPIFFS.open("/test.txt", "r");
  if (!file) {
    Serial.println("file open failed!");
  } else {
    Serial.println("file open success: web server)");
    while (file.available()) {
      //Lets read line by line from the file
      String line1 = file.readStringUntil('\n');
      if (line1.startsWith("ssid = ")) {
        line += "<tr>";
        line += " <td>";
        line += line1.substring(7);
        // line += " <td>Maria Anders</td>";
        //ssid[s] = line.substring(7);
        //  Serial.println(ssid[sizeof(ssid)]);
        //Serial.println(ssid[s]);
        // s++;
        line += " </td>";
      } else if (line1.startsWith("password = ")) {
        // password[p] = line.substring(11);
        //Serial.println(password[p]);
        // p++;
        line += " <td>";
        line += line1.substring(11);
        line += " </td>";
        line += " </tr>";
      }
      round_read++;
      Serial.println(round_read);
    }

    file.close();
  }
  if (round_read < 8) {
    for (int i = round_read / 2; i < 4; i++) {
      Serial.println("loooopppppppppppppp");
      line += " </tr>";
      line += " <td>";
      line += " </td>";
      line += " <td>";
      line += " </td>";
      line += " </tr>";
    }

  }
  line += "</table>";
  line += " </center>";
  line += "<form action='edit'>";
  line += "<button name='edit' type='submit' value='edit'>EDIT</button>";
  server.send(200, "text/html", form + line);


}
void handle_msg()
{
  int s = 0;
  int p = 0;
  //server.send(200, "text/html", form);                    // Send same page so they can send another msg

  // Display msg on Oled
  Serial.println("from web");
  String ssid_msg ;
  String password_msg ;


  Serial.println("from web");
  File file2 = SPIFFS.open("/test.txt", "w");
  if (!file2) {
    Serial.println("file open failed!");
  } else {
    Serial.println("file open success:)");

    for (int i = 0; i < server.args(); i++) {
      if (server.argName(i) == "ssid") {
        ssid_msg = server.arg(i);
        ssid_msg.trim();
        file2.println("ssid = " + ssid_msg);
        //ssid[s] = ssid_msg;
        //s++;
      } else if (server.argName(i) == "password") {
        password_msg = server.arg(i);
        password_msg.trim();
        file2.println("password = " + password_msg);
        // password[p] = password_msg;
        // p++;
      }
    }
    delay(500);

    file2.close();
  }
  delay(2000);
  web_page() ;
}

void pepare_wifilist() {
  int s = 0;
  int p = 0;
  File file = SPIFFS.open("/test.txt", "r");
  if (!file) {
    Serial.println("file open failed!");
  } else {
    Serial.println("file open success:)");

    while (file.available()) {
      //Lets read line by line from the file
      String line = file.readStringUntil('\n');
      if (line.startsWith("ssid = ")) {
        ssid_list[s] = line.substring(7);
        //  Serial.println(ssid_list[sizeof(ssid_list)]);

        Serial.println(ssid_list[s]);
        s++;
      } else if (line.startsWith("password = ")) {
        password_list[p] = line.substring(11);
        Serial.println(password_list[p]);
        p++;
      }

    }
    if (s == 3) {
      s = 0;
      p = 0;
    }
    for (int j = 0 ; j < 4 ; j++) {
      Serial.println(j);
      Serial.println(ssid_list[j]);
      Serial.println(password_list[j]);

    }
  } file.close();
}

String current_ssid     = "your-ssid";
String  current_password  = "your-password";
void setup_wifi() {
  int s = 0;
  int p = 0;
  delay(1000);
  server.stop();
  WiFi.softAPdisconnect(true);
  Serial.println("Starting in STA mode");
  WiFi.mode(WIFI_STA);
  /*WiFi.begin("chatsada", "159357123");
    while (WiFi.status() != WL_CONNECTED) {                 // Wait for connection
    delay(500);
    Serial.print(".");
    //  SendChar('.');
    }*/
  //     WiFi.config(charToIPAddress(ip), charToIPAddress(gateway), mask);
  Serial.print( WiFi.localIP());
  File file = SPIFFS.open("/test.txt", "r");
  if (!file) {
    Serial.println("file open failed!");
  } else {
    Serial.println("file open success:)");

    while (file.available()) {
      //Lets read line by line from the file
      String line = file.readStringUntil('\n');
      if (line.startsWith("ssid = ")) {
        ssid_list[s] = line.substring(7);
        current_ssid = ssid_list[s];
        //  Serial.println(ssid[sizeof(ssid)]);
        char ssid[current_ssid.length()];
        current_ssid.toCharArray(ssid, current_ssid.length());
        Serial.println( current_ssid.length());
        Serial.println(current_ssid);
        s++;
      } else if (line.startsWith("password = ")) {
        password_list[p] = line.substring(11);
        current_password = password_list[p];
        char password1[current_password.length()];
        current_password.toCharArray(password1, current_password.length());
        Serial.println( current_password.length());
        Serial.println(current_password);

        p++;
      }

    }
    if (s == 3) {
      s = 0;
      p = 0;
    }
  }
  file.close();
  for (int j = 0 ; j < 4 ; j++) {
    if (WiFi.status() != WL_CONNECTED) {
      current_ssid = ssid_list[j];
      //  Serial.println(ssid[sizeof(ssid)]);
      char ssid1[current_ssid.length()];
      current_ssid.toCharArray(ssid1, current_ssid.length());
      Serial.println( current_ssid.length());
      Serial.println(current_ssid);
      current_password = password_list[j];
      char password1[current_password.length()];
      current_password.toCharArray(password1, current_password.length());
      Serial.println( current_password.length());
      Serial.println(current_password);

      WiFi.begin(ssid1, password1);
      for (int i = 0 ; i < 20; i ++) {
        if (WiFi.status() != WL_CONNECTED) {
          delay(500);
          Serial.print(".");
        }
      }
    } else {
      j = 5;
    }
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }

}

void edit_default()
{
  String line_edit;
  int round_read = 0;
  File file = SPIFFS.open("/test.txt", "r");
  if (!file) {
    Serial.println("file open failed!");
  } else {
    Serial.println("file open success: web server)");

    while (file.available()) {
      //Lets read line by line from the file
      String line2 = file.readStringUntil('\n');
      if (line2.startsWith("ssid = ")) {
        line_edit += "<tr>";
        line_edit += " <td><input type='text' name='ssid' value='";
        line_edit += line2.substring(7);
        line_edit += "' autofocus></td>";


        // line += " <td>Maria Anders</td>";
        //ssid[s] = line.substring(7);
        //  Serial.println(ssid[sizeof(ssid)]);
        //Serial.println(ssid[s]);
        // s++;
        // line_edit += " </td>";
      } else if (line2.startsWith("password = ")) {
        // password[p] = line.substring(11);
        //Serial.println(password[p]);
        // p++;
        line_edit += " <td><input type='text' name='password' value='";
        line_edit += line2.substring(11);
        line_edit += "' autofocus>";
        line_edit += " </td>";
        line_edit += " </tr>";
        line_edit += " </tr>";


      }

      round_read++;
    }

  }
  if (round_read < 8) {
    for (int i = round_read / 2; i < 4; i++) {
      line_edit += "<tr>";
      line_edit += " <td><input type='text' name='ssid' value='";

      line_edit += "' autofocus></td>";
      line_edit += " <td><input type='text' name='password' value='";

      line_edit += "' autofocus>";
      line_edit += " </td>";
      line_edit += " </tr>";
      line_edit += " </tr>";
    }
  }
  line_edit += "</table>";
  line_edit += "<input type='submit' value='Submit'></form>";
  line_edit += "</fieldset>";
  server.send(200, "text/html", Edit_form + line_edit);
  file.close();
}


void prepareFile() {

  Serial.println("Prepare file system");
  SPIFFS.begin();

  File file = SPIFFS.open("/test.txt", "r");
  if (!file) {
    Serial.println("file open failed!");
    // open file for writing
    File f = SPIFFS.open("/test.txt", "w");
    if (!f) {
      Serial.println("file open failed");
    }
    Serial.println("====== Writing to SPIFFS file =========");
    f.close();
  } else {
    Serial.println("file open success:)");

    while (file.available()) {
      Serial.write(file.read());
    }

    file.close();
  }

}
void setup_apmode() {
  WiFi.disconnect();
  delay(1000);
  Serial.println("Starting in AP mode");
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  delay(1000);
  IPAddress apip = WiFi.softAPIP();
  Serial.print("visit: \n");
  Serial.println(apip);

  // Set up the endpoints for HTTP server,  Endpoints can be written as inline functions:
  server.on("/", []()
  { web_page() ;
  } );

  server.on("/msg", handle_msg);                          // And as regular external functions:
  server.on("/edit", edit_default);
  server.begin();                                         // Start the server

  /*
    // Just for fun... 'cause we can
    sprintf(result, "Analog A0 = %4d",analogRead(A0));
    Serial.println(result);
    sendStrXY(result, 6, 0);
  */
  // prepareFile();
}


void send_notify() {
  Pushover po = Pushover("atxx2at12gtx95rs4d54qn5unnw8xi", "uja9ap1p4ndz2et5b62114d93trhc2"); //a5730069b  "aniht4f1tap1bjfm3wcn1omu342kmq","urd352cyho726n3d9kvs5qdbgknk6r" //api key //user key
  //po.setDevice("chrome");
  po.setMessage("WARNING!!!! FALL DETECTION");
  po.setSound("siren");
  po.setPriority(2);
  po.setRetry(30);
  po.setExpire(3600);
  Serial.println(po.send()); //should return 1 on success
}
void setup() {
#ifdef ESP8266
  Wire.begin(5, 4);
#endif
  Serial.begin(9600);
  accelgyro.initialize();
  accelgyro.setFullScaleAccelRange(MPU6050_ACCEL_FS_16);
  // initialize the LED pin as an output:
  pinMode(ledPin, OUTPUT);
  // initialize the pushbutton pin as an input:
  pinMode(buttonPin, INPUT);
  //accelgyro.setXAccelOffset(0);
  pinMode(vibration, OUTPUT);
  pinMode(ledwifi, OUTPUT);
  prepareFile();
  timeOut = millis();
}

void loop() {
  //  if(WiFi.status() != WL_CONNECTED){
  //    WiFi.begin(ssid, password);
  //    digitalWrite(ledwifi,LOW);
  //   while (WiFi.status() != WL_CONNECTED) {
  //
  //    delay(500);
  //    Serial.print(".");
  //  }
  //  digitalWrite(ledwifi,HIGH);
  //  }
  timer = millis();

  buttonState = digitalRead(buttonPin);
  if (program_mode == 0 ) {
    if (((timer - timeOut) / 1000) < 10) {
      if (buttonState == HIGH) {
        unsigned long timerAck = ((timer - preTime) / 1000);
        Serial.println(timerAck);
        if ( timerAck >= 4) {
          Serial.println("I got some problem at state 0");
          delay(50);

          //   digitalWrite(led, LOW);
         program_mode = 1; //

        }
      } else {
        preTime = timer;
        digitalWrite(ledPin, LOW);
      }
    } else {
      program_mode = 2;
    }
  }
  //buttonState = digitalRead(buttonPin);

  // check if the pushbutton is pressed.
  // if it is, the buttonState is HIGH:
  if (program_mode == 1) {
    // turn LED on:

    if (start_ap == 1) {
      setup_apmode();

      start_ap = 0;
    }
    server.handleClient();
    digitalWrite(ledPin, LOW);

  }
  if (program_mode == 2) {
    if (start_ap == 1) {
      setup_wifi();
      //  digitalWrite(ledPin, LOW);
      //   ESP.restart();
      start_ap = 0;
      digitalWrite(ledPin, HIGH);
    }
    if (WiFi.status() != WL_CONNECTED) {
      setup_wifi();
    }
    //   digitalWrite(ledPin, LOW);
    //   server.handleClient();


    if (state == 0) {

     // buttonState = digitalRead(buttonPin);
      accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
      acx = (ax + cx); //(16384 MPU6050_ACCEL_FS_2)
      acy = (ay + cy); //(2048 MPU6050_ACCEL_FS_16)
      acz = (az + cz);
      tmp[0] = Raw_AM;
      Raw_AM = pow(pow(acx, 2) + pow(acy, 2) + pow(acz, 2), 0.5);
      Serial.println(Raw_AM);
      if (timer < 2000) {
        tmp[1] = Raw_AM - tmp[0];
      } else {
        m = Raw_AM - tmp[0];
      }
      //  Serial.println(m);

      if (m > 10000 || m < -10000 && check == 0) {

        Serial.println("FALL DETECTED");
        check = 1;
        state = 1;
        timeOut = timer;
      } else {
        //Serial.println("no problem");
        check = 0;
        state = 0;

      }

      if (buttonState == HIGH) {
        unsigned long timerAck = ((timer - preTime) / 1000);
        if ( timerAck >= 0.0) {
          Serial.println("I got some problem at state 0");
          delay(50);
          digitalWrite(ledPin, HIGH);
          state = 3; //

        }
      } else {
        preTime = timer;
      }


    } else if (state == 1) {
      Serial.println((timer - timeOut) / 1000);
      buttonState = digitalRead(buttonPin);

      if (((timer - timeOut) / 1000) < 4) {
        digitalWrite(ledPin, LOW);
        digitalWrite(vibration , LOW);
        delay(500);
        digitalWrite(ledPin, HIGH);
        digitalWrite(vibration , HIGH);
        delay(500);

        if (buttonState == HIGH) {
          unsigned long timerAck = ((timer - preTime) / 1000);
          if ( timerAck >= 1) {
            Serial.println("no problem ");
            digitalWrite(ledPin, LOW);
            digitalWrite(vibration , LOW);
            delay(50);
            state = 5;
          }
        } else {
          preTime = timer;
        }

      } else {
        Serial.println("notify()");
        send_notify();
        state = 2;
      }

    } else if (state == 2) {
      //buzzer , vibration on

      Serial.println("state2");
      digitalWrite(ledPin, HIGH);
      digitalWrite(vibration , HIGH);
      delay(50);
      buttonState = digitalRead(buttonPin);
      digitalWrite(ledPin, LOW);
      digitalWrite(vibration , LOW);
      delay(50);

      if (buttonState == HIGH ) {

        unsigned long timerAck = ((timer - preTime) / 1000);
        if ( timerAck >= 1) {
          state = 4;
        }

      } else {
        preTime = timer;
      }

    } else if (state == 3) {
      buttonState = digitalRead(buttonPin);
      if (buttonState == HIGH) {

      } else {
        send_notify();
        delay(50);
        state = 2;
      }
    } else if (state == 4) {
      buttonState = digitalRead(buttonPin);
      if (buttonState == HIGH) {

      } else {
        delay(50);
        state = 0;
      }
    } else if (state == 5) {
      buttonState = digitalRead(buttonPin);
      if (buttonState == HIGH) {

      } else {
        delay(50);
        state = 0;
      }
    }
  }
}

