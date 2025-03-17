#ifndef COFIG_HPP_
#define COFIG_HPP_
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFiManager.h>
String vendDetails = "";
String vendConfirmation = "";
struct VendDetail
{
  String coilIdentifier = "\0";
  float amount = 0.0;
  bool success = false;
  String error = "\0";
};
VendDetail vendDetail;
static const struct VendDetail emptyVendDetail;

void parseVendDetails(String coilIdentifier, float amount, bool success, String error)
{
  String vendDetail = "{\"coilIdentifier\":\"" + coilIdentifier + "\",\"amount\":\"" + amount + "\",\"success\":" + (success ? "true" : "false");
  if (success == false)
  {
    vendDetail += ",\"error\":\"" + error + "\"";
  }
  vendDetail += "},";
  vendDetails += vendDetail;
}
void paymentDeviceLogs();
void transactionHandle(String payload);
// vend confirmation function
void parseVendConfirmation(String txnId, bool productVended, String error, String rfidCard)
{
  vendConfirmation = "";
  
  vendConfirmation = "{\"txnId\":\"" + txnId + "\",\"productVended\":" + (productVended ? "true" : "false");

  if (productVended == false)
  {
    vendConfirmation += ",\"error\":\"" + error + "\"";
  }
}
// geting the vend payload confirmation
String getVendConfirmationPayload()
{
  vendDetails = vendDetails.substring(0, vendDetails.length() - 1);
  String payload = vendConfirmation + ",\"selectionDtoList\":[" + vendDetails + "]}";
  Serial.print("getVendConfirmationPayload ====> payload : ");
  Serial.println(payload);
  vendDetails = "";
  return payload;
}
String logBody = "";
void updateLogBody(const String msg, const String data)
{
  logBody += msg + "\t==>\t" + data + "\n";
}
void findUpdateLogBody()
{
  int index = logBody.indexOf("error duration");
  if (index > -1)
  {
    logBody = logBody.substring(0, index);
  }
}
unsigned long startTime = millis();
void startTimer()
{
  startTime = millis();
}

int elaspedTime()
{
  unsigned long currTime = millis();
  return (currTime - startTime) / 1000;
}


// creating new order
#endif
