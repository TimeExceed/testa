#!/bin/env python2.6
# -*- coding: UTF-8 -*-
import optparse
import sys
import os
import os.path as op
from Queue import Queue
import threading
import json
import subprocess as subprocess
import traceback
import re
import termcolor
from functools import partial
from itertools import groupby
from datetime import datetime, timedelta
from time import sleep

LUA = ['lua']
JAVA = ['java', '-jar']
VALGRIND = 'valgrind'

def countCpu():
    '''
    Returns the number of CPUs in the system
    Works only under *nix.
    '''
    try:
        num = os.sysconf('SC_NPROCESSORS_ONLN')
        return num
    except (ValueError, OSError, AttributeError):
        raise NotImplementedError('cannot determine number of cpus')

def parseArgs(args):
    parser = optparse.OptionParser("usage: %%prog [options] unittest-binary ...")
    parser.add_option('-j', '--jobs', action='store', type='int', dest='jobs',
        help='how many test cases can run parallelly [default: as many as CPU cores]')
    parser.add_option('-d', '--dir', action='store', type='string', dest='dir',
        default='test_results',
        help='the directory where results of test cases are put [default: test_results]')
    parser.add_option('-i', '--include', action='store',type='string',
        dest='include',
        help='A regular expression. It will be run for only test cases \
containing this pattern. [default: all]')
    parser.add_option('-e', '--exclude', action='store',type='string',
        dest='exclude',
        help='A regular expression. It will not be run for all \
test cases containing this pattern. [default: none]')
    parser.add_option('--time-limit', action='store',type='int',
        dest='timeout',
        help='how long a case is allowed to run (in msec) [default: disable]')
    parser.add_option('--valgrind', action='store', type='string', dest='valgrind',
        help='run test cases by valgrind with valgrind options as json array, \
e.g., --valgrind=["--error-exitcode=1"] [default: disable]')
    opts, executables = parser.parse_args(args)

    if not opts.jobs:
        opts.jobs = countCpu()
    return opts, executables[1:]


class ExceptInThread(Exception):
    def __init__(self, tid, exc, tb):
        self.tid = tid
        self.exc = exc
        self.tb = tb

    def show(self):
        print>>sys.stderr, 'An exception occurs in thread %d: %s' % (
            self.tid, str(self.exc))
        traceback.print_tb(self.tb)

def threadLoop(tid, qin, qout):
    while True:
        task = qin.get(True)
        if task is None:
            return
        try:
            res = task()
            qout.put(res)
        except Exception, ex:
            qout.put(ExceptInThread(tid, ex, sys.exc_info()[2]))

class ThreadPool(object):
    def __init__(self, jobs, qin, qout, func):
        self.threads = []
        for i in range(jobs):
            x = threading.Thread(target=func, args=(i, qin, qout))
            x.setDaemon(False)
            x.start()
            self.threads.append(x)
        self.qin = qin

    def __enter__(self):
        return self
    
    def __exit__(self, exc_type, exc_value, traceback):
        for _ in self.threads:
            self.qin.put(None)
        for t in self.threads:
            t.join()


def classifyLua(lua):
    if lua.endswith('_unittest.lua'):
        return 'unit test'
    elif lua.endswith('_smoketest.lua'):
        return 'smoke test'
    else:
        return 'functional test'

def fetchUTs_lua(lua, opts):
    baseDir = opts.dir
    executable = op.basename(lua)
    tmpout = op.join(baseDir, '%s.out' % executable)
    tmperr = op.join(baseDir, '%s.err' % executable)
    cmd = LUA + [op.abspath(lua), '--show-cases']
    with open(tmpout, 'wb') as cout:
        with open(tmperr, 'wb') as cerr:
            subprocess.check_call(cmd, stdout=cout, stderr=cerr, cwd=op.dirname(lua))
    with open(tmpout) as f:
        res = [x.strip() for x in f]
    res = [x for x in res if x]
    res = [{
        'executable': executable, 
        'cmd': LUA + [op.abspath(lua), x],
        'cwd': op.abspath(op.dirname(lua)),
        'outfn': genOutFn(executable, x, baseDir),
        'errfn': genErrFn(executable, x, baseDir),
        'corefn': genCoreFn(executable, x, baseDir),
        'casename': x,
        'class': classifyLua(lua),
    } for x in res]
    return res

