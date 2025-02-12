#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <map>
#include <vector>
#include <secrets.h>
#include <sstream>
#include <FastLED.h>
using namespace std;

#define LED_BUILTIN 2 //  the internal pin for the onboard LED is 2

// Define your Wi-Fi credentials
const char* ssid = SECRET_SSID;
const char* password = SECRET_PASSWORD;

// API key
const char* app_id = SECRET_APP_ID;

// Define routes
const int routes[] = { 90, 100, 190, 200, 290 };  // Red, Blue, Yellow, Green, Orange

// Struct for storing Active Vehicles
struct ActiveVehicles {
  int vehicle_id;
  int feet;
  int stop_id;
  int stop_index;
  string line;
  String stop_name;
};

// Struct for storing stop information
struct Stop {
  String stop_name;
  int stop_id;
  int feet;
};

// Define the number of LEDs on each strip
#define NUM_YELLOW_LINE_LEDS 17
#define NUM_RED_LINE_LEDS 37
#define NUM_BLUE_LINE_LEDS 48
#define NUM_GREEN_LINE_LEDS 30
#define NUM_ORANGE_LINE_LEDS 17

// Define the pin numbers for each strip (update to match your wiring)
#define YELLOW_LINE_PIN    14
#define RED_LINE_PIN       15
#define BLUE_LINE_PIN      16
#define GREEN_LINE_PIN     17
#define ORANGE_LINE_PIN    18

// Create FastLED objects for each strip
CRGB yellowLine[NUM_YELLOW_LINE_LEDS];
CRGB redLine[NUM_RED_LINE_LEDS];
CRGB blueLine[NUM_BLUE_LINE_LEDS];
CRGB greenLine[NUM_GREEN_LINE_LEDS];
CRGB orangeLine[NUM_ORANGE_LINE_LEDS];

//  map of each line and their respective stops in a pair, with first being the stop ID and second being the stop index.
const std::map<string, vector<pair<int, int>>> list_of_stops = {
  { "Red", { { 9838, 0 }, { 9839, 1 }, { 9835, 2 }, { 9834, 3 }, { 9831, 4 }, { 9830, 5 }, { 9828, 6 }, { 9822, 7 }, { 9826, 8 }, { 9824, 9 }, { 9821, 10 }, { 9969, 11 }, { 10120, 12 }, { 10118, 13 }, { 9758, 14 }, { 8333, 15 }, { 8334, 16 }, { 8336, 17 }, { 8337, 18 }, { 8338, 19 }, { 8339, 20 }, { 8340, 21 }, { 8341, 22 }, { 8342, 23 }, { 8343, 24 }, { 8344, 25 }, { 8345, 26 }, { 8346, 27 }, { 8347, 28 }, { 10572, 29 }, { 10574, 30 }, { 10576, 31 }, { 10579, 32 }, { 14250, 33 }, { 8381, 34 }, { 8383, 35 }, { 8384, 36 } } },
  { "Blue", { { 9848, 0 }, { 9846, 1 }, { 9843, 2 }, { 9841, 3 }, { 9838, 4 }, { 9839, 5 }, { 9835, 6 }, { 9834, 7 }, { 9831, 8 }, { 9830, 9 }, { 9828, 10 }, { 9822, 11 }, { 9826, 12 }, { 9824, 13 }, { 9821, 14 }, { 9969, 15 }, { 10120, 16 }, { 10118, 17 }, { 9758, 18 }, { 8333, 19 }, { 8334, 20 }, { 8336, 21 }, { 8384, 22 }, { 8383, 23 }, { 8381, 24 }, { 8337, 25 }, { 8338, 26 }, { 8339, 27 }, { 8340, 28 }, { 8341, 29 }, { 8342, 30 }, { 8343, 31 }, { 8344, 32 }, { 8345, 33 }, { 8346, 34 }, { 8347, 35 }, { 8348, 36 }, { 8349, 37 }, { 8350, 38 }, { 8351, 39 }, { 8352, 40 }, { 8353, 41 }, { 8354, 42 }, { 8355, 43 }, { 13450, 44 }, { 8356, 45 }, { 8357, 46 }, { 8359, 47 } } },
  { "Yellow", { { 10293, 0 }, { 7774, 1 }, { 13123, 2 }, { 7777, 3 }, { 7787, 4 }, { 9299, 5 }, { 7763, 6 }, { 11508, 7 }, { 11509, 8 }, { 11510, 9 }, { 11511, 10 }, { 11512, 11 }, { 11513, 12 }, { 11514, 13 }, { 11515, 14 }, { 11516, 15 }, { 11498, 16 } } },
  { "Green", { { 13132, 0 }, { 13130, 1 }, { 13129, 2 }, { 13128, 3 }, { 13127, 4 }, { 13126, 5 }, { 13125, 6 }, { 13124, 7 }, { 8347, 8 }, { 8346, 9 }, { 8345, 10 }, { 8344, 11 }, { 8343, 12 }, { 8342, 13 }, { 8341, 14 }, { 8340, 15 }, { 7601, 16 }, { 9303, 17 }, { 7627, 18 }, { 7646, 19 }, { 7608, 20 }, { 7618, 21 }, { 7606, 22 }, { 10293, 23 }, { 7774, 24 }, { 13123, 25 }, { 7777, 26 }, { 7787, 27 }, { 9299, 28 }, { 7763, 29 } } },
  { "Orange", { { 13720, 0 }, { 13721, 1 }, { 13722, 2 }, { 13723, 3 }, { 13724, 4 }, { 13725, 5 }, { 13726, 6 }, { 13727, 7 }, { 13728, 8 }, { 13729, 9 }, { 7606, 10 }, { 7618, 11 }, { 7608, 12 }, { 7646, 13 }, { 7627, 14 }, { 9303, 15 }, { 7601, 16 } } }
};


