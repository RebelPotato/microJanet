# Simple experiment
# "compile" marshals (pickles) a function with a closure to a file
# "read" reads it back, executes and disassembles it

(defn main [_ step &]
  (case step
    "compile"
    (do
      (def x @{:value 1 :other 2})
      (def y @{:value 3 :other 4})
      (defn f [z] (+ (get x :value) z))
      (pp f)
      (def bytes (marshal f))
      (pp bytes)
      (spit "./f_bytes" bytes))
    "read"
    (do
      (def g (unmarshal (slurp "./f_bytes")))
      (pp g)
      (pp (g 3))
      (pp (disasm g)))
    (error (string "unknown step \"" step "\", expected compile or read"))))