def classifyJava(jar):
    if jar.endswith('_puretest.jar'):
        return 'pure test'
    elif jar.endswith('_unittest.jar'):
        return 'unit test'
    elif jar.endswith('_smoketest.jar'):
        return 'smoke test'
    else:
        return 'functional test'

def fetchUTs_java(jar, opts):
    baseDir = opts.dir
    executable = op.basename(jar)
    tmpout = op.join(baseDir, '%s.out' % executable)
    tmperr = op.join(baseDir, '%s.err' % executable)
    cmd = JAVA + [op.abspath(jar), '--show-cases']
    with open(tmpout, 'wb') as cout:
        with open(tmperr, 'wb') as cerr:
            subprocess.check_call(cmd, stdout=cout, stderr=cerr, cwd=op.dirname(jar))
    with open(tmpout) as f:
        res = [x.strip() for x in f]
    res = [x for x in res if x]
    res = [{
        'executable': executable,
        'cmd': JAVA + [op.abspath(jar), x],
        'cwd': op.abspath(op.dirname(jar)),
        'outfn': genOutFn(executable, x, baseDir),
        'errfn': genErrFn(executable, x, baseDir),
        'corefn': genCoreFn(executable, x, baseDir),
        'casename': x,
        'class': classifyJava(jar),
    } for x in res]
    return res

def classifyRaw(exe):
    if exe.endswith('_puretest'):
        return 'pure test'
    elif exe.endswith('_unittest'):
        return 'unit test'
    elif exe.endswith('_smoketest'):
        return 'smoke test'
    else:
        return 'functional test'

def fetchUTs_raw(exe, opts):
    baseDir = opts.dir
    executable = op.basename(exe)
    tmpout = op.join(baseDir, '%s.out' % executable)
    tmperr = op.join(baseDir, '%s.err' % executable)
    cmd = JAVA + [op.abspath(exe), '--show-cases']
    with open(tmpout, 'wb') as cout:
        with open(tmperr, 'wb') as cerr:
            subprocess.check_call(cmd, stdout=cout, stderr=cerr, cwd=op.dirname(exe))
    with open(tmpout) as f:
        res = [x.strip() for x in f]
    res = [x for x in res if x]
    res = [{
        'executable': executable,
        'cmd': [op.abspath(exe), x],
        'cwd': op.abspath(op.dirname(exe)),
        'outfn': genOutFn(executable, x, baseDir),
        'errfn': genErrFn(executable, x, baseDir),
        'corefn': genCoreFn(executable, x, baseDir),
        'casename': x,
        'class': classifyRaw(exe),
    } for x in res]
    return res

def fetchUTs_comb(qin, qout, exes, opts):
    for e in exes:
        if e.endswith('.jar'):
            qin.put(partial(fetchUTs_java, e, opts))
        elif e.endswith('.lua'):
            qin.put(partial(fetchUTs_lua, e, opts))
        else:
            qin.put(partial(fetchUTs_raw, e, opts))
    r = []
    for _ in exes:
        x = qout.get()
        if isinstance(x, ExceptInThread):
            x.show()
            sys.exit(1)
        assert isinstance(x, list)
        r += x
    return r

def include(cases, opts):
    if not opts.include:
        return cases
    pat = re.compile(opts.include)
    return [x for x in cases if pat.search(x['casename'])]

def exclude(cases, opts):
    if not opts.exclude:
        return cases
    pat = re.compile(opts.exclude)
    return [x for x in cases if not pat.search(x['casename'])]

def genOutFn(exe, case, base):
    return op.join(op.abspath(base), '%s.%s.out' % (op.basename(exe), case))

def genErrFn(exe, case, base):
    return op.join(op.abspath(base), '%s.%s.err' % (op.basename(exe), case))

def genCoreFn(exe, case, base):
    return op.join(op.abspath(base), '%s.%s.core' % (op.basename(exe), case))

def myResult(exitcode, case):
    if exitcode is None:
        return 'TIMEOUT'
    elif exitcode == 0:
        return 'PASS'
    else:
        return 'FAIL'

