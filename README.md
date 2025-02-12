# microJanet

[The Janet programming language](https://github.com/janet-lang/janet) on the esp32.

It uses a slightly modified Janet (no event loop, some modifications to source).

## Status & Roadmap

`src/main.cpp` runs a Janet program that blinks the builtin LED. (Or more precisely, a C program that prepares a janet environment with bindings to Arduino/esp32 functions, and then runs in it a Janet program that blinks the builtin LED.)

For future goals, see [roadmap.md](roadmap.md).

## Why?

Developing on an esp32 should be more interactive. I want to control it from a janet repl on another computer. I want to just say: "run this function on an esp32" and see the result 2 seconds later, not 20 seconds. This project is the first step towards such an environment.
