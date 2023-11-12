# weatherStation-ESP32C3
This is the latest iteration of my solar-powered weather station project, updated to use the ESP32C3. The original version used a Wemos D1 Mini (ESP8266), powered by four AA rechargable NiCad batteries which were recharged by a small (60x110mm) 6V (nominal) solar panel. Whilst this works well in the summer months, the challenge has always been getting it to survive the winter months.

The project has been a valuable learning exercise - on the back of what I've learnt from this project, we've installed solar panels on our roof at home and have been very pleased with the results so far. There are several components to achieving the aim of getting a twelve month operational span purely powered by the sun:

* The solar panel; and maximising the charge obtained from the panel.
* The storage battery. This also needs to be protected from overcharge and over-discharge.
* The electronics. Deep sleep between readings is important, but minimising the current drain during deep sleep is crucial.
* The coding. Choosing the length of deep sleep is important, but also organising the coding so heavy current drain is generally minimised.

*The Solar Panel*

I've stuck with the original 60x110mm 6V nominal solar panel throughout the project. One easy way to achieve the aim of lasting throughout the winter months _might_ be to double the size (and capacity) of the solar panel, but (a) that feels like cheating, (b) it will stop me learning about other aspects of the project, and (c) there's every likelihood that such an approach won't actually work in practice. But recently, I've come across a CN3065-based MPPT charger for these panels, and first impressions are that it makes quite a difference as opposed to simply connecting the panel to the battery via a diode. The solar panel and the MPPT charge controller are available as a package from AliExpress. I can't find that exact package, but the board can be found here:

https://www.aliexpress.com/item/1005005898470290.html?spm=a2g0o.detail.0.0.71e63b32zJkK80&gps-id=pcDetailTopMoreOtherSeller&scm=1007.40000.327270.0&scm_id=1007.40000.327270.0&scm-url=1007.40000.327270.0&pvid=d029ae44-1ee1-4d7f-909f-4667bae6bec1&_t=gps-id:pcDetailTopMoreOtherSeller,scm-url:1007.40000.327270.0,pvid:d029ae44-1ee1-4d7f-909f-4667bae6bec1,tpp_buckets:668%232846%238115%232000&pdp_npi=4%40dis%21GBP%210.78%210.69%21%21%216.77%21%21%402103872a16998120740861996eb301%2112000034760558665%21rec%21UK%21%21AB&search_p4p_id=202311121001141305433920249196579209_1

No, I don't get any commission for giving that link ;)

*The Storage Battery*

I started by using four 1.5V nominal rechargable AA NiCad batteries, since these give a decent working voltage for 5V circuits. However, experience has taught me that LiPos are a much better bet for longevity, and the 18650 in particular sems to be a very good battery for working with. Although this has a lower working voltage of around 3.7V, this is fine for working with modern 3.3V circuitry. In addition, using an MPPT charge controller (see previous section) does a very good job of using the solar panel to keep the battery as efficiently maintained as possible. A further step forward is to include a DW01A-based board to protect the battery from deep discharge, maintaining its life significantly.

AliExpress: https://www.aliexpress.com/item/1005004332269949.html?spm=a2g0o.productlist.main.9.45b540f1o5WnxO&algo_pvid=48d111f5-56fc-4b1b-9fd0-c104b14eeb62&aem_p4p_detail=2023111209535211598463353544510001769969&algo_exp_id=48d111f5-56fc-4b1b-9fd0-c104b14eeb62-4&pdp_npi=4%40dis%21GBP%213.51%210.41%21%21%214.17%21%21%402103846916998116328994210e381f%2112000029555291294%21sea%21UK%210%21AB&curPageLogUid=RwOJRZhx6sLK&search_p4p_id=2023111209535211598463353544510001769969_5

*The Electronics*

It quickly became clear that the original D1 Mini was drawing a lot more current in deep sleep than should be necessary on an ESP8266. It transpired that this was due to the board using an AMS1117 voltage regulator which draws 4mA in quiescent mode - way too much for a project minimising current drain! So the first step was to design my own ESP8266 board using a TC1262, which draws only 80uA in quiescent mode. This was a big improvement, but a pain to have to build my own board any time I wanted to create a new weather station. Most recently the ESP32C3 has become available at a very competitive price, hence this version of the weathger station project being based on that chip. The ESP32C3 is advertised as drawing about 40uA in deep sleep; but the boards as provided draw ten times that amount, simply because they are provided with an always-on red led. So the answer to that is to remove the red power-on led, and we're back down to 40uA in deep sleep.

The ESP32C3: https://www.aliexpress.com/item/1005005967641936.html?spm=a2g0o.productlist.main.11.eeb35df98WtiZi&algo_pvid=f013d17c-acf7-4f11-a2ed-5590b61fe6ed&aem_p4p_detail=20231112102738871229939802600001816214&algo_exp_id=f013d17c-acf7-4f11-a2ed-5590b61fe6ed-5&pdp_npi=4%40dis%21GBP%216.34%210.41%21%21%2154.91%21%21%40211b600d16998136587746696e1633%2112000035107991749%21sea%21UK%210%21AB&curPageLogUid=T5DDRSRMt7Jm&search_p4p_id=20231112102738871229939802600001816214_6

The story of inefficient voltage regulators continues with the weather sensors. These can be picked up in 5V and 3.3V versions. Avoid the 5V versions - they use horrendous voltage regulators that draw around 2mA in quiescent mode. Instead go for the 3.3V versions and power them from the 3.3V supply from the ESP. Then the quiescent drain drops to a few uA. Also, the BME280 is quite expensive (beware of BMP280s sold as if they are BME280s). But there is a hybrid board which provides a BMP280 alongside an AHT20. This works every bit as well as a BME280 and is a lot cheaper. The only thing to watch out for is that the hybrid board changes the I2C address of the BMP280 to the non-standard alternative address. But my code here now automatically detects that and changes the address dynamically to cope.

On AliExpress: https://www.aliexpress.com/item/1005005652004834.html?spm=a2g0o.productlist.main.7.19c76032rHcd3g&algo_pvid=dc94f38a-73ca-421e-85d2-b897d7ea2681&aem_p4p_detail=20231112103254284459094958970001802937&algo_exp_id=dc94f38a-73ca-421e-85d2-b897d7ea2681-3&pdp_npi=4%40dis%21GBP%219.14%214.93%21%21%2179.16%21%21%40211b615316998139746403797e7ce7%2112000033892851030%21sea%21UK%210%21AB&curPageLogUid=HbCUZe4JmArQ&search_p4p_id=20231112103254284459094958970001802937_4

Be careful - this looks to be the right version, but make sure it doesn't have a silly voltage regulator on it.

The final issue is the OLED screen. Why are we using an OLED screen? Because the ESP32C3 as supplied seems to use the I2C lines for serial i/o. That's then a problem in that it's not possible to get trace via Serial.print() etc in the Arduino IDE and still keep the I2C sensor(s) running. So adding an I2C OLED screen solves this problem. But the screen draws 2-8mA all the time, depending on what's displaying on the screen, and it keeps drawing in deep sleep. So it's convenient to provide for a "pluggable" OLED screen that can be plugged in for feedback, and unplugged again for practical use. The code decides whether there is a screen present and adapts accordingly.

With all these factors taken into account, it should be possible to reduce deep sleep current to around 40uA.

*The Code*

