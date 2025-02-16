/*
runs a server on the esp32 that can load Janet programs from the host
half done: I'm trying to figure out how to start and stop tasks
*/

#include <Arduino.h>
#include <WebServer.h>
#include <b64.h>
#include <esp_wifi.h>
#include <janet.h>
#include <mjlib.h>
#include <secrets.h>
#include <setjmp.h>
// #include <esp_http_server.h>

static Janet state;

JANET_NO_RETURN static Janet cfun_rerun(int32_t argc, Janet *argv) {
  janet_fixarity(argc, 2);

  uint32_t time = janet_getinteger(argv, 0);
  janet_gcunroot(state);
  state = argv[1];
  janet_gcroot(state);

  delay(time);
  janet_signalv(JANET_SIGNAL_USER9, janet_wrap_nil());
}

void log_free() {
  log_i("free esp heap: %d", esp_get_free_heap_size());
  log_i("free MALLOC_CAP_8BIT heap: %d",
        heap_caps_get_free_size(MALLOC_CAP_8BIT));
  log_i("free arduino heap: %d", ESP.getFreeHeap());
  log_i("free arduino Psram: %d", ESP.getFreePsram());
}

static JanetFunction *currentTask;
static JanetTable *caps;
void run_task(Janet init, JanetFunction *task) { log_i("i'm running i guess"); }

void cancel_task() {
  log_i("canceled!");
  currentTask = NULL;
}

static WebServer server(80);
void handle_not_found() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void handle_root() {
  const HTTPMethod method = server.method();
  if (method == HTTP_GET) {
    server.send(200, "text/plain",
                "** MicroJanet **\n"
                "\n"
                "POST / to run task\n"
                "DELETE / to cancel task\n");
  } else if (method == HTTP_POST) {
    log_i("Got task.");
    String body = server.arg("plain");
    int index = body.indexOf(' ');
    String init_str = body.substring(0, index);
    String task_str = body.substring(index + 2);
    log_i("Init: %s", init_str.c_str());
    log_i("Task: %s", task_str.c_str());
    const uint8_t *init_image = b64_decode(init_str.c_str(), init_str.length());
    const uint8_t *task_image = b64_decode(task_str.c_str(), task_str.length());
    log_free();
    JanetTable *env = janet_core_env(NULL);
    JanetTable *lookup = janet_env_lookup(env);
    log_free();
    Janet init =
        janet_unmarshal(init_image, init_str.length(), 0, lookup, NULL);
    Janet task =
        janet_unmarshal(task_image, task_str.length(), 0, lookup, NULL);
    log_free();
    if (!janet_checktype(task, JANET_FUNCTION)) {
      server.send(400, "text/plain", "Task is not a function");
      return;
    }
    server.send(200, "text/plain", "Done");
    run_task(init, janet_unwrap_function(task));
  } else if (method == HTTP_DELETE) {
    log_i("Cancelled task.");
    server.send(200, "text/plain", "Done");
  } else {
    handle_not_found();
  }
}

static EventGroupHandle_t s_wifi_event_group;

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

static int s_retry_num = 0;
const int max_retry = 5;

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data) {
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
    esp_wifi_connect();
  } else if (event_base == WIFI_EVENT &&
             event_id == WIFI_EVENT_STA_DISCONNECTED) {
    if (s_retry_num < max_retry) {
      esp_wifi_connect();
      s_retry_num++;
      log_i("retry [%d/%d]", s_retry_num, max_retry);
    } else {
      xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
    }
    log_e("Failed to connect with wifi AP \"%s\".", ssid);
  } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    log_i("Connected. IP address: " IPSTR, IP2STR(&event->ip_info.ip));
    s_retry_num = 0;
    xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
  }
}

void wifiInit() {
  s_wifi_event_group = xEventGroupCreate();
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  ESP_ERROR_CHECK(esp_wifi_set_default_wifi_sta_handlers());
  esp_netif_t *wifi_interface = esp_netif_create_default_wifi_sta();
  esp_event_handler_instance_t instance_any_id;
  esp_event_handler_instance_t instance_got_ip;
  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL,
      &instance_any_id));
  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL,
      &instance_got_ip));

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
  log_i("Connecting to wifi %s.", ssid);
  wifi_config_t wifi_config = {};
  strncpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
  strncpy((char *)wifi_config.sta.password, password,
          sizeof(wifi_config.sta.password));

  ESP_ERROR_CHECK(
      esp_wifi_set_ps(WIFI_PS_NONE));  // default is WIFI_PS_MIN_MODEM
  ESP_ERROR_CHECK(
      esp_wifi_set_storage(WIFI_STORAGE_RAM));  // default is WIFI_STORAGE_FLASH

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());

  EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                         WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                         pdFALSE, pdFALSE, portMAX_DELAY);

  if (bits & WIFI_CONNECTED_BIT) return;
  if (bits & WIFI_FAIL_BIT)
    log_e("Failed to connect with wifi AP \"%s\".", ssid);
  else
    log_e("Unknown error occured when connecting to wifi.");
  exit(1);
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

    // state
    state = janet_wrap_integer(0);  // will receive this from host
    janet_gcroot(state);

    janet_gcunlock(handle);
  }

  wifiInit();

  // httpd_handle_t server = NULL;
  // httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  // config.lru_purge_enable = true;

  // // Start the httpd server
  // log_i("Starting server on port: '%d'", config.server_port);
  // if (httpd_start(&server, &config) == ESP_OK) {
  //     // Set URI handlers
  //     log_i("Registering URI handlers");
  //     httpd_register_uri_handler(server, &hello);
  // }
  server.on("/", handle_root);
  server.onNotFound(handle_not_found);
  server.begin();

  while (1) {
    server.handleClient();
    delay(5);
  }
}

void loop() {}