(def LED-BUILTIN 21)

(defn blink [pin every]
  (fn [{:digital-write dw
        :wait wait
        :rerun rerun} state]
    (dw pin state)
    (printf "Set pin %d to %d" pin state)
    (rerun every (- 1 state))))

(defn to-cbytes [bytes]
  (string "{" (string/join
                (map |(string/format "0x%s%X" (if (> $ 0xf) "" "0") $) bytes)
                ", ") "}"))

(defn main [&]
  (def task (blink LED-BUILTIN 1000))
  (pp (disasm task))
  (print (to-cbytes (make-image task))))
