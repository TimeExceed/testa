import argparse
import sys
from pathlib import Path
import json

fixtures = {}

class Case:
    def __init__(self, case_f) -> None:
        self.case_f = case_f
        self.is_broken = False
        self.broken_reason = None

def broken(**kws):
    def go(cs):
        cs.is_broken = True
        assert 'reason' in kws, 'broken case requires a reason'
        cs.broken_reason = kws['reason']
    return go

def _is_testbench(trial_f, expect):
    actual = trial_f()
    if actual != expect:
        return 'expect: {} actual: {}'.format(expect, actual)

def is_(**kws):
    def go(trial_f):
        global fixtures
        casename = trial_f.__name__
        expect = kws['expect']
        res = Case(lambda: _is_testbench(trial_f, expect))
        fixtures[casename] = res
        return res
    return go

def _eq_case_f(trial_f, oracle_f, *args):
    expect = oracle_f(*args)
    actual = trial_f(*args)
    if expect != actual:
        return 'expect: {} actual: {} by applying ({})'\
            .format(
                expect, actual,
                ', '.join(['{}'.format(x) for x in args]))

def eq(**kws):
    def go(trial_f):
        global fixtures
        casename = trial_f.__name__
        tb = kws['testbench']
        oracle_f = kws['oracle']
        case_f = lambda *args: _eq_case_f(trial_f, oracle_f, *args)
        res = Case(lambda: tb(case_f))
        fixtures[casename] = res
        return res
    return go

def _verify_case_f(trial_f, verifier, *args):
    actual = trial_f(*args)
    result = verifier(actual, *args)
    if result is not None:
        return result

def verify(**kws):
    def go(verifier_f):
        global fixtures
        casename = verifier_f.__name__
        tb = kws['testbench']
        trial_f = kws['trial']
        case_f = lambda *args: _verify_case_f(trial_f, verifier_f, *args)
        res = Case(lambda: tb(case_f))
        fixtures[casename] = res
        return res
    return go

def _throws_tb(trial_f, expect_except):
    try:
        trial_f()
        return 'No exception is thrown.'
    except Exception as actual_except:
        if not isinstance(actual_except, expect_except):
            return 'expect {} but got a {}'.format(expect_except, type(actual_except))

def throw(**kws):
    def go(trial_f):
        global fixtures
        casename = trial_f.__name__
        expect_throw = kws['throw']
        res = Case(lambda: _throws_tb(trial_f, expect_throw))
        fixtures[casename] = res
        return res
    return go

def _parse_args():
    parser = argparse.ArgumentParser(description='The wrapper of test cases')
    parser.add_argument(
        'casename', metavar='case', type=str, nargs='?',
        help='name of a test case')
    parser.add_argument(
        '--show-cases', dest='list', action='store_true',
        help='show names of all test cases.')
    args = parser.parse_args()
    if args.list:
        return 'list', None
    else:
        return 'case', args.casename

def main():
    global fixtures
    action, case = _parse_args()
    if action == 'list':
        res = [{'name': x} for x in sorted(fixtures.keys())]
        for x in res:
            cs = fixtures[x['name']]
            if cs.is_broken:
                x['broken'] = True
                x['broken_reason'] = cs.broken_reason
        print(json.dumps(res, sort_keys=True, indent=2))
        sys.exit(0)
    elif action == 'case':
        try:
            t = fixtures[case].case_f
        except KeyError:
            print('unknown case:', case)
            sys.exit(1)
        res = t()
        if res is None:
            sys.exit(0)
        else:
            print(res)
            sys.exit(1)

