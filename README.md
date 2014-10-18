# testa

Testa is another testing framework for clojure and lua.

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

Testa, consists of both library (`testa.core` in clojure and `testa` in lua) and an application (`runtests.py`).
Users define their tests by `testa` library and run these tests by `runtests.py`.
`runtests.py` can run test cases in parallel with suitable isolation.

## How to define test cases and suite?

### In Clojure

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

### In Lua

Testa for Lua is conceptually is same as that for Clojure.
Please see lua/testa_test.lua for examples.

## How to run tests

Here are some examples.

First, the simplest way to run tests.

    $ python runtests.py example.jar
    1/13 pass: example.main.bugfix-1 in example.jar
    2/13 pass: example.main.eq-case1 in example.jar
    3/13 fail: example.main.eq-fail-case2 in example.jar
    4/13 pass: example.main.is-case1 in example.jar
    5/13 pass: example.main.is-case2 in example.jar
    6/13 fail: example.main.is-fail-case3 in example.jar
    7/13 fail: example.main.is-fail-case4 in example.jar
    8/13 fail: example.main.is-fail-case5 in example.jar
    9/13 pass: example.main.throw-case1 in example.jar
    10/13 pass: example.main.throw-case2 in example.jar
    11/13 fail: example.main.throw-fail-case1 in example.jar
    12/13 fail: example.main.throw-fail-case2 in example.jar
    13/13 fail: example.main.throw-fail-case3 in example.jar

    7/13 failed, costs 0:00:21.591279 secs

We can see 7 out of 13 cases are failed.

There must be a work directory for `runtests.py`.
By default, it is `test_results/` under current directory.
One can look at `test_results/report.json` for results of all cases,
and `test_results/EXECUTABLE.TESTCASE.out` or `test_results/EXECUTABLE.TESTCASE.err`, 
where `TESTCASE` must be replaced by real case name,
and `EXECUTABLE` by `example.jar`, 
for both standard out and standard err for output of this test case.

    $ ls -1 test_results/
    report.json
    example.jar.example.main.bugfix-1.err
    example.jar.example.main.bugfix-1.out
    example.jar.example.main.eq-case1.err
    example.jar.example.main.eq-case1.out
    ...
    $ cat test_results/report.json
    {
      "startTimestamp": "2014-10-18T07:29:14.601880Z", 
      "endTimestamp": "2014-10-18T07:29:36.529804Z",
      "cases": [
        {
          "executable": "example.jar", 
          "casename": "example.main.eq-case1", 
          "result": "PASS", 
          "class": "functional test", 
          ...
        }, 
        {
          "executable": "example.jar", 
          "casename": "example.main.eq-fail-case2", 
          "result": "FAIL", 
          "class": "functional test", 
          ...
        }, 
        ...
      ]
    }

    $ cat test_results/example.jar.example.main.is-fail-case3.out
    Exception in thread "main" java.lang.AssertionError: (* 5 8) expects 42, but 40 actually
      at testa.core$test_is.invoke(core.clj:29)
      ...

The test example.main.is-fail-case3 is:

    (:fact is-fail-case3 (* 5 8) :is 42)

The content of `test_results/example.jar.example.main.is-fail-case3.out`, i.e., the standard outof executing example.main.is-fail-case3, says we get 40 rather than 42 for `(* 5 8)`.

In my virtualbox, this example takes tens of seconds.
This is because `runtests.py` can not recognize type of `example.jar`, so it regards it as functional test.
Then it runs tests in it one by one, i.e., starts a JVM, runs one single test, shuts the JVM down, starts another JVM, runs the next test, shuts the JVM down again, and so on.

To speed it up, we have to obey the naming convention of test jar-files.
Say, `XXX_puretest.jar` consists of pure tests, and `XXX_unittest.jar`, `XXX_smoketest.jar`, `XXX_functests.jar` of unit tests, smoke tests and functional tests respectively.

Let us try.

    $ cp example.jar example_unittest.jar
    $ python runtests.py example_unittest.jar

Good!
We save basically half of time in a two-core machine.
`runtests.py` will launch as many workers as cpus in system to run unit tests.

    $ cp example.jar example_puretest.jar
    $ python runtests.py example_puretest.jar

Much better!
Since pure tests need no isolation at all, `runtests.py` reuses JVM to reduce JVM starting-up time.

In development, we often want to run a small set of tests.
`runtests.py` allow us to do that by `--include` and `--exclude` options.
Both accept a regular expression (of course Python-flavored).
Only test cases whose names match regular expression of `--include` and not
match that of `--exclude` will be executed.

    $ python runtests.py example.jar --include ".*is.*"
    1/5 pass: example.main.is-case1 in example.jar
    2/5 pass: example.main.is-case2 in example.jar
    3/5 fail: example.main.is-fail-case3 in example.jar
    4/5 fail: example.main.is-fail-case4 in example.jar
    5/5 fail: example.main.is-fail-case5 in example.jar

    3/5 failed, costs 0:00:09.060452 secs

See?
Only test cases related to `:is` are executed.

## How to build?

Please make sure the following requisitions are ready.
* jdk6+. Testa is developed under jdk7, but we think jdk6 is ok too.
* python2.6+. necessary to run scons and runtests.py
* scons 2.1+.
* wget
* network connection for first build

Then, under the root directory of testa, run `scons`, and all jar-files will be placed under `build/` directory.
