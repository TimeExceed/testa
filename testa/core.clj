(ns testa.core
    (:require
        [clojure.string :as str]
        [clojure.java.io :as io]
    )
    (:import
        [java.io PrintWriter]
    )
)

(defmacro ^:private throw-if [pred except & args]
    `(when ~pred
        (throw (~except ~@args))
    )
)

(defmacro ^:private throw-if-not [pred except & args]
    `(throw-if (not ~pred) ~except ~@args)
)


(defn- test-is [expr actual expect]
    (let [expect (expect)
            actual (actual)
        ]
        (throw-if-not (= expect actual)
            AssertionError.
            (apply format "%s expects %s, but %s actually"
                (map pr-str [expr expect actual])
            )
        )
    )
)

(defn- test-eq [expr actual expect & args]
    (let [actual (apply actual args)
            expect (apply expect args)
        ]
        (throw-if-not (= actual expect)
            AssertionError.
            (apply format "apply %s to %s expects %s, but %s actually"
                (str/join ", " (map pr-str args))
                (map pr-str [expr expect actual])
            )
        )
    )
)

(defn- test-throw* [actual args]
    (try
        (apply actual args)
        nil
    (catch Throwable e
        e
    ))
)

(defn- test-throw [expr actual expect & args]
    (let [e (test-throw* actual args)]
        (cond
            (nil? e)
                (throw (AssertionError.
                    (if args
                        (apply format "apply %s to %s expects %s, but nothing happen"
                            (str/join ", " (map pr-str args))
                            (map pr-str [expr expect])
                        )
                        (apply format "%s expects %s, but nothing happen"
                            (map pr-str [expr expect])
                        )
                    )
                ))
            (not (instance? expect e))
                (throw (AssertionError.
                    (if args
                        (apply format "apply %s to %s expects %s"
                            (str/join ", " (map pr-str args))
                            (map pr-str [expr expect])
                        )
                        (apply format "%s expects %s"
                            (map pr-str [expr expect])
                        )
                    )
                    e
                ))
        )
    )
)

(defn- test-throw-nothing [expr actual & args]
    (let [e (test-throw* actual args)]
        (if-not (nil? e)
            (throw (AssertionError.
                (if args
                    (format "apply %s to %s expects no exception"
                        (str/join ", " (map pr-str args))
                        (pr-str expr)
                    )
                    (format "%s expects no exception"
                        (pr-str expr)
                    )
                )
                e
            ))
        )
    )
)

(defn- testbench-for-basic [test]
    (test)
)

(defn- resolve-it [x]
    (if-not (symbol? x)
        x
        (if-let [y (resolve x)]
            y
            x
        )
    )
)

(defn- one-fact [testbench fact]
    (let [[_ cs expr rel expect] fact]
        (case rel
            :is
                (do
                    (throw-if (fn? expr)
                        IllegalArgumentException.
                        ":is requires no fn before it"
                    )
                    (throw-if (fn? expect)
                        IllegalArgumentException.
                        ":is requires no fn after it"
                    )
                    `(def ~(symbol (format "--testcase-%s" (str cs)))
                        (#'partial ~testbench
                            (#'partial #'test-is '~expr
                                ~(eval (list #'fn '[] expr))
                                ~(eval (list #'fn '[] expect))
                            )
                        )
                    )
                )
            :eq
                (do
                    (throw-if-not (fn? (eval expr))
                        IllegalArgumentException.
                        ":eq requires fn before it"
                    )
                    (throw-if-not (fn? (eval expect))
                        IllegalArgumentException.
                        ":eq requires fn after it"
                    )
                    `(def ~(symbol (format "--testcase-%s" (str cs)))
                        (#'partial ~testbench
                            (#'partial #'test-eq '~expr
                                ~(eval expr)
                                ~(eval expect)
                            )
                        )
                    )
                )
            :throws
                (let [expect (resolve-it expect)]
                    (throw-if-not (fn? (eval expr))
                        IllegalArgumentException.
                        ":throws requires fn before it"
                    )
                    (throw-if-not (or (= expect :nothing) (isa? expect Throwable))
                        IllegalArgumentException.
                        ":throws requires an exception class or :nothing after it"
                    )
                    (if (= :nothing expect)
                        `(def ~(symbol (format "--testcase-%s" (str cs)))
                            (#'partial ~testbench
                                (#'partial #'test-throw-nothing '~expr
                                    ~(eval expr)
                                )
                            )
                        )
                        `(def ~(symbol (format "--testcase-%s" (str cs)))
                            (#'partial ~testbench
                                (#'partial #'test-throw '~expr
                                    ~(eval expr) '~expect
                                )
                            )
                        )
                    )
                )
        )
    )
)

