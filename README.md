# Module 6 - Interrupts

## Objective

This module demonstrates basic interrupt handling on Arduino Uno.

The tasks cover:

- External interrupts
- Interrupt Service Routine (ISR)
- volatile variables
- ISR and main program interaction
- Interrupt counting
- Critical section

---

# Task 1 - External Interrupt + ISR

## Objective

To detect a button press using an external interrupt.

## Working

The push button is connected to Arduino pin D2.

When the button is pressed:

1. An external interrupt is generated.
2. The ISR executes.
3. The ISR sets an interrupt flag.
4. The main loop detects the flag.
5. The LCD displays the interrupt event.

## Connection

- LCD RS → D8
- LCD EN → D9
- LCD D4 → D4
- LCD D5 → D5
- LCD D6 → D6
- LCD D7 → D7
- Push button → D2
- Push button other side → GND

## Output

Initial output:

MODULE 6
INTERRUPT READY

After pressing the button:

INTERRUPT!
ISR EVENT

## Result

The external interrupt and ISR were successfully tested.

---

# Task 2 - Interrupt Counter

## Objective

To count interrupt events using an ISR.

## Working

Every time an interrupt occurs, the ISR increments the interrupt counter.

The main program safely reads the counter and displays it on the LCD.

## Output

Initial:

ISR COUNTER
Count: 0

After interrupt:

ISR COUNTER
Count: 1

Further interrupts:

ISR COUNTER
Count: 2

ISR COUNTER
Count: 3

## Important Note

A mechanical push button can produce multiple electrical transitions during one press. This is called switch bouncing.

Therefore, one physical button press can sometimes increase the counter by more than one.

## Concepts Demonstrated

- External interrupt
- ISR
- volatile variable
- Shared data
- Critical section
- Interrupt counting