def runSingleCase(case, opts):
    base = opts.dir
    exe = case['executable']
    casename = case['casename']
    cwd = case['cwd']
    outfn = case['outfn']
    errfn = case['errfn']
    corefn = case['corefn']
    cmd = case['cmd']
    if opts.valgrind:
        valgrindOpts = json.loads(opts.valgrind)
        cmd = [VALGRIND, '--error-exitcode=1'] + valgrindOpts + cmd
    with open(outfn, 'wb') as cout:
        with open(errfn, 'wb') as cerr:
            p = subprocess.Popen(cmd, stdout=cout, stderr=cerr, cwd=cwd)
            if not opts.timeout:
                exitcode = p.wait()
            else:
                duration = timedelta(milliseconds=opts.timeout)
                start = datetime.utcnow()
                while True:
                    exitcode = p.poll()
                    if exitcode is not None:
                        break
                    if datetime.utcnow() - start > duration:
                        break
                    sleep(0.1)
                if exitcode is None:
                    subprocess.call(['gcore', '-o', corefn, '%d' % p.pid], 
                        stdout=cout, stderr=cerr)
                    subprocess.call(['kill', '-9', '%d' % p.pid],
                        stdout=cout, stderr=cerr)
    res = case.copy()
    res['result'] = myResult(exitcode, case)
    return res

def dispatchCases(cases, opts, qin):
    ptut = [k for k in cases if k['class'] in ['pure test', 'unit test']]
    for case in ptut:
        qin.put(partial(runSingleCase, case, opts))
    for i in range(opts.jobs - 1):
        qin.put(None) # shutdown all workers except one
    stft = [k for k in cases if k['class'] in ['smoke test', 'functional test']]
    for case in stft:
        qin.put(partial(runSingleCase, case, opts))
    qin.put(None) # shutdown the last worker

def colored(s, color):
    if not os.isatty(sys.stdout.fileno()):
        return s
    else:
        return termcolor.colored(s, color)

def collectResults(cases, opts, qout):
    passed = []
    failed = []
    caseNum = len(cases)
    while len(passed) + len(failed) < caseNum:
        x = qout.get()
        if isinstance(x, ExceptInThread):
            x.show()
            sys.exit(1)
        case = x
        if case['result'] == 'PASS':
            passed.append(case)
            result = colored('pass', 'green')
        elif case['result'] == 'FAIL':
            failed.append(case)
            result = colored('fail', 'red')
        elif case['result'] == 'KILL':
            failed.append(case)
            result = colored('kill', 'red')
        else:
            assert false
        print '%d/%d %s: %s in %s' % (
            len(passed) + len(failed), caseNum, result, case['casename'], 
            case['executable'])
    return passed, failed

def compareCase(a, b):
    if a['executable'] < b['executable']:
        return -1
    elif a['executable'] > b['executable']:
        return 1
    else:
        if a['casename'] < b['casename']:
            return -1
        elif a['casename'] > b['casename']:
            return 1
        else:
            return 0

def formReport(cases):
    cases = sorted(cases, cmp=compareCase)
    return {'cases': cases}

def iso8601(dt):
    s = dt.isoformat()
    if dt.utcoffset():
        return s
    else:
        return s + 'Z'

def main(exes, opts):
    assert isinstance(exes, list)
    assert exes
    assert opts.jobs
    assert opts.jobs > 0
    assert opts.dir
    if not op.exists(opts.dir):
        os.mkdir(opts.dir)
    assert op.isdir(opts.dir)
    assert not opts.timeout or opts.timeout > 0

    start = datetime.utcnow()
    qin = Queue()
    qout = Queue()
    with ThreadPool(opts.jobs, qin, qout, threadLoop):
        cases = fetchUTs_comb(qin, qout, exes, opts)
        cases = include(cases, opts)
        cases = exclude(cases, opts)
        dispatchCases(cases, opts, qin)
        passed, failed = collectResults(cases, opts, qout)
    end = datetime.utcnow()
    print '\n%d/%d failed, costs %s secs' % (len(failed), len(passed) + len(failed), 
        end - start)
    failed.sort(cmp=compareCase)

    if failed:
        for case in failed:
            print
            print '%s in %s' % (case['casename'], case['executable'])
            with open(genOutFn(case['executable'], case['casename'], opts.dir)) as fin:
                c = fin.read()
            print c

    report = formReport(passed + failed)
    report['startTimestamp'] = iso8601(start)
    report['endTimestamp'] = iso8601(end)
    with open(op.join(opts.dir, 'report.json'), 'w') as fout:
        json.dump(report, fout, indent=2)

    return 1 if failed else 0

if __name__ == '__main__':
    opts, exes = parseArgs(sys.argv)
    exitcode = main(exes, opts)
    sys.exit(exitcode)
