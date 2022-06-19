
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include "DHT.h"

#define DHTPIN 4 
#define DHTTYPE DHT11 

#define WIFI_SSID "xxxxxx"
#define WIFI_PASSWORD "xxxxxxxxxxxxxxxxxxxxxxxxx"
// Telegram BOT Token (Get from Botfather)
#define BOT_TOKEN "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"

DHT dht(DHTPIN, DHTTYPE);
const unsigned long BOT_MTBS = 1000; // mean time between scan messages

X509List cert(TELEGRAM_CERTIFICATE_ROOT);
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
unsigned long bot_lasttime; // last time messages' scan has been done

const int vol = LED_BUILTIN;
const int ledPin = 5;
int ledStatus = 0;
String ipAddress = "";

void handleNewMessages(int numNewMessages)
{
  Serial.print("Mesaj işleniyor..: ");
  Serial.println(numNewMessages);

  for (int i = 0; i < numNewMessages; i++)
  {
    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;
    Serial.print("Alınan mesaj: ");
    Serial.println(text);
    String from_name = bot.messages[i].from_name;
    if (from_name == "")
      from_name = "Guest";

    if (text == "/cicek")
    {
      digitalWrite(ledPin, HIGH);
      ledStatus = 1;
      bot.sendMessage(chat_id, "*Bitki sulanıyor. 🌸💧💦 \n5 saniye sonra işlem tamamlanacak! ⏳ *", "Markdown");
      delay(5000);
      ledStatus = 0;
      digitalWrite(ledPin, LOW);
      bot.sendMessage(chat_id, "*Bitki sulandı. 🌸💧✅ \nSu hayattır, teşekkürler. 🥰🦆 *", "Markdown");      
    }
    if (text == "/kapat")
    {
      ledStatus = 0;
      digitalWrite(ledPin, LOW);
      digitalWrite(vol, LOW);
      bot.sendMessage(chat_id, "*Sulama durduruldu. ⛔ \nSu pompası kapatıldı..! ❌ *", "Markdown");
      digitalWrite(vol, HIGH);
    }

    if (text == "/durum")
    {
      if (ledStatus)
      {
        bot.sendMessage(chat_id, "Bitki sulanıyor... 🌸💧", "");
      }
      else
      {
        String durum = "*Wifi SSID:      *" + WiFi.SSID() + "";
        durum += "\n";
        IPAddress ip = WiFi.localIP();
        ipAddress = ip.toString();
        durum += "*IP Adresi:    *" + ipAddress + "";
        durum += "\n";
        durum += "Bitki şuan sulanmıyor. 🚱⚠ \nSistem beklemede! ⏳ ";
        bot.sendMessage(chat_id, durum, "Markdown");
      }
    }

    if (text == "/iklim")
      {   
          float t = dht.readTemperature();
          float h = dht.readHumidity();
          String msg = "*Ortam Sıcaklığı :    *";
          msg += ("%0.2f", t);
          msg += " °C  🌡️\n";
          msg += "*Nem Oranı :             *";
          msg += ("%0.2f", h);
          msg += "  %  ♨";
          bot.sendMessage(chat_id,msg, "Markdown"); 
      }

    if (text == "/start")
    {
      String welcome = "*Hoşgeldin, *" + from_name + ". 🦆\n";
      welcome += "*Uzaktan Bitki Sulama Sistemi TA2KVC 2022* \n\n";
      welcome += "* /cicek : * Bitkiyi 5 saniye boyunca sulayıp, otomatik kapanır.\n";
      welcome += "* /iklim : * Ortam sıcaklığı ve nem oranı bilgilerini verir.\n";
      welcome += "* /kapat : * Sulamayı durdurup, su pompasını kapatır.\n";
      welcome += "* /durum : * Sistem durumu hakkında bilgi verir.\n";
      welcome += "* /start : * Başlangıç ekranını ve komutları listeler.\n";
      bot.sendMessage(chat_id, welcome, "Markdown");
    }
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println();
  dht.begin();
  pinMode(ledPin, OUTPUT);
  pinMode(vol, OUTPUT);
  delay(10);
  digitalWrite(ledPin, LOW);
  digitalWrite(vol, HIGH);
  // attempt to connect to Wifi network:
  configTime(0, 0, "pool.ntp.org");      // get UTC time via NTP
  secured_client.setTrustAnchors(&cert); // Add root certificate for api.telegram.org
  Serial.print("Wifi SSID bağlanıyor:  ");
  Serial.print(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.print("\nWiFi bağlandı. IP adres: ");
  Serial.println(WiFi.localIP());
  // Check NTP/Time, usually it is instantaneous and you can delete the code below.
  Serial.print("Tarih/Saat bilgisi eşitleniyor: ");
  time_t now = time(nullptr);
  while (now < 24 * 3600)
  {
    Serial.print(".");
    delay(500);
    now = time(nullptr);
  }
  Serial.println("\nUzaktan Bitki Sulama Sistemi TA2KVC 2022 \nSistem Hazır");
}

void loop()
{
  if (millis() - bot_lasttime > BOT_MTBS)
  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages)
    {
      Serial.println("Mesaj alındı:");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    bot_lasttime = millis();
  }
}
