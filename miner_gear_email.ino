#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "ESP32_MailClient.h"
 
const char* ssid = "";        //SSID of Wifi network
const char* password = "";   // Password of wifi network
 
#define emailSenderAccount    "vigsthrone4iot@gmail.com"    // Sender email address
#define emailSenderPassword   "odbz pkgw afxp eeej"            // Sender email password
#define smtpServer            "smtp.gmail.com"
#define smtpServerPort        465
#define emailSubject          "ALERT! Smoke Detected"   // Email subject
 
 
String inputMessage = "";   //Reciepent email alert.
String enableEmailChecked = "checked";
String inputMessage2 = "true";
 
// Default Threshold Value
String inputMessage3 = "40.0";                    // Default gas_value
String lastgaslevel;
 
 
// HTML web page to handle 3 input fields (email_input, enable_email_input, threshold_input)
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>Email Notification with Gas Level</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  <h2>Gas Level Detection</h2> 
  <h3>%GASVALUE%</h3>
  <h2>ESP Email Alert</h2>
  <form action="/get">
    Email Address <input type="email" name="email_input" value="%EMAIL_INPUT%" required><br>
    Enable Email Notification <input type="checkbox" name="enable_email_input" value="true" %ENABLE_EMAIL%><br>
    Gas Level Threshold <input type="number" step="0.1" name="threshold_input" value="%THRESHOLD%" required><br>
    <input type="submit" value="Submit">
  </form>
</body></html>)rawliteral";
 
void notFound(AsyncWebServerRequest *request) 
{
  request->send(404, "text/plain", "Not found");
}
AsyncWebServer server(80);
 
String processor(const String& var)
{
  if(var == "GASVALUE")
  {
    return lastgaslevel;
  }
  else if(var == "EMAIL_INPUT")
  {
    return inputMessage;
  }
  else if(var == "ENABLE_EMAIL")
  {
    return enableEmailChecked;
  }
  else if(var == "THRESHOLD")
  {
    return inputMessage3;
  }
  return String();
}
 
 
// Flag variable to keep track if email notification was sent or not
bool emailSent = false;
const char* PARAM_INPUT_1 = "email_input";
const char* PARAM_INPUT_2 = "enable_email_input";
const char* PARAM_INPUT_3 = "threshold_input";
 
// Interval between sensor readings. 
unsigned long previousMillis = 0;     
const long interval = 5000;    
 
SMTPData smtpData;
 
 
void setup() 
{
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) 
  {
    Serial.println("WiFi Failed!");
    return;
  }
  Serial.println();
  Serial.print("ESP IP Address: http://");
  Serial.println(WiFi.localIP());
  
 
  // Send web page to client
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  // Receive an HTTP GET request at <ESP_IP>/get?email_input=<inputMessage>&enable_email_input=<inputMessage2>&threshold_input=<inputMessage3>
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    // GET email_input value on <ESP_IP>/get?email_input=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1)) {
      inputMessage = request->getParam(PARAM_INPUT_1)->value();
      // GET enable_email_input value on <ESP_IP>/get?enable_email_input=<inputMessage2>
      if (request->hasParam(PARAM_INPUT_2)) {
        inputMessage2 = request->getParam(PARAM_INPUT_2)->value();
        enableEmailChecked = "checked";
      }
      else 
      {
        inputMessage2 = "false";
        enableEmailChecked = "";
      }
      // GET threshold_input value on <ESP_IP>/get?threshold_input=<inputMessage3>
      if (request->hasParam(PARAM_INPUT_3)) {
        inputMessage3 = request->getParam(PARAM_INPUT_3)->value();
      }
    }
    else {
      inputMessage = "No message sent";
    }
    Serial.println(inputMessage);
    Serial.println(inputMessage2);
    Serial.println(inputMessage3);
    request->send(200, "text/html", "HTTP GET request sent to your ESP.<br><a href=\"/\">Return to Home Page</a>");
  });
  server.onNotFound(notFound);
  server.begin();
}
 
 
 
void loop() 
{
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
 
    float gas_analog_value = analogRead(35);
    float gas_value = ((gas_analog_value/1023)*100);
    Serial.print(gas_analog_value);
    Serial.print(", ");
    Serial.println(gas_value);
    
    lastgaslevel = String(gas_value);
    
    // Check if gas_value is above threshold and if it needs to send the Email alert
    if(gas_value > inputMessage3.toFloat() && inputMessage2 == "true" && !emailSent)
    {
      String emailMessage = String("Smoke detected in the mine. Current Gas Level: ") + String(gas_value);//changed- one closing bracket removed
      if(sendEmailNotification(emailMessage)) {
        Serial.println(emailMessage);
        emailSent = true;
      }
      else {
        Serial.println("Email failed to send");
      }    
    }
    // Check if gas_value is below threshold and if it needs to send the Email alert
    else if((gas_value < inputMessage3.toFloat()) && inputMessage2 == "true" && emailSent) 
    {
      String emailMessage = String("Gas Level below threshold. Current gas_value: ") + String(gas_value) + String(" C");
      if(sendEmailNotification(emailMessage)) 
      {
        Serial.println(emailMessage);
        emailSent = false;
      }
      else {
        Serial.println("Email failed to send");
      }
    }
  }
}
 
 
bool sendEmailNotification(String emailMessage)
{
  // Set the SMTP Server Email host, port, account and password
  smtpData.setLogin(smtpServer, smtpServerPort, emailSenderAccount, emailSenderPassword);
  
  smtpData.setSender("ESP32_Gas_Alert_Mail", emailSenderAccount);
  
  // Set Email priority or importance High, Normal, Low or 1 to 5 (1 is highest)
  smtpData.setPriority("High");
  
  // Set the subject
  smtpData.setSubject(emailSubject);
  
  // Set the message with HTML format
  smtpData.setMessage(emailMessage, true);
  
  // Add recipients
  smtpData.addRecipient(inputMessage);
  smtpData.setSendCallback(sendCallback);
  
  if (!MailClient.sendMail(smtpData)) 
{
    Serial.println("Error sending Email, " + MailClient.smtpErrorReason());
    return false;
  }
 
  smtpData.empty();
  return true;
}
 
 
void sendCallback(SendStatus msg) 
{
  // Print the current status
  Serial.println(msg.info());
  
  // Do something when complete
  if (msg.success()) 
{
    Serial.println("----------------");
  }
}
