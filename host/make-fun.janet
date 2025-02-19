(import "./lib/base64/btoa" :as base64)

(def ip "http://192.168.137.81")
(def LED-BUILTIN 21)

(defn blink [pin every]
  (fn [{:digital-write dw
        :rerun rerun} state]
    (dw pin state)
    (printf "Set pin %d to %d" pin state)
    (rerun every (- 1 state))))

(defn to-cbytes [bytes]
  (string "{" (string/join
                (map |(string/format "0x%s%X" (if (> $ 0xf) "" "0") $) bytes)
                ", ") "}"))

(defn to-base64 [obj]
  (-> obj (make-image) (string) (base64/btoa)))

(defn request [method ip body]
  "shell out to curl and make a request with plain text body"
  (os/execute @[`curl` `-H` `Content-Type: text/plain` `-d` body `-X` method ip]
              :px))

(defn init [{:digital-write dw
            :delay delay
            :pinmode pinmode}]
  (pinmode LED-BUILTIN :output)
  (delay 1000)
  1)
(def task (blink LED-BUILTIN 1000))

(defn main [&]
  (pp (disasm init))
  (pp (disasm task))
  (def body (string (to-base64 init) "  " (to-base64 task)))
  (pp body))
