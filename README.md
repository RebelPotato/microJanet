# microJanet

[The Janet programming language](https://github.com/janet-lang/janet) on an esp32 board.

It runs a slightly modified Janet (no event loop, some modifications to source) on the [Seeed Studio XIAO ESP32S3 Sense](https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/) development board.
Capabilities of the platform are provided as janet functions.

## Status & Roadmap

`src/main.cpp` runs a Janet program that blinks the esp32's built-in LED. It is compiled on a host computer (see `host/make-fun.janet`), so it cannot access any data on the esp32, except for those passed as parameters. Safety through [capabilities](https://en.wikipedia.org/wiki/Object-capability_model)!

For future goals, see [roadmap.md](roadmap.md).

## Why?

Developing on an esp32 should be more interactive. I want to say: "run this function on an esp32" and see the result 2 seconds later, not after half a minute.

This project is the first step towards such an environment.
