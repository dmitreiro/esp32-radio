#include <Arduino.h>
#include <WiFi.h>
#include <Audio.h>
#include <ESPAsyncWebServer.h>
#include <ezButton.h>
#include "src/index_html.h"
#include "src/config.h"

// audio pins
#define I2S_DOUT      25  // connect to DAC pin DIN
#define I2S_BCLK      26  // connect to DAC pin BCK
#define I2S_LRC       27  // connect to DAC pin LCK

// volume control
#define VOLUME_CONTROL_STEPS  100
#define VOLUME  14

// define pins for buttons
#define CHAN_UP_PIN         5   //increases channel number
#define CHAN_DOWN_PIN       17  //decreases channel number
#define VOL_UP_PIN          16  //increases volume
#define VOL_DOWN_PIN        4   //decreases volume
#define DEBOUNCE_TIME 50
#define NUMBER_OF_CHANNELS  4  //this should match the number of URLs found in the connect() function below

// create buttons
ezButton upChanBtn(CHAN_UP_PIN);
ezButton downChanBtn(CHAN_DOWN_PIN);
ezButton upVolBtn(VOL_UP_PIN);
ezButton downVolBtn(VOL_DOWN_PIN);

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Create audio object
Audio audio;

// Auxiliar variables to store the current output state
String output26State = "off";
String output27State = "off";

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

int currentChannelNumber = 1; // starts channel list at 1

bool knobCommand = false; // when true, passes volume control to POT_PIN
bool sliderCommand = false; // when true, passes volume control to slider
int initPotVol;
int sliderVol = VOLUME;
int potVol;
int setpointVol;

// Internal volume variable
int volume = 14;

// Radio stream links
String radio1 = "http://streamlink1.com";
String radio2 = "http://streamlink2.com";
String radio3 = "http://streamlink3.com";
String radio4 = "http://streamlink4.com";
String currentStream = "";

void setupAudio(){
  Serial.println("Setting I2S output pins and initial volume level.");

  //configure the I2S output pins
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);

  //set the initial volume level
  audio.setVolumeSteps(VOLUME_CONTROL_STEPS);
  audio.setVolume(VOLUME);
  Serial.print("Inital volume set at ");
  Serial.print(VOLUME);
  Serial.println("%");
}

// void volCheck(){
//   int currPotVol = map(analogRead(POT_PIN), 0, 4095, 0, VOLUME_CONTROL_STEPS);
//   // if knob commands
//   if (knobCommand == true && sliderCommand == false && abs(sliderVol - setpointVol) >= 1 && sliderVol <= potVol) {
// 		knobCommand = false;
//     sliderCommand = true;
//     potVol = currPotVol;
// 	}
//   // if slider commands
//   else if (knobCommand == false && sliderCommand == true && abs(currPotVol - potVol) >= 10 && currPotVol <= sliderVol) {
//     knobCommand = true;
//     sliderCommand = false;
//     setpointVol = sliderVol;
//   }
// }

