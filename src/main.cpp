#include <Arduino.h>
#include <b64.h>
#include <janet.h>
#include <mjlib.h>
#include <setjmp.h>

static Janet cfun_rerun(int32_t argc, Janet *argv) {
  janet_fixarity(argc, 2);

  uint32_t time = janet_getinteger(argv, 0);
  Janet state = argv[1];

  delay(time);
  janet_signalv(JANET_SIGNAL_EVENT, state);
}

void log_free() {
  log_i("free esp heap: %d", esp_get_free_heap_size());
  log_i("free MALLOC_CAP_8BIT heap: %d",
        heap_caps_get_free_size(MALLOC_CAP_8BIT));
  log_i("free arduino heap: %d", ESP.getFreeHeap());
  log_i("free arduino Psram: %d", ESP.getFreePsram());
}

static JanetTable *caps;
bool halt = false;  // this should be atomic
void do_actor(void *pvParameters) {
  JanetFunction *actor = ((JanetFunction **)pvParameters)[0];
  Janet initial_value = janet_wrap_nil();
  while (!halt) {
    Janet args[2] = {janet_wrap_table(caps), initial_value};
    Janet result;
    JanetSignal sig = janet_pcall(actor, 2, args, &result, NULL);
    if (sig == JANET_SIGNAL_OK) {
      // actor finished
      break;
    } else if (sig == JANET_SIGNAL_EVENT) {
      // yield to event loop
    } else {
      // actor failed
      log_i("actor failed with signal %d", sig);
      break;
    }
  }
  halt = false;
}

void load_actor(String body) {
  // todo: wait for current iteration to finish
  int index = body.indexOf(' ');
  String init_str = body.substring(0, index);
  String actor_str = body.substring(index + 2);
  log_i("Init: %s", init_str.c_str());
  log_i("Actor: %s", actor_str.c_str());
  const uint8_t *init_image = b64_decode(init_str.c_str(), init_str.length());
  const uint8_t *actor_image = b64_decode(actor_str.c_str(), actor_str.length());
  log_free();
  JanetTable *env = janet_core_env(NULL);
  JanetTable *lookup = janet_env_lookup(env);
  log_free();
  Janet got_init =
      janet_unmarshal(init_image, init_str.length(), 0, lookup, NULL);
  Janet got_actor =
      janet_unmarshal(actor_image, actor_str.length(), 0, lookup, NULL);
  log_free();
  if (!janet_checktype(got_init, JANET_FUNCTION)) {
    log_e("init is not a function");
    return;
  }
  if (!janet_checktype(got_actor, JANET_FUNCTION)) {
    log_e("actor is not a function");
    return;
  }
  JanetFunction *args[2] = {janet_unwrap_function(got_init),
                            janet_unwrap_function(got_actor)};
  xTaskCreate(do_actor, "janet program", 8192, (void *)args, 1, NULL);
}

void halt_actor() {
  janet_interpreter_interrupt(NULL);
  halt = true;
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  // if the initialization runs out of memory, the esp32 reboots instantly
  // and it becomes very hard to upload a new sketch
  // so we wait for a while here
  delay(5000);

  // checking before initialization
  if (!psramInit()) {
    log_e("\nPSRAM is not correctly initialized.");
    exit(1);
  }
  if (setjmp(oom) != 0) {
    // out of memory error
    log_free();
    exit(1);
  }

  // initialize janet
  janet_init();
  {
    const int handle = janet_gclock();

    // parameter list
    caps = janet_table(sizeof(mjlib));
    janet_gcroot(janet_wrap_table(caps));
    for (const JanetReg *fn = mjlib; fn->name; fn++) {
      Janet fun = janet_wrap_cfunction(fn->cfun);
      janet_table_put(caps, janet_ckeywordv(fn->name), fun);
    }
    janet_table_put(caps, janet_ckeywordv("rerun"),
                    janet_wrap_cfunction(cfun_rerun));
    // janet_def(caps, "LED-BUILTIN", janet_wrap_integer(LED_BUILTIN), NULL);

    janet_gcunlock(handle);
  }
}

String acc = "";

void loop() {
  if (Serial.available() > 0) {
    char incomingByte = Serial.read();
    if (incomingByte == '\b') {
      if (acc.length() > 0) {
        acc = acc.substring(0, acc.length() - 1);
        Serial.print("\b \b");
      }
    } else if (incomingByte == '\n') {
      Serial.print("\n");

      if (acc.indexOf("load") == 0) {
        // todo: wait for current actor to finish and load actor
      } else if (acc.indexOf("start") == 0) {
        // todo: start current actor if not running already
      } else if (acc.indexOf("halt") == 0) {
        // todo: stop current actor immediately
      } else {
        Serial.println("Unknown command");
      }

      acc = "";
      Serial.print("> ");
    } else {
      acc += incomingByte;
      Serial.print(incomingByte);
    }
  }
}