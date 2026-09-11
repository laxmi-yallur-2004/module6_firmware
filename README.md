# MODULE 6 – INTERRUPTS

## Objective

Learn how to use **external interrupts** with Arduino Uno.

In this module, we use a button connected to **D2** to generate an interrupt.

---

## Hardware

* Arduino Uno
* 16x2 LCD
* Push button
* USB cable

### LCD Connections

| LCD | Arduino |
| --- | ------- |
| RS  | D8      |
| EN  | D9      |
| D4  | D4      |
| D5  | D5      |
| D6  | D6      |
| D7  | D7      |

### Button Connection

```text
Button → D2
Button → GND
```

D2 is used because Arduino Uno supports an external interrupt on **D2**.

---

# Task 1 – External Interrupt

## What we do

When the button is pressed, an interrupt occurs.

The ISR (Interrupt Service Routine) sets an event flag.

```text
Button Press
     ↓
External Interrupt
     ↓
ISR
     ↓
interruptFlag = true
     ↓
Main Loop
     ↓
LCD displays interrupt event
```

## Main concepts

* `attachInterrupt()` – enables the external interrupt.
* `ISR` – small function that runs when the interrupt occurs.
* `volatile` – tells the compiler that a variable can change inside an interrupt.
* `FALLING` – interrupt occurs when the signal changes from HIGH to LOW.

## Expected Result

When the button is pressed:

```text
INTERRUPT!
ISR EVENT
```

The LED can also indicate that the interrupt occurred.

---

# Task 2 – Interrupt Counter

## What we do

In Task 2, we count button interrupt events.

The ISR sets an event flag:

```text
Button Press
     ↓
Interrupt
     ↓
ISR
     ↓
interruptEvent = true
     ↓
Main Loop
     ↓
Debounce
     ↓
interruptCount++
     ↓
LCD displays count
```

## Debouncing

A real button can produce multiple electrical changes during one press.

This is called **button bouncing**.

We use a debounce time of **200 ms** to avoid counting one press multiple times.

No `delay()` is used.

---

## Critical Section

The interrupt counter is protected using:

```cpp
noInterrupts();
interruptCount++;
interrupts();
```

This prevents an interrupt from changing the shared variable while it is being accessed.

---

## Expected Output

When the button is pressed:

```text
INTERRUPT!
Count: 1
```

Next press:

```text
INTERRUPT!
Count: 2
```

Next press:

```text
INTERRUPT!
Count: 3
```

The count increases for every valid button press.

---

## Important Concepts Learned

### Interrupt

An interrupt allows the Arduino to immediately respond to an event without continuously checking the button.

### ISR

**ISR = Interrupt Service Routine**

It is the function that runs when an interrupt occurs.

### volatile

`volatile` is used for variables shared between the ISR and the main program.

### Non-blocking

The program does not use `delay()` for button handling. The main loop continues running normally.

---

## Conclusion

Module 6 demonstrates:

* External interrupts
* ISR
* `volatile` variables
* Button debounce
* Interrupt event handling
* Interrupt counter
* Critical sections
* Non-blocking programming

The Arduino Uno detects the button press using the **D2 external interrupt** and processes the event in the main program.
