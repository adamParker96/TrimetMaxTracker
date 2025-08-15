Small C++ program for an ESP32. Used to power a live train tracker w/ LEDs and a map.
Uses the Trimet API to pull information on active MAX trains. If a train is pulling into/at a station, lights up that station.


My first attempt at building this ended in failure - I was originally using the station map provided by TriMet: <img width="11520" height="8640" alt="Trimet3x4v3" src="https://github.com/user-attachments/assets/5ea817e3-0a39-409d-9dd6-555ba97534f0" />
However, When I printed out the map and began to lay down the LEDs, I realized that even when blown up to 4'x3' the distance between the stops was too small to easily place an LED in it's required location. My limited soldering ability meant that I was NOT going to be able to make this work. 


So I went back and redesigned the map to have larger nodes for each station, as well as increasing the distance between each station and thickening each color line:

<img width="14400" height="10800" alt="trimetMapAug15th" src="https://github.com/user-attachments/assets/9c22a9ea-5bcd-4541-9c50-29d325443b96" />
