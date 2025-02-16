/*
runs a Janet program that blinks the esp32's built-in LED.
It is compiled on a host computer (see `host/make-fun.janet`),
so it cannot access any data on the esp32, except for those
passed as parameters. Safety through 
[capabilities](https://en.wikipedia.org/wiki/Object-capability_model)!
*/

#include <Arduino.h>
#include <janet.h>
#include <setjmp.h>

// tiny library for janet-esp32 interop

static Janet cfun_wait(int32_t argc, Janet *argv) {
  janet_fixarity(argc, 1);

  uint32_t time = janet_getinteger(argv, 0);
  delay(time);

  return janet_wrap_nil();
}

static Janet cfun_digital_write(int32_t argc, Janet *argv) {
  janet_fixarity(argc, 2);

  uint16_t pin = janet_getuinteger16(argv, 0);
  if (pin > UINT8_MAX) {
    janet_panicf("pin number must be between 0 and %d", UINT8_MAX);
  }
  uint16_t value = janet_getuinteger16(argv, 1);
  if (value != HIGH && value != LOW) {
    janet_panicf("value must be either HIGH or LOW");
  }
  digitalWrite(pin, value);

  return janet_wrap_nil();
}

bool should_rerun;
Janet State;

static Janet cfun_rerun(int32_t argc, Janet *argv) {
  janet_fixarity(argc, 2);

  uint32_t time = janet_getinteger(argv, 0);
  janet_gcunroot(State);
  State = argv[1];
  janet_gcroot(State);

  should_rerun = true;
  delay(time);
  return janet_wrap_nil();
}

// TODO: name according to Arduino conventions
static const JanetReg cfuns[] = {
    {"wait", cfun_wait,
     "(wait ms)\n\n"
     "Stops the world for a given amount of milliseconds."},
    {"digital-write", cfun_digital_write,
     "(digital-write pin value)\n\n"
     "void digitalWrite(uint8_t pin, uint8_t val) in Arduino.\n"
     "Writes a value to a digital pin."},
    {"rerun", cfun_rerun,
     "(rerun ms state)\n\n"
     "Reruns the lambda function after a given amount of milliseconds."},
    {NULL, NULL, NULL}};

void log_free() {
  log_d("free MALLOC_CAP_8BIT heap: %d",
        heap_caps_get_free_size(MALLOC_CAP_8BIT));
}

