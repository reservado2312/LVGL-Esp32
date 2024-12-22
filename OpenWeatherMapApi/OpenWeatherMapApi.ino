/*
  Rui Santos
  Complete project details at Complete project details at https://RandomNerdTutorials.com/esp32-http-get-open-weather-map-thingspeak-arduino/

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/

#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>

const char* ssid = "Rede Casa";
const char* password = "gerlianexavier23";

// Your Domain name with URL path or IP address with path
String openWeatherMapApiKey = "61855384d32841d12b366a61d18064b7";
// Example:
//String openWeatherMapApiKey = "bd939aa3d23ff33d3c8f5dd1dd435";

// Replace with your country code and city
String city = "Pitangueiras";
String countryCode = "Brasil";

// THE DEFAULT TIMER IS SET TO 10 SECONDS FOR TESTING PURPOSES
// For a final application, check the API call limits per hour/minute to avoid getting blocked/banned
unsigned long lastTime = 0;
// Timer set to 10 minutes (600000)
//unsigned long timerDelay = 600000;
// Set timer to 10 seconds (10000)
unsigned long timerDelay = 10000;

String jsonBuffer;

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
 
  //Serial.println("Timer set to 10 seconds (timerDelay variable), it will take 10 seconds before publishing the first reading.");
  Serial.println("o Timer foi Configurado para exibir os dados meteriologicos  a cada 10 segundos.");
}

void loop() {
  // Send an HTTP GET request
  if ((millis() - lastTime) > timerDelay) {
    // Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
      String serverPath = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "," + countryCode + "&APPID=" + openWeatherMapApiKey;
      
      jsonBuffer = httpGETRequest(serverPath.c_str());
     // Serial.println(jsonBuffer);
      JSONVar myObject = JSON.parse(jsonBuffer);
  
      // JSON.typeof(jsonVar) can be used to get the type of the var
      if (JSON.typeof(myObject) == "undefined") {
        Serial.println("Parsing input failed!");
        return;
      }
    
      //Serial.print("JSON object = ");
      //Serial.println(myObject);
      Serial.println("");              
      Serial.println("Dados Meteriologicos de: " + city);
      Serial.println("");              
      if (myObject.hasOwnProperty("main")) {
        if (myObject["main"].hasOwnProperty("temp")) {
            float tempKelvin = (float)(double)myObject["main"]["temp"]; // Conversão explícita para double e depois para float
            float tempCelsius = tempKelvin - 273.15;    // Converte para Celsius
            Serial.print("Temperatura: ");
            Serial.print(tempCelsius, 1);               // Exibe com uma casa decimal
            Serial.println(" C° ");              
        }
      }

      Serial.print("Pressão Atimosférica: ");
      Serial.print(myObject["main"]["pressure"]);
      Serial.println(" hPa ");
      Serial.print("Umidade: ");
      Serial.print(myObject["main"]["humidity"]);
      Serial.println(" % ");

      if (myObject.hasOwnProperty("wind")) {
        if (myObject["wind"].hasOwnProperty("speed")) 
        {
            float windSpeedMps = (float)(double)myObject["wind"]["speed"]; // Conversão explícita para double e depois para float            
            float windSpeedKmh = windSpeedMps * 3.6;        // Converte para km/h
            Serial.print("Velocidade do vento: ");
            Serial.print(windSpeedKmh, 1);                  // Exibe com uma casa decimal
            Serial.println(" km/h");
        }
      }
      if (myObject.hasOwnProperty("weather")) {
        JSONVar weatherArray = myObject["weather"];
        if (weatherArray.length() > 0) {
          JSONVar weather = weatherArray[0];
          String main = String((const char*)weather["main"]);
          String description = String((const char*)weather["description"]);

          // Traduções para português
          String mainPt;
          String descriptionPt;

          // Mapeamento de traduções
          if (main == "Clear") mainPt = "Limpo";
          else if (main == "Clouds") mainPt = "Nuvens";
          else if (main == "Rain") mainPt = "Chuva";
          else if (main == "Snow") mainPt = "Neve";
          else if (main == "Thunderstorm") mainPt = "Trovoada";
          else if (main == "Drizzle") mainPt = "Garoa";
          else if (main == "Atmosphere") mainPt = "Atmosfera";
          else mainPt = "Desconhecido";

          if (description == "clear sky") descriptionPt = "Céu limpo";
          else if (description == "few clouds") descriptionPt = "Poucas nuvens";
          else if (description == "scattered clouds") descriptionPt = "Nuvens dispersas";
          else if (description == "broken clouds") descriptionPt = "Nuvens quebradas";
          else if (description == "overcast clouds") descriptionPt = "Nuvens encobertas";
          else if (description == "light rain") descriptionPt = "Chuva leve";
          else if (description == "moderate rain") descriptionPt = "Chuva moderada";
          else if (description == "heavy intensity rain") descriptionPt = "Chuva forte";
          else if (description == "light snow") descriptionPt = "Neve leve";
          else if (description == "fog") descriptionPt = "Nevoeiro";
          else descriptionPt = "desconhecido";

          // Exibe os valores traduzidos
          Serial.print("Clima Atual: ");
          Serial.println(mainPt);
          Serial.print("Descrição: ");
          Serial.println(descriptionPt);
        }
      }       
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
}

String httpGETRequest(const char* serverName) {
  WiFiClient client;
  HTTPClient http;
    
  // Your Domain name with URL path or IP address with path
  http.begin(client, serverName);
  
  // Send HTTP POST request
  int httpResponseCode = http.GET();
  
  String payload = "{}"; 
  
  if (httpResponseCode>0) {
    //Serial.print("HTTP Response code: ");
    //Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();

  return payload;
}