// function for pulling words out of a string
vector<string> split_sentence(string sen) {
    // Create a stringstream object
    stringstream ss(sen);
    // Variable to hold each word
    string word;
    // Vector to store the words
    vector<string> words;
    // Extract words using getline with space as the delimiter
    while (getline(ss, word, ' ')) {
        // Add the word to the vector
        words.push_back(word);
    }
    return words;
}


// Function to get stops for a given route
std::map<int, String> getStops(int route_id) {
  std::map<int, String> stop_ids;
  char url[200];
  snprintf(url, sizeof(url), "https://developer.trimet.org/ws/V1/routeConfig?appID=%s&route=%d&stops=true&dir=true&json=true", app_id, route_id);

  HTTPClient http;
  http.begin(url);
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, payload);

    JsonArray routeArray = doc["resultSet"]["route"]; //  routeArray is an array of routes that the GET call provides
    for (JsonObject route : routeArray) { // iterate through each route in routeArray
      if (route["routeSubType"] != "Light Rail") continue;  // ignore routes for Busses, Commuter Train, and StreetCars
      JsonArray dirArray = route["dir"];
      for (JsonObject dir : dirArray) {
        JsonArray stopArray = dir["stop"];  //  an array of stops for that direction of the MAX (Inbound / Outbound)
        for (JsonObject stop : stopArray) {
          String stop_desc = stop["desc"].as<String>(); // stop_desc is the name of the stop (Rose Quarter Transit Center, N Rosa Parks, etc)
          int locid = stop["locid"];  // locid is the ID for that stop (8224, 29990, etc)
          stop_ids[locid] = stop_desc;  //  add the stop_desc to our map, where the key is that stop's locid
        }
      }
    }
  }
  else {  //  error handling
    Serial.print("HTTP Request failed with error code: ");
    Serial.println(httpCode);
    http.end();
    return stop_ids;
  }
  http.end();
  return stop_ids;  //  return our map of stop_ids
}


