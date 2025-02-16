# Simple experiment
# "compile" makes an image of a function along with its closure to a file (similar to python's pickle)
# "read" reads it back, executes and prints its disassembly

(defn main [_ step &]
  (case step
    "compile"
    (do
      (def x @{:value 1 :other 2})
      (def y @{:value 3 :other 4})
      (defn f [z] (+ (get x :value) z))
      (pp f)
      (def bytes (make-image f))
      (pp bytes)
      (spit "./f_bytes" bytes))
    "read"
    (do
      (def g (load-image (slurp "./f_bytes")))
      (pp g)
      (pp (g 3))
      (pp (disasm g)))
    (error (string "unknown step \"" step "\", expected compile or read"))))
