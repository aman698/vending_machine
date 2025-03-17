#include <TFT_eSPI.h> // Hardware-specific libra
#include <Wire.h>
#include <qrcode.h>
#include <cofig.hpp>
#define LOGS // Uncomment this to get logs over serial port

#define relay 12 // LOCK connected to GPIO D3 of ESP8266
#define buzzer 27
#define ltSwitch 21
TFT_eSPI tft;
WiFiManager wm;

String machId = "NT-GGN-047-VD";
const String baseUrl = "https://kiosk-backend.nutritap.in";
// const String baseUrl = "http://nutritapserver:8080";

double itemPrice = 10;
bool configFound = true;
bool qrDisplayed = false;
bool creditFound = false;
bool productVended = false;
double paymentDone = 0.0;
String payload = "NULL";

void parseVendConfirmation(String txnId, bool productVended, String error, String rfidCard);

void getkiosk()
{
  DynamicJsonDocument posId(1024);
  String macId = WiFi.macAddress();
  bool errormsgdisplayed = false;
  Serial.println(macId); //
  posId["posDeviceId"] = macId;
  String postMacId;
  serializeJson(posId, postMacId);
  bool response = false;
  HTTPClient https;
  while (!response)
  {
    if ((WiFi.status() == WL_CONNECTED))
    {
      if (https.begin(baseUrl + "/api/posdevice/kiosk-serial"))
      {
        https.addHeader("Content-Type", "application/json");
        int httpCode = https.POST(postMacId);
        String httpcodeStr = String(httpCode);
        Serial.println(postMacId);
        Serial.print("posName post status: ");
        Serial.println(httpCode);
        if (httpCode == 200)
        {
          Serial.printf("In True [HTTPS] POST... code: %d\n", httpCode);

          DynamicJsonDocument kiosk(2048);
          String machIdResponse = https.getString();

          deserializeJson(kiosk, machIdResponse);
          Serial.print(machIdResponse);

          String serial = kiosk["dto"]["kioskSerial"];
          if (serial != "null")
          {
            machId = serial;
            Serial.print("machId:");
            Serial.println(serial);

            response = true;
          }
          else if (serial == "null" && !errormsgdisplayed)
          {
            tft.drawString("< Mapping Not FOUND >", 10, 125, 1);
            errormsgdisplayed = true;
          }
        }
        else
        {
          Serial.println("get kiosk  :- Unable To Connect");

          https.end();
        }
      }
      delay(1000);
    }
  }
}
void createNewOrder()
{
  vendDetail = emptyVendDetail;
  DynamicJsonDocument newOrder(1024);
  newOrder["kioskId"] = machId;
  bool selfCheckout = true;
  newOrder["isSelfCheckout"] = selfCheckout;
  String startTransactionRequest;
  serializeJson(newOrder, startTransactionRequest);
  bool response = false;
  bool errormsgdisplayed = false;
  while (!response)
  {
    if ((WiFi.status() == WL_CONNECTED))
    {
      // if WiFi is Connected
      HTTPClient https;
      https.setConnectTimeout(30000);
      https.setTimeout(30000);
      if (https.begin(baseUrl + "/api/kiosk/selfcheckout/start-transaction"))
      {
        //  //HTTP header has been send and Server response header has been handled
        https.addHeader("Content-Type", "application/json");
        int httpCode = https.POST(startTransactionRequest);
        String httpcodeStr = String(httpCode);
        Serial.println(startTransactionRequest);
        Serial.print("Start Transaction Status : ");
        Serial.println(httpCode);
        if (httpCode == 200)
        {
          String order_details = https.getString();
          DynamicJsonDocument Order(1024);
          deserializeJson(Order, order_details);
          response = true;
        }
        else if (!errormsgdisplayed && httpCode != 200)
        {
          updateLogBody("Order created failed", "Null");
          updateLogBody("httpcode", httpcodeStr);
          paymentDeviceLogs();
          errormsgdisplayed = true;
        }
        https.end();
      }
      if (!response)
      {
        delay(1000);
      }
    }
  }
}
void handleVending()
{
  bool ltswitchlimit = false;
  paymentDone = paymentDone - itemPrice;
  digitalWrite(relay, LOW);
  delay(2000);
  while (!ltswitchlimit)
  {
    if (digitalRead(ltSwitch) == LOW)
    {
      ltswitchlimit = true;
    }
  }
  digitalWrite(relay, HIGH);
}
void paymentDeviceLogs()
{
  String networkErrorDuration = "";
  startTimer();
  bool response = false;

  bool errormsgdisplayed = false;
  while (!response)
  {
    unsigned long errorTime = elaspedTime();
    networkErrorDuration = String(errorTime);
    findUpdateLogBody();
    if ((WiFi.status() == WL_CONNECTED))
    {
      // if WiFi is Connected
      HTTPClient https;
      https.setConnectTimeout(30000);
      https.setTimeout(30000);
      if (https.begin(baseUrl + "/api/payment-device-logs/add"))
      {

        updateLogBody("error duration", networkErrorDuration);
        https.addHeader("Content-Type", "application/json");
        Serial.println(logBody);
        DynamicJsonDocument logs(2048);
        logs["kioskSerial"] = machId;
        logs["log"] = logBody;
        String logToTheServer;
        serializeJson(logs, logToTheServer);
        // start connection and send HTTP header
        int httpCode = https.POST(logToTheServer);
        Serial.println(logToTheServer);
        Serial.print("log update Status : ");
        Serial.println(httpCode);
        if (httpCode == 200)
        {
          String log = https.getString();
          Serial.print(response);
          logBody = "\0";
          response = true;
        }
        https.end();
      }
      else
      {
        Serial.println("log update ===> Unable To Connect");
      }
      if (!response)
      {
        delay(1000);
      }
      if (networkErrorDuration == "600")
      {
        ESP.restart();
      }
    }
  }
}
void getMachinePlano()
{
  HTTPClient https;
  bool errorMsgDisplayed = false;
  https.setConnectTimeout(30000);
  https.setTimeout(30000);
  int httpCode;
  bool response = false;
  while (WiFi.status() == WL_CONNECTED && !response)
  {
    if (https.begin(baseUrl + "/api/filler/getKiosksProducts/" + machId))
    {
      https.addHeader("Authorization", "Bearer 20619214210436534");
      httpCode = https.GET();
      String httpcodeStr = String(httpCode);
      if (httpCode == HTTP_CODE_OK)
      {
        String plano = https.getString();
        Serial.println("payload at plannogam :");
        // Serial.print(payload);
        DynamicJsonDocument jsonDoc(20000);
        DeserializationError error = deserializeJson(jsonDoc, plano);
        if (error)
        {
          Serial.println("Failed to parse JSON");
          Serial.println(error.c_str());
        }
        else
        {
          double cost = jsonDoc[0]["productsDto"]["cost"];
          itemPrice = cost;
          Serial.print("Cost: ");
          Serial.println(itemPrice);
          response = true;
          configFound = true;
        }
        // free(jsonDoc);
      }
      else if (httpCode != HTTP_CODE_OK && !errorMsgDisplayed)
      {
        tft.drawString("Error setting-up:" + httpcodeStr, 10, 125, 1);
        errorMsgDisplayed = false;
      }
    }
    https.end();
  }
}