// Function to get arrivals for a given set of stops. Takes a map of stop_ids that we get from getStops(), and a vector of ActiveVehicles that we generate/update in this function.
vector<ActiveVehicles> getArrivals(std::map<int, String> stop_ids, vector<ActiveVehicles> active_vehicles) {
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)

  String stop_list = "";  //  making a batch call to the API requires us to turn our map of stop_ids into a commam seperated string
  for (auto& stop : stop_ids) {
    stop_list += String(stop.first) + ",";  //  make sure  comma seperated
  }
  stop_list.remove(stop_list.length() - 1);  // Remove the last comma
  String url = "https://developer.trimet.org/ws/V2/arrivals?appID=" + String(app_id) + "&locIDs=" + stop_list + "&json=true";

  HTTPClient http;
  http.begin(url);
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, payload);
    JsonArray arrivalArray = doc["resultSet"]["arrival"]; //  an array of arrivals that we get from Trimet

    for (JsonObject arrival : arrivalArray) { //  iterate through the array
      if (arrival["routeSubType"] != "Light Rail") continue;  //  Ignore Buses, Commuter Rail, and StreetCars
      if (arrival["status"] == "scheduled") continue; //  Ignore scheduled trains
      if (!arrival["departed"]) continue; //  Ignore trains that haven't departed
      if (arrival["feet"] > 2000) continue;  // Ignore vehicles too far away
      
      ActiveVehicles current_vehicle; //  Create an ActiveVehicle object to track the current vehicle we're looking at

      //  set our variables using information from the arrival data
      int vehicle_id = arrival["vehicleID"];
      int stop_id = arrival["locid"];
      string sign = arrival["shortSign"];
      string line = split_sentence(sign)[0];
      String stop_name = stop_ids[arrival["locid"]];
      //  check to make sure that the stop is reporting info on the right train (Some stops will report on trains that are nearby that aren't a part of its route)
      auto check = list_of_stops.find(line);  // pull the list of stops for the line (Red, Yellow, Blue, Green, Orange) from list_of_stops
      auto stop_index = find_if(check->second.begin(), check->second.end(),[stop_id](const auto& p)   // iterate through the list of stops in the route to try and find our stop ID
                    { return p.first == stop_id; });
      if (stop_index == check->second.end()){ // if it's not in the list of stops, skip this vehicle
        continue;
      }

      int feet = arrival["feet"];
      auto it = std::find_if(active_vehicles.begin(), active_vehicles.end(),
                             [vehicle_id](const ActiveVehicles& vehicle) {
                               return vehicle.vehicle_id == vehicle_id;
                             });

      if (it != active_vehicles.end()) {                         // if the vehicle exists in active_vehicles:
        int index = std::distance(active_vehicles.begin(), it);  // if the vehicle exists in active_vehicles already, get the index so we can reference it in active_vehicles
        int old_feet = active_vehicles[index].feet;              // get the distance that's currently in active_vehicles
        if (old_feet > feet) {  //  if the stop reports that the train is closer to it than the stop that's currently in active_vehicles, overwrite the value in active_vehicles w/ this new stop
          active_vehicles[index].vehicle_id = vehicle_id;
          active_vehicles[index].stop_id = stop_id;
          active_vehicles[index].line = line;
          active_vehicles[index].stop_name = stop_name;
          active_vehicles[index].feet = feet;
          active_vehicles[index].stop_index = stop_index->second;
        }
      } else {                                     // if the vehicle meets the "active vehicle" requirements and doesnt exist in active_vehicles
        current_vehicle.vehicle_id = vehicle_id;
        current_vehicle.stop_id = stop_id;
        current_vehicle.line = line;
        current_vehicle.stop_name = stop_name;
        current_vehicle.feet = feet;
        current_vehicle.stop_index = stop_index->second;
        active_vehicles.push_back(current_vehicle);  // add the current_vehicle to the active_vehicles vector
      }
      // Print active vehicle info
      //Serial.printf("Stop ID %d | Line: %s | Stop: %s | Vehicle ID: %d |Feet Away: %d\n", stop_id, line.c_str(), stop_name.c_str(), vehicle_id, feet);
    }
  }
  else {
    Serial.print("HTTP Request failed with error code: ");
    Serial.println(httpCode);
    http.end();
    return active_vehicles;
}
  http.end();
  digitalWrite(LED_BUILTIN, LOW);
  return active_vehicles;
}


// main method. parse through the list of stops, see if a train is arriving, if it is, print that mf.
vector<ActiveVehicles> getActiveStops() {
  vector<ActiveVehicles> active_vehicles;

  for (int route : routes) {                           // iterate through the routes
    std::map<int, String> stop_ids = getStops(route);  //pull a map of the stops from each route
    if (stop_ids.empty()) {
      Serial.println("No stops found!");
      continue;
    }
    Serial.println("Getting arrivals for stops...");
    active_vehicles = getArrivals(stop_ids, active_vehicles);
  }
  int active_stop_count = active_vehicles.size();
  Serial.printf("Active Vehicle Count: %d \n", active_stop_count);

  /*  debugging
  for (const auto& vehicle : active_vehicles) {
    Serial.print(", Stop ID: ");
    Serial.print(vehicle.stop_id);
    Serial.print("Line: ");
    Serial.print(vehicle.line.c_str());
    Serial.print(", Stop Index: ");
    Serial.println(vehicle.stop_index);
  }
  */
  return active_vehicles;
}


