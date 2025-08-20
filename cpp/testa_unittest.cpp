#include "testa.hpp"
#include "prettyprint.hpp"
#include <tuple>
#include <string>
#include <functional>

using namespace std;

void junit_like1_correct(const string&) {
    int x = 0;
    TESTA_ASSERT(x == 0)
        .hint("x: {}", x)
        .issue();
}
TESTA_DEF_JUNIT_LIKE1(junit_like1_correct);

void junit_like1_wrong(const string&) {
    int x = 0;
    TESTA_ASSERT(x != 0)
        .hint("x: {}", x)
        .issue();
}
TESTA_DEF_JUNIT_LIKE1(junit_like1_wrong);

void junit_like2_correct_orig(const string&) {
    int x = 0;
    TESTA_ASSERT(x == 0)
        .hint("x: {}", x)
        .issue();
}
TESTA_DEF_JUNIT_LIKE2(junit_like2_correct, junit_like2_correct_orig);

void junit_like2_wrong_orig(const string&) {
    int x = 0;
    TESTA_ASSERT(x != 0)
        .hint("x: {}", x)
        .issue();
}
TESTA_DEF_JUNIT_LIKE2(junit_like2_wrong, junit_like2_wrong_orig);

int add_trial(const tuple<int, int>& in)
{
    return get<0>(in) + get<1>(in);
}

int multiply_trial(const tuple<int, int>& in)
{
    return get<0>(in) * get<1>(in);
}

int multiply_oracle(const tuple<int, int>& in)
{
    int r = 0;
    for(int i = 0; i < get<1>(in); ++i) {
        r += get<0>(in);
    }
    return r;
}

void permutation_tb(const string& name, function<void(const tuple<int, int>&)> cs)
{
    for(int i = 1; i <= 5; ++i) {
        for(int j = 1; j <= 5; ++j) {
            cs(make_tuple(i, j));
        }
    }
}

TESTA_DEF_EQ_WITH_TB(CorrectMultiple, permutation_tb, multiply_trial, multiply_oracle);
TESTA_DEF_EQ_WITH_TB(WrongMultiple, permutation_tb, add_trial, multiply_oracle);


int gcd(const tuple<int, int>& in)
{
    int a = get<0>(in);
    int b = get<1>(in);
    if (a == 0) {
        return b;
    } else {
        return gcd(make_tuple(b % a, a));
    }
}

void gcd_tb(const string& name, function<void(const tuple<int, int>&)> cs)
{
    for(int i = 0; i <= 12; ++i) {
        for(int j = 0; j <= 12; ++j) {
            if (i != 0 || j != 0) {
                cs(make_tuple(i, j));
            }
        }
    }
}

void gcd_verifier(const int& res, const tuple<int, int>& in)
{
    TESTA_ASSERT(get<0>(in) % res == 0 && get<1>(in) % res == 0)
        .hint("res={}", res)
        .hint("in=({})", in)
        .issue();
}

void gcd_wrong_verifier(const int& res, const tuple<int, int>& in)
{
    TESTA_ASSERT(get<0>(in) % res != 0)
        .hint("res={}", res)
        .hint("in=({})", in)
        .issue();
}

TESTA_DEF_VERIFY_WITH_TB(CorrectGcd, gcd_tb, gcd_verifier, gcd);
TESTA_DEF_VERIFY_WITH_TB(WrongGcd, gcd_tb, gcd_wrong_verifier, gcd);

int trial1(const int& x)
{
    return x;
}

TESTA_DEF_EQ_1(Eq1, trial1, trial1, 0);
TESTA_DEF_EQ_2(WrongEq2, add_trial, multiply_trial, 1, 2);

int trial3(const tuple<int, int, int>& in)
{
    return get<0>(in) + get<1>(in) + get<2>(in);
}

TESTA_DEF_EQ_3(Eq3, trial3, trial3, 1, 2, 3);