void pollHandler()
{
  int paymentcount = 0;
  if ((WiFi.status() == WL_CONNECTED))
  {

    HTTPClient https;
    https.setConnectTimeout(30000);
    https.setTimeout(30000);
    int httpCode;
    String httpsrequestdata = "{\"machineId\":\"" + machId + "\"}";
    if (https.begin(baseUrl + "/api/transaction/status-of-payment"))
    {
      // HTTP header has been send and Server response header has been handled

      Serial.print("[HTTPS] POST...\n");

      Serial.println(httpsrequestdata);
      https.addHeader("Content-Type", "application/json");
      // start connection and send HTTP header
      httpCode = https.POST(httpsrequestdata);

      // httpCode will be negative on error
      if (httpCode > 0)
      {

        Serial.printf("[HTTPS] POST... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK)
        {
          Serial.println("http is equal to 200");
          payload = https.getString();

          Serial.print("payload_1 : ");
          Serial.println(payload);
        }
      }
      else
      {

        // Serial.printf("[HTTPS] POST... failed, error: %s\n", https.errorToString(httpCode).c_str());
        Serial.printf("[HTTPS] POST... failed, error: %d\n", httpCode);
      }
      https.end();
    }
    else
    {

      Serial.printf("[HTTPS] Unable to connect\n");
    }
  }
  paymentcount++;
  if (payload != "NULL")
  {
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, payload);
    JsonObject obj = doc.as<JsonObject>();
    String credit = obj[String("amount")];
    String canVendProduct = obj[String("canVendProduct")];
    String txnId = obj[String("txnId")];

    if (!credit.equalsIgnoreCase("null") && canVendProduct == "true")
    {
      Serial.println("Credit Found...");
      paymentDone = credit.toInt();
      if (paymentDone < itemPrice)
      {
        parseVendConfirmation(txnId, false, "amount less than min balance", "\0");
        transactionHandle(getVendConfirmationPayload());
        qrDisplayed = false;
      }
      else
      {

        creditFound = true;
      }
    }
    else
    {
      Serial.println("Credit Not Found...");
    }
  }
  if (paymentcount > 10)
  {
    paymentDone = 10;
    creditFound = true;
  }

  delay(1000);
}

