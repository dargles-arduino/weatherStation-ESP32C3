# weatherStation-ESP32C3
This is the latest iteration of my solar-powered weather station project, updated to use the ESP32C3. The original version used a Wemos D1 Mini (ESP8266), powered by four AA rechargable NiCad batteries which were recharged by a small (60x110mm) 6V (nominal) solar panel. Whilst this works well in the summer months, the challenge has always been getting it to survive the winter months.

The project has been a valuable learning exercise - on the back of what I've learnt from this project, we've installed solar panels on our roof at home and have been very pleased with the results so far. There are several components to achieving the aim of getting a twelve month operational span purely powered by the sun:

* The solar panel; and maximising the charge obtained from the panel.
* The storage battery. This also needs to be protected from overcharge and over-discharge.
* The electronics. Deep sleep between readings is important, but minimising the current drain during deep sleep is crucial.

*The Solar Panel*


a development of the Weather Station code to allow it to work with the ESP32C3
