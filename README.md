# weatherStation-ESP32C3
This is the latest iteration of my solar-powered weather station project, updated to use the ESP32C3. The original version used a Wemos D1 Mini (ESP8266), powered by four AA rechargable NiCad batteries which were recharged by a small (60x110mm) 6V (nominal) solar panel. Whilst this works well in the summer months, the challenge has always been getting it to survive the winter months.

The project has been a valuable learning exercise - on the back of what I've learnt from this project, we've installed solar panels on our roof at home and have been very pleased with the results so far. There are several components to achieving the aim of getting a twelve month operational span purely powered by the sun:

* The solar panel; and maximising the charge obtained from the panel.
* The storage battery. This also needs to be protected from overcharge and over-discharge.
* The electronics. Deep sleep between readings is important, but minimising the current drain during deep sleep is crucial.

*The Solar Panel*

I've stuck with the original 60x110mm 6V nominal solar panel throughout the project. One easy way to achieve the aim of lasting throughout the winter months _might_ be to double the size (and capacity) of the solar panel, but (a) that feels like cheating, (b) it will stop me learning about other aspects of the project, and (c) there's every likelihood that such an approach won't actually work in practice. But recently, I've come across a CN3065-based MPPT charger for these panels, and first impressions are that it makes quite a difference as opposed to simply connecting the panel to the battery via a diode. The solar panel and the MPPT charge controller are available as a package from AliExpress. I can't find that exact package, but the board can be found here:

https://www.aliexpress.com/item/1005005898470290.html?spm=a2g0o.detail.0.0.71e63b32zJkK80&gps-id=pcDetailTopMoreOtherSeller&scm=1007.40000.327270.0&scm_id=1007.40000.327270.0&scm-url=1007.40000.327270.0&pvid=d029ae44-1ee1-4d7f-909f-4667bae6bec1&_t=gps-id:pcDetailTopMoreOtherSeller,scm-url:1007.40000.327270.0,pvid:d029ae44-1ee1-4d7f-909f-4667bae6bec1,tpp_buckets:668%232846%238115%232000&pdp_npi=4%40dis%21GBP%210.78%210.69%21%21%216.77%21%21%402103872a16998120740861996eb301%2112000034760558665%21rec%21UK%21%21AB&search_p4p_id=202311121001141305433920249196579209_1

No, I don't get any commission for giving that link ;)

*The Storage Battery*

I started by using four 1.5V nominal rechargable AA NiCad batteries, since these give a decent working voltage for 5V circuits. However, experience has taught me that LiPos are a much better bet for longevity, and the 18650 in particular sems to be a very good battery for working with. Although this has a lower working voltage of around 3.7V, this is fine for working with modern 3.3V circuitry. In addition, using an MPPT charge controller (see previous section) does a very good job of using the solar panel to keep the battery as efficiently maintained as possible. A further step forward is to include a DW01A-based board to protect the battery from deep discharge, maintaining its life significantly.

AliExpress: https://www.aliexpress.com/item/1005004332269949.html?spm=a2g0o.productlist.main.9.45b540f1o5WnxO&algo_pvid=48d111f5-56fc-4b1b-9fd0-c104b14eeb62&aem_p4p_detail=2023111209535211598463353544510001769969&algo_exp_id=48d111f5-56fc-4b1b-9fd0-c104b14eeb62-4&pdp_npi=4%40dis%21GBP%213.51%210.41%21%21%214.17%21%21%402103846916998116328994210e381f%2112000029555291294%21sea%21UK%210%21AB&curPageLogUid=RwOJRZhx6sLK&search_p4p_id=2023111209535211598463353544510001769969_5