void updateLEDs(vector<ActiveVehicles> active_vehicles){
  // clear the LEDs before we turn on our new ones
  fill_solid(yellowLine, NUM_YELLOW_LINE_LEDS, CRGB::Black);
  fill_solid(redLine, NUM_RED_LINE_LEDS, CRGB::Black);
  fill_solid(blueLine, NUM_BLUE_LINE_LEDS, CRGB::Black);
  fill_solid(greenLine, NUM_GREEN_LINE_LEDS, CRGB::Black);
  fill_solid(orangeLine, NUM_ORANGE_LINE_LEDS, CRGB::Black);

  for (const auto& active_stop : active_vehicles) { //  iterate through each active_vehicle in active_vehicles
    String color = active_stop.line.c_str();  //  pull the color of the line (Red,Yellow,Green,Blue,Orange)
    int position = active_stop.stop_index;  //  pull the index of the stop on the line (we use this to tell FastLED which led on which strip to turn on)

    //  debugging 
    Serial.print("Active Station on the ");
    Serial.print(color);
    Serial.print(" Line at array position ");
    Serial.print(position);
    Serial.print("\n");

    // Determine which strip and turn on the LED at the specified position
    if (color == "Yellow") {
      if (position < NUM_YELLOW_LINE_LEDS) {  //  make sure we're not trying to turn on a stop that's out of bounds for that line
        yellowLine[position] = CRGB::Yellow;  // Set LED to Yellow
      }
    } else if (color == "Red") {
      if (position < NUM_RED_LINE_LEDS) { 
        redLine[position] = CRGB::Red;  // Set LED to Red
      }
    } else if (color == "Blue") {
      if (position < NUM_BLUE_LINE_LEDS) { 
        blueLine[position] = CRGB::Blue;  // Set LED to Blue
      }
    } else if (color == "Green") {
      if (position < NUM_GREEN_LINE_LEDS) { 
        greenLine[position] = CRGB::Green;  // Set LED to Green
      }
    } else if (color == "Orange") {
      if (position < NUM_ORANGE_LINE_LEDS) { 
        orangeLine[position] = CRGB::Orange;  // Set LED to Orange
      }
    }
    // Update the LEDs
    FastLED.show();
    //delay(100); // Delay between actions for visibility
  }
}


//  connect to wifi
void connectWifi() {
  WiFi.mode(WIFI_STA);  // Set ESP32 to station mode
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  
  // Wait until connected
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi!");
}


// Main setup
void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT); //  enable the onboard LED (blue light will blink when pulling data from trimet)
  connectWifi();

   // Initialize each strip with its respective pin
  FastLED.addLeds<WS2812, YELLOW_LINE_PIN, GRB>(yellowLine, NUM_YELLOW_LINE_LEDS);
  FastLED.addLeds<WS2812, RED_LINE_PIN, GRB>(redLine, NUM_RED_LINE_LEDS);
  FastLED.addLeds<WS2812, BLUE_LINE_PIN, GRB>(blueLine, NUM_BLUE_LINE_LEDS);
  FastLED.addLeds<WS2812, GREEN_LINE_PIN, GRB>(greenLine, NUM_GREEN_LINE_LEDS);
  FastLED.addLeds<WS2812, ORANGE_LINE_PIN, GRB>(orangeLine, NUM_ORANGE_LINE_LEDS);

  // Initially turn on all LEDs
  fill_solid(yellowLine, NUM_YELLOW_LINE_LEDS, CRGB::Yellow);
  fill_solid(redLine, NUM_RED_LINE_LEDS, CRGB::Red);
  fill_solid(blueLine, NUM_BLUE_LINE_LEDS, CRGB::Blue);
  fill_solid(greenLine, NUM_GREEN_LINE_LEDS, CRGB::Green);
  fill_solid(orangeLine, NUM_ORANGE_LINE_LEDS, CRGB::Orange);
  FastLED.show();
}


  // Main loop - every 10 seconds, call getActiveStops(), and feed the list of active_vehicles to updateLEDs()
void loop() {
  //  clear stops (turn all LEDs)
  updateLEDs(getActiveStops());
  delay(10000);  // 10s delay between updating stops
}

