;; This file is picked from project testa [https://github.com/TimeExceed/testa.git]
;; Copyright (c) 2013, Taoda (tyf00@aliyun.com)
;; All rights reserved.
;;
;; Redistribution and use in source and binary forms, with or without modification,
;; are permitted provided that the following conditions are met:
;;
;; * Redistributions of source code must retain the above copyright notice, this
;;   list of conditions and the following disclaimer.
;;
;; * Redistributions in binary form must reproduce the above copyright notice, this
;;   list of conditions and the following disclaimer in the documentation and/or
;;   other materials provided with the distribution.
;;
;; * Neither the name of the {organization} nor the names of its
;;   contributors may be used to endorse or promote products derived from
;;   this software without specific prior written permission.
;;
;; THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
;; ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
;; WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
;; DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
;; ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
;; (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
;; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
;; ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
;; (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
;; SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

(ns testa.core
    (:require
        [clojure.string :as str]
    )
    (:import
        [java.io PrintWriter]
    )
)

(defmacro ^:private throw-if [pred except & args]
    `(when ~pred
        (throw (~except ~@args))))

(defmacro ^:private throw-if-not [pred except & args]
    `(throw-if (not ~pred) ~except ~@args))

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

(defn- test-verify [actual verify & args]
    (let [actual (apply actual args)]
        (apply verify actual args)
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
            :verify
                (do
                    (throw-if-not (fn? (eval expr))
                        IllegalArgumentException.
                        ":verify requires fn before it"
                    )
                    (throw-if-not (fn? (eval expect))
                        IllegalArgumentException.
                        ":verify requires fn after it"
                    )
                    `(def ~(symbol (format "--testcase-%s" (str cs)))
                        (#'partial ~testbench
                            (#'partial #'test-verify
                                ~(eval expr)
                                ~(eval expect)
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
        (throw-if-not (some #(= rel %) [:is :eq :throws :verify])
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
        (try
            ((cases cs))
        (catch Throwable ex
            (with-open [wrt (PrintWriter. *out*)]
                (.printStackTrace ex wrt)
            )
            (System/exit 1)
        ))
    )
)

(defn main [args cases]
    (cond
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
