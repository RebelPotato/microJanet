(defn chunk-string [str]
  (var unread true)
  (fn [buf _]
    (when unread
      (set unread false)
      (buffer/blit buf str))))

(defn capture-stderr [f & args]
  (def buf @"")
  (with-dyns [*err* buf *err-color* false]
    (f ;args))
  (string/slice buf 0 -2))

(defn evaluate [user-script]
  (def env (make-env root-env))

  (var err nil)
  (var err-fiber nil)

  (defn on-parse-error [parser where]
    (set err (capture-stderr bad-parse parser where))
    (set (env :exit) true))

  (defn on-compile-error [msg fiber where line col]
    (set err (capture-stderr bad-compile msg nil where line col))
    (set err-fiber fiber)
    (set (env :exit) true))

  (run-context
    {:env env
     :chunks (chunk-string user-script)
     :on-status (fn [fiber value]
                  (printf "> %q (%q)" value (fiber/status fiber)))
     :on-parse-error on-parse-error
     :on-compile-error on-compile-error})

  (if (nil? err)
    env
    (if (nil? err-fiber)
      (error err)
      (propagate err err-fiber))))

(defn main [])