void show_QR(String data)
{
  String price = String(itemPrice);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_CYAN);
  tft.drawString("Price of each item :- " + price, 10, 10, 2);
  tft.setTextColor(TFT_RED);
  tft.setTextSize(2);
  tft.drawString("SCAN QR TO PAY", 18, 40, 2);
  const char *url;
  int n = 10;
  QRCode qrcode;
  uint8_t qrcodeData[qrcode_getBufferSize(50)];
  Serial.printf("Payment url in QR function : ");
  Serial.println(data);
  url = data.c_str();
  Serial.println(url);
  qrcode_initText(&qrcode, qrcodeData, 6, 3, url);

  int moduleSize = min((220 - n) / qrcode.size, (220 - n) / qrcode.size);
  for (uint8_t y = 0; y < qrcode.size; y++)
  {
    for (uint8_t x = 0; x < qrcode.size; x++)
    {
      if (qrcode_getModule(&qrcode, x, y))
      {
        tft.fillRect((x * moduleSize) + 20, (y * moduleSize) + 80, moduleSize, moduleSize, TFT_WHITE);
      }
    }
  }
  qrDisplayed = true;
}
void transactionHandle(String payload)
{
  int retryCount = 0;
  String balance = String(paymentDone);

  if (balance != 0)
  {
    updateLogBody("Refund amount", balance);
    updateLogBody("Vend confirmation payload:", payload);
  }
  Serial.print("Vend-Confirmation ===> Payload : ");
  Serial.println(payload);
  bool response = false;
  bool errormsgdisplayed = false;
  while (!response && retryCount <= 10)
  {
    if ((WiFi.status() == WL_CONNECTED))
    {
      // if WiFi is Connected
      HTTPClient https;
      https.setConnectTimeout(30000);
      https.setTimeout(30000);
      if (https.begin(baseUrl + "/api/transaction/vend-confirmation"))
      {
        // const String vendeddatarequest = "{\"txnId\":\"" + txnId + "\",\"productVended\":\"true\",\"error\":\"null\"}";
        // //HTTP header has been send and Server response header has been handled
        https.addHeader("Content-Type", "application/json");

        // start connection and send HTTP header
        int httpCode = https.POST(payload);
        String httpcodeStr = String(httpCode);
        Serial.printf("Vend-Confirmation ===> Response Code : %d\n", httpCode);

        if (httpCode == HTTP_CODE_OK)
        {
          Serial.println(https.getString());
          response = true;
        }
        else if (!errormsgdisplayed && httpCode != 200)
        {
          updateLogBody("vend confirmation error:", httpcodeStr);
          errormsgdisplayed = true;
        }
        https.end();
      }
      else
      {
        Serial.println("Vend-Confirmation ===> Unable To Connect");
      }
      delay(1000);
      retryCount++;
    }
  }
  paymentDeviceLogs();
  createNewOrder();
}

void setup()
{
  Serial.begin(9600);
  pinMode(relay, OUTPUT);
  pinMode(ltSwitch, INPUT);
  digitalWrite(relay, HIGH);
  tft.init();
  tft.setRotation(2);
  tft.setTextWrap(true);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_RED);
  tft.setTextSize(2);

  tft.drawString(WiFi.macAddress(), 10, 10, 1);
  tft.setTextColor(TFT_GREEN);
  Serial.print("Connecting to WiFi");
  tft.drawString("Connecting to WiFi", 10, 40, 1);

  tft.setTextSize(1.5);
  tft.setTextColor(TFT_CYAN);
  tft.drawString("Connect to NutritapAP to configure", 10, 70, 1.5);
  while ((WiFi.status() != WL_CONNECTED))
  {

    wm.setConfigPortalTimeout(45); // 45
    wm.startConfigPortal("NutritapAP", "ntpd0011");
    bool autores = wm.autoConnect("NutritapAP", "ntpd0011");
    if (autores)
    {
      // if you get here you have connected to the WiFi
      Serial.println("connected...yeey :)");
    }
    else
    {
      Serial.println("unable to connect");
    }
  }
  tft.setTextColor(TFT_YELLOW);
  tft.drawString("Connected to wifi..!!!", 10, 90, 1);
  tft.drawString("SSId: " + wm.getWiFiSSID(), 10, 110, 1.9);
  Serial.println("WiFi Connected");
  // getkiosk();
  // getMachinePlano();
  // createNewOrder();
}

void loop()
{
  while (configFound && !qrDisplayed)
  {
    show_QR("upi://pay?pa=paytmqrh37lhle44b@paytm&pn=Paytm");
  }
  while (configFound && qrDisplayed && WiFi.status() == WL_CONNECTED && !creditFound)
  {
    pollHandler();
  }
  while (creditFound && productVended)
  {
    handleVending();
  }
}
