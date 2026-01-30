# esp32-power-test

Simple code to test power consumption in various modes on ESP32 boards

(c) 2026 John Mueller / MIT License

Runs on Arduino (but is generic C code and is easy to port)

Tested with boards using ESP32 C6, ESP32 S3, ESP32.

## Usage

1. Pick a test mode (see code comments)
2. Compile & upload
3. Remove USB, connect battery pins to measurement device
4. Turn on, potentially wait, take measurements
5. Repeat as desired

Very exciting, much wow. Yes, it does very little. Most of the code is also very basic, so that it mirrors the code that might be used for your application.

Requires a power measurement device like the [Nordic Power Profiler II](https://www.nordicsemi.com/Products/Development-hardware/Power-Profiler-Kit-2) (very nice, love it). 

Since measuring power consumption is generally sign that you're using battery power, make sure to power the board through the battery pins (or with the LDO that you plan to use).

If the board has an on-board RGB LED, keep in mind that this will suck 300mA'ish even when turned off. Remove it, if you want to reduce power.

## Test modes

(This is taken from the comments in the code.)

  1  = empty loop

  2  = delay 1s loop (does delay affect power?)

  3  = empty loop with SIG enabled as output, not set

  4  = empty loop with SIG enabled and set HIGH

  5  = empty loop with SIG enabled and set LOW

  6  = delay 1s loop, alternating SIG (does pin toggling affect power?)

  7  = delay 1s loop, enable Serial 9600, print "." (does Serial affect power?)

  17 = wait 3s, power down all pins, delay 1s loop (do pins affect power?)

  16 = wait 3s, 160MHz CPU, delay 1s loop

  8  = wait 3s, 80MHz CPU, delay 1s loop.

  9  = wait 3s, 40MHz CPU, delay 1s loop.

  10 = wait 3s, 20MHz CPU, delay 1s loop.

  11 = wait 3s, 10MHz CPU, delay 1s loop.

  12 = wait 3s, light sleep for 15s, empty loop

  13 = wait 3s, deep sleep for 15s, empty loop

  14 = wait 3s, disable stuff, deep sleep for 15s, empty loop

  15 = wait 3s, 10MHz, pause, deep sleep for 15s, empty loop (does speed affect deep sleep power?)

  18 = wait 3s, disable stuff, power down pins, deep sleep for 15s, empty loop

Many of these test modes include a 3 second delay before they switch on. If you do not do this, your board will be hard to reprogram. For example, turning on deep sleep right at start means your USB port will disappear. To reprogram boards in a funky state like this, hold BOOT, click RESET, release BOOT buttons.

## Anecdotes from testing

From trying out a few boards. Add your anecdotes if you want.

* delay() uses slightly less power than an empty loop (duh, I guess)
* toggling 1 output pin or using the serial power doesn't visibly change consumption
* however, powering down all pins (set to INPUT_PULLDOWN) will get you ca 10% when the device isn't in deep sleep
* doing magic dances before deep sleep does nothing (it probably disables these things automatically)
* reducing CPU frequency directly affects consumption while running (not in deep sleep)
* that RGB LED sucks a lot of power, even when off.
* some boards have a relatively high deep sleep consumption, probably because of the LDO. If you don't need very long battery life, maybe that's fine. 

## Questions / comments / feedback?

Use the repo please. 