// compiled janet code for blink from client/make-fun.janet
static const unsigned char bytes[] = {
    0xD7, 0x01, 0xCD, 0x00, 0xD4, 0x00, 0x00, 0x08, 0x02, 0x02, 0x02, 0x05,
    0x16, 0x01, 0x07, 0xCE, 0x10, 0x2E, 0x5C, 0x6D, 0x61, 0x6B, 0x65, 0x2D,
    0x66, 0x75, 0x6E, 0x2E, 0x6A, 0x61, 0x6E, 0x65, 0x74, 0xD0, 0x04, 0x77,
    0x61, 0x69, 0x74, 0xD0, 0x05, 0x72, 0x65, 0x72, 0x75, 0x6E, 0xD0, 0x0D,
    0x64, 0x69, 0x67, 0x69, 0x74, 0x61, 0x6C, 0x2D, 0x77, 0x72, 0x69, 0x74,
    0x65, 0xCE, 0x10, 0x53, 0x65, 0x74, 0x20, 0x70, 0x69, 0x6E, 0x20, 0x25,
    0x64, 0x20, 0x74, 0x6F, 0x20, 0x25, 0x64, 0xD8, 0x06, 0x70, 0x72, 0x69,
    0x6E, 0x74, 0x66, 0xBF, 0xFF, 0x00, 0x00, 0xCF, 0x03, 0x70, 0x69, 0x6E,
    0xBF, 0xFF, 0x00, 0x01, 0xCF, 0x05, 0x65, 0x76, 0x65, 0x72, 0x79, 0xBF,
    0xFF, 0x00, 0x02, 0xCF, 0x05, 0x62, 0x6C, 0x69, 0x6E, 0x6B, 0x00, 0x16,
    0x01, 0xCF, 0x05, 0x73, 0x74, 0x61, 0x74, 0x65, 0x02, 0x16, 0x03, 0xCF,
    0x04, 0x77, 0x61, 0x69, 0x74, 0x05, 0x16, 0x04, 0xCF, 0x05, 0x72, 0x65,
    0x72, 0x75, 0x6E, 0x08, 0x16, 0x05, 0xCF, 0x02, 0x64, 0x77, 0x2C, 0x03,
    0x00, 0x00, 0x3A, 0x02, 0x00, 0x03, 0x1B, 0x03, 0x02, 0x00, 0x2C, 0x04,
    0x01, 0x00, 0x3A, 0x02, 0x00, 0x04, 0x1B, 0x04, 0x02, 0x00, 0x2C, 0x05,
    0x02, 0x00, 0x3A, 0x02, 0x00, 0x05, 0x1B, 0x05, 0x02, 0x00, 0x2D, 0x00,
    0x00, 0x00, 0x32, 0x00, 0x01, 0x00, 0x35, 0x00, 0x05, 0x00, 0x2C, 0x02,
    0x03, 0x00, 0x2D, 0x06, 0x00, 0x00, 0x33, 0x02, 0x06, 0x01, 0x2C, 0x06,
    0x04, 0x00, 0x35, 0x02, 0x06, 0x00, 0x2B, 0x07, 0x01, 0x00, 0x08, 0x06,
    0x07, 0x01, 0x2D, 0x07, 0x00, 0x01, 0x32, 0x07, 0x06, 0x00, 0x36, 0x04,
    0x00, 0x00, 0xBF, 0xFF, 0x04, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03,
    0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x03, 0x05,
    0x00, 0x05, 0x00, 0x05, 0x01, 0x05, 0x00, 0x05, 0x00, 0x05, 0x00, 0x05,
    0x00, 0x05, 0x01, 0x12, 0x00, 0x12, 0x00, 0x05, 0x00, 0x05, 0x00, 0x05,
    0x00, 0x04, 0x15, 0x83, 0xE8, 0xC9, 0xC9};

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  // if the initialization runs out of memory, the esp32 reboots instantly
  // and it becomes very hard to upload a new sketch
  // so we wait for a while here
  delay(5000);

  // checking before initialization
  if (!psramInit()) {
    log_e("\nPSRAM is not correctly initialized.");
    exit(1);
  }
  log_d("total heap: %d", ESP.getHeapSize());
  log_d("total PSRAM: %d", ESP.getPsramSize());
  if (setjmp(oom) != 0) {
    // out of memory error
    log_free();
    exit(1);
  }

  // initialize janet
  janet_init();
  JanetTable *coreEnv = janet_core_env(NULL);
  int handle = janet_gclock();

  // environment
  JanetTable *caps = janet_table(sizeof(cfuns));
  janet_gcroot(janet_wrap_table(caps));
  for (const JanetReg *fn = cfuns; fn->name; fn++) {
    Janet fun = janet_wrap_cfunction(fn->cfun);
    janet_table_put(caps, janet_ckeywordv(fn->name), fun);
  }
  // janet_def(caps, "LED-BUILTIN", janet_wrap_integer(LED_BUILTIN), NULL);

  // function to run
  JanetTable *lookup = janet_env_lookup(coreEnv);
  const unsigned char *image = bytes;
  size_t image_size = sizeof(bytes);
  Janet marsh_out = janet_unmarshal(image, image_size, 0, lookup, NULL);
  if (!janet_checktype(marsh_out, JANET_FUNCTION)) {
    log_e("invalid bytecode image - expected function.");
    exit(1);
  }
  janet_gcroot(marsh_out);
  JanetFunction *func = janet_unwrap_function(marsh_out);

  // state
  should_rerun = true;
  State = janet_wrap_integer(0);  //will receive this from host
  janet_gcroot(State);

  janet_gcunlock(handle);

  // run the function
  while (should_rerun) {
    should_rerun = false;
    const Janet argv[2] = {janet_wrap_table(caps), State};
    Janet result; 
    JanetSignal sig = janet_pcall(func, 2, argv, &result, NULL);
    if (sig != JANET_SIGNAL_OK) {
      // todo: print janet stacktrace
      log_e("got signal %d when running function", sig);
      exit(1);
    }
  }
}

void loop() {}