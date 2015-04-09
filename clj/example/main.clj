(ns example.main
  (:use [testa.core :only (suite load-cases main)])
  (:gen-class))

(suite ":is usage: compare exprs to values"
  (:fact is-case1 (* 6 7) :is 42)
  (:fact is-case2 (* 6 7) :is (* 3 14))
  (:fact is-fail-case3 (* 5 8) :is 42) ; intend to fail
  (:fact is-fail-case4 (* 6 7) :is "42") ; intend to fail
  (:fact is-fail-case5 (* 6 7) :is (* 5 8)) ; intend to fail
)


(defn gcd [x y]
  (if (= 0 x)
    y
    (recur (rem y x) x)))

(defn gcd-counterpart [x y]
  (if (= 0 x)
    y
    (->> (range 1 (inc' x))
      (filter #(= 0 (rem x %)))
      (filter #(= 0 (rem y %)))
      (apply max))))

(defn testbench [test]
  (let [
    x (rand-int 10)
    y (rand-int 10)
    ]
    (test x y)))

(suite ":eq usage: with a testbench"
  (:testbench testbench)
  (:fact eq-case1 gcd :eq #(gcd-counterpart %1 %2))
  (:fact eq-fail-case2 gcd :eq (fn [_ _] 42)) ; intend to fail
)


(defn f [] (/ 1 0))
(defn g [] (/ 1 1))

(suite ":throw usage: with exception"
  (:fact throw-case1 f :throws ArithmeticException)
  (:fact throw-case2 g :throws :nothing)
  (:fact throw-fail-case1 f :throws NullPointerException) ; intend to fail
  (:fact throw-fail-case2 g :throws ArithmeticException) ; intend to fail
  (:fact throw-fail-case3 f :throws :nothing) ; intend to fail
)

(defn ff []
  (println "hehe"))

(suite "bug fix: outputs in functions under testing appear in compile time"
  (:fact bugfix-1 (ff) :is nil))

(defn -main [& args]
  (->>
    (load-cases
      'example.main)
    (main args)))
