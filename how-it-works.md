# How does this work?

The project is incomplete for now, so unfortunately this document cannot be written.
Here are some key things that may make this possible, however:

* Janet can marshal (pickle) a function (along with its closure) into bytes and unmarshal it back,
  * as long as it does not contain weird stuff like file handles
* esp32 implements [newlib](https://sourceware.org/newlib/), a small part of the C stdlib
  that happens to compile janet (with lots of parts removed though)
* esp32 runs freeRTOS, so multitasking is a breeze
* esp32 has 8 Megabytes of RAM
* esp32 can do wifi and bluetooth
