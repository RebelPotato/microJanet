(def to-run 
  (fn [{:digital-write dw
        :delay delay
        :LED-BUILTIN LED-BUILTIN}]
    (dw LED-BUILTIN 1)
    (delay 1000)
    (dw LED-BUILTIN 0)
    (delay 1000)))