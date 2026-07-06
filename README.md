# Single Dosing Tool (SDT)

A system to automatically dispense a single dose of coffee beans into the grinder. Used for brewing espresso.


## System Description


### Overview

The SDT is a tool that is mounted on top of a coffee grinder and replaces the bean hopper.  
It weighs the beans stored in the tool's chamber and can dispense the beans more or less one by one.
Using these together, it dispenses a given amount of beans (usually 16 grams) directly into the mill.

Compared to a normal hopper, the beans are dispensed over a longer time and one by one, which gives a better grind.

Compared to a normal time-based control, the SDT gives a better consistency, and you can directly control the important weight parameter.


### Dispense mechanism

There are two identical dispensers!

The dispense mechanism is a wheel mounted on a stepper motor (5V 28BYJ-48 with ULN2003  https://www.amazon.de/dp/B0F6T5Q7FT). 
The dispenser is mounted in a 60° angle.
The wheel has 10 holes, each the size that one big or two small coffee beans can be held by it.
By turning the wheel, it lifts coffee beans out of the storage and releases them into a tube on the backside of the wheel. 
The beans are released on the uppermost position (12 o'clock) of the wheel. 
The beans are retained in the hole by the 60° tilt and gravity.
The wheel is mounted directly to a tilted wall, so the beans are only able to leave in one position.
The wheel is 12cm in diameter and 5mm thick.
The tube is 13 cm long and slightly bent to give room for the motor.

The motors turn with 10-15 rpm.

The two dispensers are mounted opposite of each other.


### Weighing mechanism

For weighing, a 1kg load cell is used. The load cell is connected to the Arduino using a HX711 board (https://www.amazon.de/dp/B0FQC6RK3W?th=1).
The load cell is mounted on the underside of the SDT between the ends of the bean tubes.
The other side of the load cell is mounted on the mill.

There is a small gap between the mill and the SDT; the load cell is the only physical connection.

The beans are released at the end of the pipe at the same height as the load cell. After that they don't 

The system is a Loss-in-Weight (LiW). The Arduino measures the weight decrease of the SDT and determines how much is dispensed. 



### Electronics

5V external supply
Arduino Nano
0.91 inch OLED LCD Display Module Mini IIC 128 × 32 https://www.amazon.de/dp/B0CT2QP43S
2x 28BYJ-48 with ULN2003

https://app.cirkitdesigner.com/project/4007499b-9b01-44e2-a004-56db9d34cd1c

### 3d Model and printing

The system is modeled using Onshape and then 3D printed.
Model link: https://cad.onshape.com/documents/0dff8796480512d936f44321/w/78978a18521bd89a6884aa3c/e/6b9b33d8fd7be016eea2bcd9?renderMode=0&uiState=6a44ede6a47f8baaa3505c5f

## Open Problems

How can we filter out vibrations from the mill the best way? 
How can we model the system to predict the weight of the dispensed beans most accurately?
Right now we use a moving windows and a linear regression. Maybe use a Kalman Filter or something like that?

Can we make sure that the wheel stops between two holes, so that in the resting position there is no extra air coming in to reduce oxidation? Do we need a wheel sensor or can we use a smart algorithm to locate the holes using the load cell?

We need to run cables between the mill and the Arduino and the motors, so the cables will be a connection between the two parts. How can we make sure that this does not interfere with our measurements? Probably if we run the cables directly on the load cell there should be no changing interference, but maybe electrostatic problems?

How can we control the mill's motor the best (Bezzera BB005)?