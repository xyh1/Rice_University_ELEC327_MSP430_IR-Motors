Rice University ELEC327 MSP430 IR Motor Control
===============================================

This is a presentation and tutorial on utilizing an MSP430 G2553 with an IR remote to control motors.

Quick overview: 
----------------
A 21 button IR Remote sends signals using an NEC IR signal protocol. An IR detector connected to an MSP430G2553 reads and decodes the signal.
The MSP430 interprets the signal and sends the corresponding motor control signal through the motor driver. It uses PWM to drive a left and right motor such that the vehicle is able to turn left and right or move forward and backward.

Direct link to project page: http://xyh1.github.io/Rice_University_ELEC327_MSP430_IR-Motors/
