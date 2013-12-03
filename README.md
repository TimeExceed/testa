# testa

Testa is another behaviour-driven testing framework for clojure.

## Why another testing framework?

Allow me to ask one question: what is a testing framework supposed to do?
Easy, two things:

1. Provide a way to write down expectations and compose them into cases and suites.
1. Execute test cases (often a specified subset) with suitable isolation.

Most testing frameworks can do both.
Few of them consider isolation seriously.
But we do.

From the view of isolation, we define four types of test cases, corresponding to four levels of isolation.

1. Pure tests, no isolation.
  These tests target pure functions, so isolation is not necessary at all.
1. Unit tests, process level isolation.
  These tests may touch global in-memory data structures, so they should be isolated by process.
1. Smoke tests, machine level isolation.
  These tests read or write some files on disks,
  or require collaboration of several processes in a single machine, which are often tests' responsibility to start and stop those processes.
  So they can be executed in parallel by different machines (e.g., real machines/[virtualbox](https://www.virtualbox.org/)/[docker](http://www.docker.io/)).
1. Functional tests, cluster level isolation.
  Like smoke tests, functional tests require collaboration of several processes, but they can never be deployed in the same machine.
  Usually, machines with necessary processes are just there, as environment.
  Tests generate some stimulations to see whether these processes behave expectedly.
  Therefore, tests can only be executed one by one.

## What is testa?

Testa, consists of both a library (`testa.core`) and an application (`runtests.jar`).
Users define their tests for all the four types of tests in the same way by the library,
compile them into executable jar files and then run these jar files by `runtests.jar`.
`runtests.jar` can run test cases in parallel with suitable isolation.

## How to define test cases and suite?

First, you should involve `suite` macro, like:

    (use `[testa.core :only (suite)])

Second, define your test suite with test cases in it, like:

    (suite ":is usage: compare exprs to values"
      (:fact is-case1 (* 6 7) :is 42)
      (:fact is-case2 (* 6 7) :is (* 3 14))
    )

where `is-case1` and `is-case2` are shorthands of names of these two test cases.
Their whole names are `example.main.is-case1` and `example.main.is-case2` respectively.
(Suppose `example.main` is the name space defining them.)

Last step, connect your test suites to an entry point, like:

    (defn -main [& args]
      (->>
        (load-cases
          'example.main)
        (main args)))

`load-cases` can accept a series of name spaces and search test cases in them.

Besides `:is`, there are also `:eq` and `:throws`.

* `:is` accepts two lists and expects evaluating them will cause the same result.
* `:eq` accepts two functions and expects applying same inputs to them will cause the same result.

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
        )

  In this example, `:testbench` generates two integer numbers between 0 and 10.
  `:eq` takes these two numbers and applies them to both `gcd` and `gcd-counterpart`(to be precisely, the anonymous function `#(gcd-counterpart %1 %2)`).

  There is an issue that `constantly` can be used on both hands of `:eq`.
  But this is not a big problem.
  We can use `(fn [& _] ...)` instead.
* `:throws` accepts a function on its left hand and an exception on its right hand.
  While evaluating the left-hand function, the right-hand exception is expected to be thrown.

      (suite ":throw usage: with exception"
        (:fact throw-case1 (fn [] (/ 1 0)) :throws ArithmeticException)
      )

A few more words on `:testbench`.

Test benches are usefully for two purposes.
One is setting up environments before running test cases and cleaning them up after their execution, like `setUp` and `tearDown` in [junit](http://junit.org/).
Another is test input generation, like [QuickCheck](http://www.cse.chalmers.se/~rjmh/QuickCheck/) in Haskell.

To support both scenes, each test case defined by `:fact` is actually invoked by the test bench specified in their suite.
(if not specified, a default do-nothing test bench will be applied.)
As shown by the `:eq` example, test bench accepts a single parameter, `test`, which is actually a test case, or say `:fact` in precise.

`:testbench` can also work with `:is`, since `:is` is in fact a syntax sugar of `:eq`.
Strictly, `(:fact N X :is Y)` is equivalent to `(:fact N (fn [] X) :eq (fn [] Y))`.

All examples above and more can be found in example/main.clj.

## How to run tests

Here are some examples.

First, the simplest way to run tests.

    $ java -jar runtests.jar example.jar
      1/13 [PASS] example.main.bugfix-1 example.jar
      2/13 [PASS] example.main.eq-case1 example.jar
      3/13 [FAIL] example.main.eq-fail-case2 example.jar
      4/13 [PASS] example.main.is-case1 example.jar
      5/13 [PASS] example.main.is-case2 example.jar
      6/13 [FAIL] example.main.is-fail-case3 example.jar
      7/13 [FAIL] example.main.is-fail-case4 example.jar
      8/13 [FAIL] example.main.is-fail-case5 example.jar
      9/13 [PASS] example.main.throw-case1 example.jar
     10/13 [PASS] example.main.throw-case2 example.jar
     11/13 [FAIL] example.main.throw-fail-case1 example.jar
     12/13 [FAIL] example.main.throw-fail-case2 example.jar
     13/13 [FAIL] example.main.throw-fail-case3 example.jar
      7/13 failed

We can see 7 out of 13 cases are failed.

There must be a work directory for `runtests.jar`.
By default, it is `res/` under current directory.
One can look at `res/failed` for all failed cases,
and `res/TESTCASE.out`, where `TESTCASE` must be replaced by real case name, for both standard out and standard err for output of this test case.

    $ cat res/failed
    example.main.eq-fail-case2 example.jar
    example.main.is-fail-case3 example.jar
    example.main.is-fail-case4 example.jar
    example.main.is-fail-case5 example.jar
    example.main.throw-fail-case1 example.jar
    example.main.throw-fail-case2 example.jar
    example.main.throw-fail-case3 example.jar
    $ cat res/example.main.is-fail-case3.out
    Exception in thread "main" java.lang.AssertionError: (* 5 8) expects 42, but 40 actually
      at testa.core$test_is.invoke(core.clj:29)
      ...

The test example.main.is-fail-case3 is:

    (:fact is-fail-case3 (* 5 8) :is 42)

The content of `res/example.main.is-fail-case3.out`, i.e., the standard out and standard err of executing example.main.is-fail-case3, says we get 40 rather than 42 for `(* 5 8)`.

In my virtualbox, this example takes tens of seconds.
This is because `runtests.jar` can not recognize type of `example.jar`, so it regards it as functional test jar-file.
Then it runs tests in it one by one, i.e., starts a JVM, runs one single test, shuts the JVM down, starts another JVM, runs the next test, shuts the JVM down again, and so on.

To speed it up, we have to obey the naming convention of test jar-files.
Say, `XXX_puretest.jar` consists of pure tests, and `XXX_unittest.jar`, `XXX_smoketest.jar`, `XXX_functests.jar` of unit tests, smoke tests and functional tests respectively.

Let us try.

    $ cp example.jar example_unittest.jar
    $ java -jar runtests.jar example_unittest.jar

Good!
We save basically half of time in a two-core machine.
`runtests.jar` will launch as many workers as cpus in system to run unit tests.

    $ cp example.jar example_puretest.jar
    $ time java -jar runtests.jar example_puretest.jar

Much better!
Since pure tests need no isolation at all, `runtests.jar` reuses JVM to reduce JVM starting-up time.

In development, we often want to run a small set of tests.
`runtests.jar` allow us to do that by `--cases` option.
It accepts a regular expression (of course Java-style).
Only test cases whose names match this regular expression will be executed.

    $ java -jar runtests.jar example.jar --cases ".*is.*"
      1/5 [PASS] example.main.is-case1 example.jar
      2/5 [PASS] example.main.is-case2 example.jar
      3/5 [FAIL] example.main.is-fail-case3 example.jar
      4/5 [FAIL] example.main.is-fail-case4 example.jar
      5/5 [FAIL] example.main.is-fail-case5 example.jar
      3/5 failed

See?
Only test cases related to `:is` are executed.

## How to build?

Please make sure the following requirements are ready.
* jdk6+. Testa is developed under jdk7, but we think jdk6 is ok too.
* python2.5+. necessary to run scons.
* scons 2.1+.
* wget
* network connection for first build

Then, under the root directory of testa, run `scons`, and all jar-files will be placed under `build/` directory.
