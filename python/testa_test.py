import testa

@testa.is_(expect=3)
def correct_addition():
    return 1 + 2

@testa.is_(expect=3)
def wrong_addition():
    return 2 + 2

@testa.broken(reason = 'this is a broken case.')
@testa.is_(expect=3)
def broken_addition():
    return 2 + 2

def multiple_tb(case_f):
    for i in range(5):
        for j in range(5):
            res = case_f(i, j)
            if res is not None:
                return res

def multiple_oracle(x, y):
    return sum(x for _ in range(y))

@testa.eq(testbench=multiple_tb, oracle=multiple_oracle)
def correct_multiple(x, y):
    return x * y

@testa.eq(testbench=multiple_tb, oracle=multiple_oracle)
def wrong_multiple(x, y):
    return x + y

@testa.broken(reason = 'this is a broken case.')
@testa.eq(testbench=multiple_tb, oracle=multiple_oracle)
def broken_multiple(x, y):
    return x + y


def gcd_tb(case_f):
    for a in range(13):
        for b in range(13):
            if a == 0 and b == 0:
                continue
            result = case_f(a, b)
            if result is not None:
                return result

def gcd(a, b):
    if a == 0:
        return b
    else:
        return gcd(b % a, a)

@testa.verify(testbench=gcd_tb, trial=gcd)
def correct_verifer(result, a, b):
    if a == 0 and b == 0:
        return 'undefined on (0, 0)'
    if a % result != 0:
        return '{} can not divided by {}'.format(a, result)
    if b % result != 0:
        return '{} can not divided by {}'.format(b, result)

@testa.verify(testbench=gcd_tb, trial=gcd)
def wrong_verifier(result, a, b):
    if a == 0 and b == 0:
        return 'undefined on (0, 0)'
    if a == 1:
        return 'fake error on a=1'

@testa.broken(reason = 'this is a broken case.')
@testa.verify(testbench=gcd_tb, trial=gcd)
def broken_verifier(result, a, b):
    if a == 0 and b == 0:
        return 'undefined on (0, 0)'
    if a == 1:
        return 'fake error on a=1'


@testa.throw(throw=KeyError)
def throw_correct():
    empty = {}
    empty['xxx']

@testa.throw(throw=IOError)
def throw_wrong():
    empty = {}
    empty['xxx']

@testa.broken(reason = 'this is a broken case.')
@testa.throw(throw=IOError)
def broken_throw():
    empty = {}
    empty['xxx']

if __name__ == '__main__':
    testa.main()
