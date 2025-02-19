(def LED1 2)
(def LED2 3)

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