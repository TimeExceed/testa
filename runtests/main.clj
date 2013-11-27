(ns runtests.main
  (:require
    [clojure.string :as str]
    [clojure.java.io :as io]
    [clansi.core :as color]
    [runtests.utilities :as util]
    [runtests.argparser :as arg])
  (:import
    [java.io File FileReader ByteArrayInputStream]
    [java.nio.file Files FileSystems Path]
    [java.nio.file.attribute FileAttribute]
    [java.util.concurrent ArrayBlockingQueue])
  (:gen-class))

(defn scan-test-jar [work-dir exejar]
  (let [
    cmd ["java" "-jar" exejar "--show-cases"]
    f (->> exejar
        (util/str->bytes)
        (ByteArrayInputStream.)
        (util/sha1-stream)
        (util/hexdigits)
        (format "%s.show-cases")
        (File. work-dir))
    {exitcode :exitcode} (util/execute cmd :out f)
    ]
    (if (= 0 exitcode)
      (let [res (-> (slurp f) (str/trim))]
        (if (empty? res)
            []
            (str/split res #"\n")))
        (do
          (println "execution fail: " cmd)
          (System/exit 1)))))

(defn scan-test-jars [work-dir exejars]
  (for [
    x exejars
    csn (scan-test-jar work-dir x)
    ]
    [csn x]))

(def waiting (ArrayBlockingQueue. 100))
(def outq (ArrayBlockingQueue. 100))

(defn- run-in-new-process [work-dir cs-name exejar]
  (let [
    out (->> cs-name
      (format "%s.out")
      (File. work-dir))
    {:keys [exitcode]} (util/execute ["java" "-jar" exejar cs-name]
        :out out :err :out)
    ]
    (if (= 0 exitcode)
      (.put outq [:pass cs-name exejar])
      (.put outq [:fail cs-name exejar]))))

(defn- new-process [exejar]
  (util/popen ["java" "-jar" exejar] :in :pipe :err :out :out :pipe))

(defn- read-until-case-finish' [sb out]
  (let [x (.readLine out)]
    (cond
      (nil? x) :quit
      (= x "RESPONSE: PASS") :pass
      (= x "RESPONSE: FAIL") :fail
      :else (do
        (.append sb x)
        (.append sb \newline)
        (recur sb out)))))

(defn- read-until-case-finish [proc]
  (let [
    sb (StringBuilder.)
    out (io/reader (.getInputStream proc))
    res (read-until-case-finish' sb out)
    ]
    [res (str sb)]))

(defn- dump-a-case [proc c]
  (doto (io/writer (.getOutputStream proc))
    (.write c)
    (.newLine)
    (.flush)))

(declare run-in-shared-process)

(defn- run-in-shared-process' [work-dir exejar cs-names proc]
  (when-not (empty? cs-names)
    (let [
      [c & cs] cs-names
      _ (dump-a-case proc c)
      [res output] (read-until-case-finish proc)
      ]
      (spit (format "%s/%s.out" work-dir c) output)
      (if (= res :pass)
        (.put outq [:pass c exejar])
        (.put outq [:fail c exejar]))
      (if (= res :quit)
        (run-in-shared-process work-dir exejar cs)
        (recur work-dir exejar cs proc)))))

(defn- run-in-shared-process [work-dir exejar cs-names]
  (let [p (new-process exejar)]
    (run-in-shared-process' work-dir exejar cs-names p)
    (.destroy p)))

(defn- provider [work-dir cases parallel]
  (let [
    pts (:pt cases)
    g (group-by second pts)
    ]
    (doseq [
      [exejar cs] g
      :let [cs (map first cs)]
      ]
      (.put waiting (partial run-in-shared-process work-dir exejar cs))))
  (let [uts (:ut cases)]
    (doseq [[cs-name exejar] uts]
      (.put waiting (partial run-in-new-process work-dir cs-name exejar))))
  (doseq [x (range (dec parallel))]
    (.put waiting :quit))
  (let [remains (:else cases)]
    (doseq [[cs-name exejar] remains]
      (.put waiting (partial run-in-new-process work-dir cs-name exejar))))
  (.put waiting :quit))

(defn- worker []
  (let [work-item (.take waiting)]
    (when (not= work-item :quit)
      (work-item)
      (recur))))

(defn fetch-results [k ncase]
  (lazy-seq
    (when (< k ncase)
      (let [
        [result csn jar] (.take outq)
        show-res (case result
          :pass (color/style "PASS" :green)
          :fail (color/style "FAIL" :red))
        nk (inc k)
        ]
        (printf "%3d/%d [%s] %s %s" nk ncase show-res csn jar)
        (println)
        (cons [result csn jar] (fetch-results nk ncase))))))

(defn- count-cases [cases]
  (->> cases
    (vals)
    (map count)
    (reduce + 0)))

(defn- execute-all [parallel dir cases]
  (future (provider dir cases parallel))
  (doseq [_ (range parallel)] (future (worker)))
  (let [
    failed (doall
      (for [
        [result cs-name exejar] (fetch-results 0 (count-cases cases))
        :when (= result :fail)
        ]
        [cs-name exejar]))
    ]
    (shutdown-agents)
    failed))

(defn- detect-cores []
  (.availableProcessors (Runtime/getRuntime)))

(defn- classifier [[cs-name exejar]]
  (cond
    (.endsWith exejar "puretest.jar") :pt
    (.endsWith exejar "unittest.jar") :ut
    :else :else))

(defn parseArgs [args]
  (let [
    arg-spec {
      :usage "Usage: [options] exejar ..."
      :args [
        (arg/opt :cases
          "--cases <pattern>"
          "a regexp which matches all cases to run, default \".*\"")
        (arg/opt :dir
          "-d|--dir <dir>"
          "a directory to put results, default \"res\"")
        (arg/opt :else
          "exejar" "add exejar to run")]}
    opts (arg/transform->map (arg/parse arg-spec args))
    ]
    (-> opts
      (arg/extract-with-default :dir first "res")
      (arg/extract-with-default :cases #(->> % (first) (re-pattern)) #".*")
      (assoc :parallel (detect-cores)))))

(defn -main [& args]
  (let [
    opts (parseArgs args)
    dir (:dir opts)
    _ (.mkdirs (File. dir))
    pat (:cases opts)
    parallel (:parallel opts)
    exejars (:else opts)
    cases (->> exejars
      (scan-test-jars dir)
      (filter (fn [[cs-name _]] (re-matches pat cs-name)))
      (group-by classifier)
    )
    failed (execute-all parallel dir cases)
    ]
    (println (format "%3d/%d failed" (count failed) (count-cases cases)))
    (spit (format "%s/failed" dir)
      (str/join "\n"
        (for [cs failed]
          (str/join " " cs))))))
