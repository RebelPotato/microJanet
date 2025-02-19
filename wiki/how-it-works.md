# How does MicroJanet work? 🚧

MicroJanet is incomplete for now, so unfortunately this document cannot be written yet.
Here are some key things that may make it possible, however:

## Janet lang

* esp32 implements [newlib](https://sourceware.org/newlib/), a small part of the C stdlib
  that happens to compile janet (with lots of parts removed though)
* esp32 has 8 Megabytes of RAM

## Uploading programs

* Janet can marshal (pickle) a function (along with its closure) into bytes and unmarshal it back,
  * as long as it does not contain weird stuff like file handles
* esp32 can do wifi and bluetooth

## Concurrency

See ["Concurrency in MicroJanet"](concurrency.md).

tl;dr: Janet has [fibers](https://janet-lang.org/docs/fibers/index.html) (coroutines), esp32 runs freeRTOS.
