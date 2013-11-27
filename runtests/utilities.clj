(ns runtests.utilities
  (:require
    [clojure.java.io :as io])
  (:import
    [java.nio.charset StandardCharsets]
    [java.io File StringWriter InputStream]
    [java.security MessageDigest]))

(defmacro throw-if [pred except & args]
  `(when ~pred (throw (~except ~@args))))

(defn zip [& args]
  (apply map vector args))

(defn enumerate
  ([init xs]
  (zip (iterate inc' init) xs))

  ([xs]
  (enumerate 0 xs))
)

(defn- array-lazy-seq' [arr i]
  (if (= i (alength arr))
    []
    (lazy-seq
      (cons (aget arr i) (array-lazy-seq' arr (inc i))))))

(defn array->lazy-seq [arr]
  (array-lazy-seq' arr 0))

(defn str->bytes ^bytes [^String s]
  (.getBytes s StandardCharsets/UTF_8))

(defn hash-stream [^bytes buf ^InputStream in ^MessageDigest hasher]
  (let [sz (.read in buf)]
    (if (< sz 0)
      (.digest hasher)
      (do
        (.update hasher buf 0 sz)
        (recur buf in hasher)))))

(defn sha1-stream
  ([^InputStream in]
  (let [buf (byte-array (* 512 1024))] (sha1-stream buf in)))

  ([^bytes buf ^InputStream in]
  (let [hasher (MessageDigest/getInstance "SHA-1")] (hash-stream buf in hasher)))
)

(defn low-byte->char [b]
{
  :pre [(<= 0 b 15)]
}
  (cond
    (<= 0 b 9) (char (+ b 48))
    :else (char (+ b 87))))

(defn byte->digits! [^StringBuilder sb b]
{
  :pre [(<= -128 b 127)]
}
  (if-not (neg? b)
    (do
      (.append sb (low-byte->char (quot b 16)))
      (.append sb (low-byte->char (rem b 16))))
    (let [b (+ b 256)]
      (.append sb (low-byte->char (quot b 16)))
      (.append sb (low-byte->char (rem b 16))))))

(defn hexdigits [^bytes bytes]
  (let [sb (StringBuilder. (* 2 (alength bytes)))]
    (doseq [b (array->lazy-seq bytes)]
      (byte->digits! sb b))
    (str sb)))

(defn popen [cmd & opts]
{
  :doc "Execute a program in subprocess.

  cmd - command line args to start the program
  opts - environments and redirections
  :in - possible values include :inherit, :pipe, a file or a string.
      :inherit, the default, will redirect stdin of subprocess from the
          parent process.
      :pipe will open an OutputStream in the parent process, linking to stdin
          of the subprocess.
      a file, i.e., an instance of File, will redirect the stdin of
          subprocess from the file.
      a string, will push the string into stdin of the subprocess.
  :out - possible values include :inherit, :pipe or a file.
      :inherit, the default, will redirect stdout to that of the parent process.
      a file, i.e., an instance of File, will overwrite the file by
          contents from stdout.
      :pipe will put stdout of subprocess to an InputStream of the parent process.
  :err - possible values include :inherit, :pipe, :out or a file.
      :inherit, the default, will redirect stderr to that of the parent process.
      :pipe will put stderr of subprocess to an InputStream of the parent process
      a file, i.e., an instance of File, will overwrite the file by
          contents from stderr.
      :out will redirect stderr to its stdout.
  This returns the subprocess.
  "
}
  (let [
    opts (merge {:in :inherit, :out :inherit, :err :inherit}
      (apply hash-map opts))
    pb (ProcessBuilder. (into-array String cmd))
    ]
    (let [in (:in opts)]
      (cond
        (= :inherit in)
          (.redirectInput pb java.lang.ProcessBuilder$Redirect/INHERIT)
        (= :pipe in)
          (.redirectInput pb java.lang.ProcessBuilder$Redirect/PIPE)
        (string? in)
          (.redirectInput pb java.lang.ProcessBuilder$Redirect/PIPE)
        (instance? File in)
          (.redirectInput pb in)
        :else (throw (IllegalArgumentException.
          (str "unknown argument for :in " in)))))
    (let [out (:out opts)]
      (cond
        (= :inherit out)
          (.redirectOutput pb java.lang.ProcessBuilder$Redirect/INHERIT)
        (= :pipe out)
          (.redirectOutput pb java.lang.ProcessBuilder$Redirect/PIPE)
        (instance? File out)
          (.redirectOutput pb out)
        :else (throw (IllegalArgumentException.
          (str "unknown argument for :out " out)))))
    (let [err (:err opts)]
      (cond
        (= :inherit err)
          (.redirectError pb java.lang.ProcessBuilder$Redirect/INHERIT)
        (= :pipe err)
          (.redirectError pb java.lang.ProcessBuilder$Redirect/PIPE)
        (= :out err)
          (.redirectErrorStream pb true)
        (instance? File err)
          (.redirectError pb err)
        :else (throw (IllegalArgumentException.
          (str "unknown argument for :err " err)))))
    (when-let [dir (:dir opts)]
      (cond
        (instance? String dir) (.directory pb (File. dir))
        (instance? File dir) (.directory pb dir)
        :else (throw (IllegalArgumentException.
          (str "unknown argument for :dir " dir)))))
    (let [p (.start pb)]
      (if (string? (:in opts))
        (with-open [wtr (io/writer (.getOutputStream p))]
          (.write wtr (:in opts))))
      p)))

(defn execute [cmd & opts]
{
  :doc "Execute a program in subprocess.

  cmd - command line args to start the program
  opts - environments and redirections
  :in - possible values include :inherit, a file or a string.
      :inherit, the default, will redirect stdin of subprocess from the
          parent process.
      a file, i.e., an instance of File, will redirect the stdin of
          subproces from the file.
      a string, will push the string into stdin of the subprocess.
  :out - possible values include :inherit, :pipe or a file.
      :inherit, the default, will redirect stdout to that of the parent process.
      a file, i.e., an instance of File, will overwrite the file by
          contents from stdout.
      :pipe will results a string in return, keyed by :out. Use it carefully,
          for it will possibly cause subprocess hung.
  :err - possible values include :inherit, :pipe, :out or a file.
      :inherit, the default, will redirect stderr to that of the parent process.
      :pipe will results a string in return, keyed by :err. Use it carefully,
          for it will possibly cause subprocess hung.
      a file, i.e., an instance of File, will overwrite the file by
          contents from stderr.
      :out will redirect stderr to its stdout.
  This returns a map which contains :exitcode, and optionally :out and :err.
  "
}
  (let [
    p (apply popen cmd opts)
    opts (apply hash-map opts)
    ]
    (-> {:exitcode (.waitFor p)}
      (merge
        (if (= :pipe (:out opts))
          {:out (slurp (.getInputStream p))}
          {}))
      (merge
        (if (= :pipe (:err opts))
          {:err (slurp (.getErrorStream p))}
          {})))))
