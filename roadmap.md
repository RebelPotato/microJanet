# Roadmap

## in progress

* [ ] start and stop one task through serial
* [ ] start and stop multiple tasks through serial
* [ ] start and stop multiple tasks through http

## completed

in reverse chronological order:

* [x] run on the esp32 a lambda function compiled on the laptop
* [x] Janet repl through serial
* [x] control builtin LED with Janet
* [x] Janet runs on an esp32

## ideas

* local Janet communication with esp32 Janet through wifi
* declarative deployment: "run this function on this node"
* capabilities: esp32 caps passed as arguments
  * the distributed part of [Run, Build, and Grow Small Systems Without Leaving Your Text Editor](https://albertzak.com/run-build-grow/)
* an tiny event loop using freeRTOS (should be easier than writing from scratch)
