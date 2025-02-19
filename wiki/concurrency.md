# Concurrency in MicroJanet 🚧

This explaination expects prior knowledge about basic concurrency and multitasking primitives.

MicroJanet runs actors concurrently. This capability is built on top of Janet fibers and esp32's FreeRTOS.

## Cooperative Multitasking

In Janet, all programs run in fibers (coroutines). Fibers allow a program to stop and resume execution later, essentially enabling multiple returns from a function.

In MicroJanet, each actor runs in its own fiber. It may pause itself at any time by "yielding to the event loop", typically by calling a builtin function.

In the blink example:

``` clojure
(defn blink [pin every]
  (fn [{:digital-write dw
        :sleep sleep
        :rerun rerun}]
    (dw pin 1)
    (sleep every)
    (dw pin 0)
    (rerun every)))

(def blink1 (blink LED1 500))
(def blink2 (blink LED2 1000))
```

The functions `sleep` and `rerun` yield to the event loop. `sleep` pauses the current actor and schedules it to continue after some given time. `rerun` ends the actor and schedules it to rerun later.

The Janet virtual machine is single-threaded, much like Javascript. So if a actor does not yield, it is guaranteed to run from top to bottom, with no other actors running in between (in the absence of system events). Actors are scheduled by the actors themselves in a sense.

## The Event Loop

The event loop is delegated to FreeRTOS. Each actor corresponds with a fiber and a FreeRTOS task. Each task has a unique id called its "pid".

A global semaphore controls the execution. After taking it, a task runs its fiber until one of the following things happen:

* The fiber finishes. Its actor has ended its life, and the task returns.
* The fiber yields. The task pauses and another actor starts running.
* The fiber yields to the event loop. Rescheduling happens, and its actor may sleep, rerun or do something else.
* The fiber emits an error signal. Its actor ends and the task returns.

An actor can be halted: if its task is not running then end the task, otherwise interrupt the interpreter and the task will return (the fiber will emit `JANET_SIGNAL_INTERRUPT`).

## System Events

I don't know much about esp32's event system yet. Surely this will not be too big a problem ...