(defn- testbench-suite [testbench facts]
    (throw-if-not (= :testbench (first testbench))
        IllegalArgumentException. "require :testbench under \"suite\""
    )
    (throw-if-not (= 2 (count testbench))
        IllegalArgumentException.
        "there must be exactly one testbench function following :testbench"
    )
    (throw-if-not (ifn? (eval (nth testbench 1)))
        IllegalArgumentException.
        "it must be testbench function which is following :testbench"
    )
    (doseq [[fact _ _ rel _] facts]
        (throw-if-not (= :fact fact)
            IllegalArgumentException. "require :fact under \"suite\""
        )
        (throw-if-not (some #(= rel %) [:is :eq :throws])
            IllegalArgumentException.
            "require :is, :eq or :throws between exprs in \"fact\""
        )
    )
    (let [tb (nth testbench 1)]
        (list* 'do (map #(#'one-fact (eval tb) %) facts))
    )
)

(defmacro suite [description & facts]
    (throw-if-not (string? description)
        IllegalArgumentException. "missing description"
    )
    (if (first facts)
        (if (= :testbench (ffirst facts))
            (testbench-suite (first facts) (rest facts))
            (testbench-suite (list :testbench #'testbench-for-basic) facts)
        )
    )
)

(defn- searchForCases [ns]
    (->>
        (for [[cs-symb cs-fn] (ns-publics ns)]
            (let [cs-name (str cs-symb)]
                (if (.startsWith cs-name "--testcase-")
                    [
                        (format "%s.%s" (str ns)
                            (.substring cs-name (.length "--testcase-"))
                        )
                        cs-fn
                    ]
                )
            )
        )
        (filter #(not (nil? %)))
        (into {})
    )
)

(defn load-cases [& namespaces]
    (->> (for [ns namespaces] (searchForCases ns))
        (reduce #(conj %1 %2))
    )
)

(defn- silently-run [cases to-run]
    (doseq [cs to-run]
        ((cases cs))
    )
)

(defn- run [cs]
    (try
        (cs)
        (println)
        (println "RESPONSE: PASS")
    (catch AssertionError ex
        (let [wrt (PrintWriter. *out*)]
            (.printStackTrace ex wrt)
        )
        (println)
        (println "RESPONSE: FAIL")
    )
    (catch Throwable ex
        (with-open [wrt (PrintWriter. *out*)]
            (.printStackTrace ex wrt)
        )
        (System/exit 1)
    ))
)

(defn- interactively-run' [cases in]
    (when-let [csname (.readLine in)]
        (let [cs (cases csname)]
            (run cs)
            (recur cases in)
        )
    )
)

(defn- interactively-run [cases]
    (let [in (io/reader *in*)]
        (interactively-run' cases in)
    )
)

(defn main [args cases]
    (cond
        (empty? args) (interactively-run cases)
        (= args ["--show-cases"]) (do
            (->> (for [[cs-name _] cases] cs-name)
                (sort)
                (str/join "\n")
                (println)
            )
            (System/exit 0)
        )
        (= (count args) 1) (silently-run cases args)
        :else (throw (IllegalArgumentException. (str "unknown args " args)))
    )
)