void connect(Audio *audio, int channel) {
  switch (channel){
  
    //  *** radio streams ***
    case 1:
    //(*audio).connecttohost("https://stream-icy.bauermedia.pt/m80.aac");
    //(*audio).connecttohost("https://github.com/pgiacalo/audio_test/raw/main/LeftRightCenterTest.mp3"); // audio left and right test
    (*audio).connecttohost("https://corus.leanstream.co/CJKRFM-MP3"); // rock 108
    break;
		
    case 2:
    (*audio).connecttohost("http://1.fm/tunestream/hits2000/listen.pls"); // 1.fm top hits 2000
    break;
    
    case 3:
    (*audio).connecttohost("https://webprod.sipse.com.mx:8080/kissfm/"); // kiss fm
    break;
  
    case 4:
    (*audio).connecttohost("http://stream.regenbogen2.de/bawue/aac-64/"); // 64 kbp/s aac+ rock fm
    break;
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(1500);

  // Print the IP address
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  
  setupAudio();
  Serial.println("------- Audio Setup Complete -------");

  // define pinmode for buttons
  pinMode(CHAN_UP_PIN, INPUT_PULLUP);
  pinMode(CHAN_DOWN_PIN, INPUT_PULLUP);
  pinMode(VOL_UP_PIN, INPUT_PULLUP);
  pinMode(VOL_DOWN_PIN, INPUT_PULLUP);

  // set debounce time for buttons
  upChanBtn.setDebounceTime(DEBOUNCE_TIME);
  downChanBtn.setDebounceTime(DEBOUNCE_TIME);
  upVolBtn.setDebounceTime(DEBOUNCE_TIME);
  downVolBtn.setDebounceTime(DEBOUNCE_TIME);

  Serial.println("Playing audio...");
  Serial.print("Playing Channel #");
  Serial.println(currentChannelNumber);
  
  connect(&audio, currentChannelNumber);

  // Serve the HTML page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  // Endpoint to get the current volume
  server.on("/getVolume", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = "{\"volume\":" + String(volume) + "}";
    request->send(200, "application/json", json);
  });

  // Endpoint to set the volume
  server.on("/setVolume", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("value")) {
      sliderVol = request->getParam("value")->value().toInt();
      //audio.setVolume(volume);
      //Serial.println("Volume set to: " + String(volume));
    }
    request->send(200, "text/plain", "OK");
  });

  // Endpoint to set the radio stream
  server.on("/setStream", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("radio")) {
      String radio = request->getParam("radio")->value();
      if (radio == "radio1") currentStream = radio1;
      else if (radio == "radio2") currentStream = radio2;
      else if (radio == "radio3") currentStream = radio3;
      else if (radio == "radio4") currentStream = radio4;
      Serial.println("Current stream set to: " + currentStream);
    }
    request->send(200, "text/plain", "OK");
  });

  // Endpoint to set custom stream
  server.on("/setCustomStream", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("link")) {
      currentStream = request->getParam("link")->value();
      Serial.println("Current stream set to: " + currentStream);
    }
    request->send(200, "text/plain", "OK");
  });

  // Endpoint to get the current stream
  server.on("/getCurrentStream", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = "{\"currentStream\":\"";
    if (currentStream == radio1) json += "radio1";
    else if (currentStream == radio2) json += "radio2";
    else if (currentStream == radio3) json += "radio3";
    else if (currentStream == radio4) json += "radio4";
    else json += "";
    json += "\"}";
    request->send(200, "application/json", json);
  });

  // Start server
  server.begin();
}

void loop() {
  audio.setVolume(volume);

  bool changingChannels = false;

  upChanBtn.loop();
	downChanBtn.loop();
  upVolBtn.loop();
	downVolBtn.loop();

  if (upChanBtn.isReleased()) {
		Serial.println("Up channel button pressed");
    changingChannels = true;
    currentChannelNumber = currentChannelNumber + 1;
    if (currentChannelNumber > NUMBER_OF_CHANNELS){
      currentChannelNumber = 1;
    }
  }

  if (downChanBtn.isReleased()) { 
    Serial.println("Down channel button pressed");
    changingChannels = true;
    currentChannelNumber = currentChannelNumber - 1;
    if (currentChannelNumber < 1){
      currentChannelNumber = NUMBER_OF_CHANNELS;
    }
	}

  if (upVolBtn.isReleased()) {
		Serial.println("Up volume button pressed");
    volume = volume + 3;
  }

  if (downVolBtn.isReleased()) { 
    Serial.println("Down volume button pressed");
    volume = volume - 3;
	}

  if (changingChannels){
    Serial.print("Playing Channel #");
    Serial.println(currentChannelNumber);
    connect(&audio, currentChannelNumber);
  }

  audio.loop();
}

void audio_info(const char *info){
  Serial.print("info        ");
  Serial.println(info);
}
