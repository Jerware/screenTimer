# ScreenTimer
Internet-connected timer for "screen time"

This timer will send events to the cloud when it is paused, reset, or finished. Settings & controls are accessed via phone app or 
secure web. You can tap into events using [IFTT](https://ifttt.com/discover) to send emails, text messages, turn on/off lights, or 
a myriad of other actions. You can also have the ScreenTimer itself control the power supply to electronincs like lights or, 
more deviously, game consoles, televisions, or monitors.

Bill of materials:

* [3D Printed Body](https://www.thingiverse.com/thing:3419817)
* [PCB from OshPark](https://oshpark.com/shared_projects/3eUUt0Xh)
* [Particle Photon](https://www.amazon.com/Particle-Reprogrammable-Development-Prototyping-Electronics/dp/B016YNU1A0/)
* [Adafruit 7-Segment Display w/Backpack](https://www.amazon.com/Adafruit-4-Digit-7-Segment-Display-Backpack/dp/B00SLYARJQ/)
* [Piezo Buzzer](https://www.amazon.com/BETAFPV-Terminals-Electronic-Continuous-12X9-5mm/dp/B073RH8TQK/)
* [Push Button](https://www.amazon.com/Adafruit-Colorful-Tactile-Button-Assortment/dp/B00XW2KD82/)

The timer settings are all configurable using the official Particle app or the Particle [web console](https://console.particle.io/).
Just select your device and see the FUNCTIONS listed (under the Data tab in the app). You can type in the values you wish to send
and then execute the functions. They should take effect immediately.

To control power to an AC device, you can use a product like one of the following.

* [IOT Relay](https://www.amazon.com/Iot-Relay-Enclosed-High-Power-Raspberry/dp/B00WV7GMA2/)
* [PowerSwitch Tail II](https://www.amazon.com/POWERSWITCHTAIL-COM-PowerSwitch-Tail-II/dp/B00B888VHM/)

Using wires, connect ground and signal to the device's microcontroller input terminals, and solder the other end to the pads on
the back of the ScreenTimer PCB. You'll see 8 pads -- the ones next to the Photon D2-D5 pins are the signal pins. The other four are all
ground. The effects of each pin are as follows.

* D2: Normally HIGH, LOW when timer paused or expired
* D3: Normally HIGH, LOW when timer expired
* D4: Normally LOW, HIGH when timer paused or expired
* D5: Normally LOW, HIGH when timer expired
