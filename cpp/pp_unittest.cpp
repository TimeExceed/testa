#include "testa.hpp"
#include "prettyprint.hpp"

#if __cplusplus < 201103L
#include <tr1/tuple>
#else
#include <tuple>
#endif
#if __cplusplus >= 201703L
#include <optional>
#endif

#include <limits>
#include <cstdio>

using namespace std;
#if __cplusplus < 201103L
using namespace std::tr1;
#endif

void junit_style_deque(const string&)
{
    deque<int64_t> xs;
    xs.push_back(1);
    xs.push_back(2);
    xs.push_back(3);
    string res = pp::prettyPrint(xs);
    TESTA_ASSERT(res == "[1,2,3]")
        (res)
        .issue();
}
TESTA_DEF_JUNIT_LIKE2(PP_JunitStyleDeque, junit_style_deque);

void junit_style_map(const string&)
{
    map<string, int64_t> xs;
    xs.insert(make_pair("a", 1));
    xs.insert(make_pair("b", 2));
    xs.insert(make_pair("c", 3));
    string res = pp::prettyPrint(xs);
    TESTA_ASSERT(res == "{\"a\":1,\"b\":2,\"c\":3}")
        (res)
        .issue();
}
TESTA_DEF_JUNIT_LIKE2(PP_JunitStyleMap, junit_style_map);

void tp0(const string&)
{
    tuple<> tp;
    string res = pp::prettyPrint(tp);
    TESTA_ASSERT(res == "()")
        (res)
        .issue();
}
TESTA_DEF_JUNIT_LIKE2(PP_Tuple0, tp0);

void tp1(const string&)
{
    tuple<string> tp("abc");
    string res = pp::prettyPrint(tp);
    TESTA_ASSERT(res == "(\"abc\")")
        (res)
        .issue();
}
TESTA_DEF_JUNIT_LIKE2(PP_Tuple1, tp1);

void signed_integers(const string&)
{
    tuple<int8_t, int16_t, int32_t, int64_t> tp0(
        numeric_limits<int8_t>::min(),
        numeric_limits<int16_t>::min(),
        numeric_limits<int32_t>::min(),
        numeric_limits<int64_t>::min());
    string min = pp::prettyPrint(tp0);
    TESTA_ASSERT(min == "(-128,-32768,-2147483648,-9223372036854775808)")
        (min)
        .issue();

    tuple<int8_t, int16_t, int32_t, int64_t> tp1(
        numeric_limits<int8_t>::max(),
        numeric_limits<int16_t>::max(),
        numeric_limits<int32_t>::max(),
        numeric_limits<int64_t>::max());
    string max = pp::prettyPrint(tp1);
    TESTA_ASSERT(max == "(127,32767,2147483647,9223372036854775807)")
        (max)
        .issue();
}
TESTA_DEF_JUNIT_LIKE2(PP_SignedIntegers, signed_integers);

void unsigned_integers(const string&)
{
    tuple<uint8_t, uint16_t, uint32_t, uint64_t> tp0(
        numeric_limits<uint8_t>::min(),
        numeric_limits<uint16_t>::min(),
        numeric_limits<uint32_t>::min(),
        numeric_limits<uint64_t>::min());
    string min = pp::prettyPrint(tp0);
    TESTA_ASSERT(min == "(0,0,0,0)")
        (min)
        .issue();

    tuple<uint8_t, uint16_t, uint32_t, uint64_t> tp1(
        numeric_limits<uint8_t>::max(),
        numeric_limits<uint16_t>::max(),
        numeric_limits<uint32_t>::max(),
        numeric_limits<uint64_t>::max());
    string max = pp::prettyPrint(tp1);
    TESTA_ASSERT(max == "(255,65535,4294967295,18446744073709551615)")
        (max)
        .issue();
}
TESTA_DEF_JUNIT_LIKE2(PP_UnsignedIntegers, unsigned_integers);

void booleans(const string&)
{
    tuple<bool, bool> tp(true, false);
    string res = pp::prettyPrint(tp);
    TESTA_ASSERT(res == "(true,false)")
        (res)
        .issue();
}
TESTA_DEF_JUNIT_LIKE2(PP_Booleans, booleans);

void characters(const string&)
{
    tuple<char, char, char> tp('\n', 'A', 130);
    string res = pp::prettyPrint(tp);
    TESTA_ASSERT(res == "('\\x0A','A','\\x82')")
        (res)
        .issue();
}
TESTA_DEF_JUNIT_LIKE2(PP_Characters, characters);

namespace {
void fpnumber(const string&)
{
    tuple<float, double> tp(1.0, 2.0);
    string res = pp::prettyPrint(tp);
    TESTA_ASSERT(res == "(1.0000,2.0000)")
        (res)
        .issue();
}
} // namespace
TESTA_DEF_JUNIT_LIKE2(PP_FloatPointNumber, fpnumber);

void literal_str(const string&)
{
    string res = pp::prettyPrint("abc");
    TESTA_ASSERT(res == "abc")
        (res)
        .issue();
}
TESTA_DEF_JUNIT_LIKE2(PP_LiteralStr, literal_str);

#if __cplusplus >= 201703L
void optional_none(const string&)
{
    optional<int> none{};
    string res = pp::prettyPrint(none);
    TESTA_ASSERT(res == "None")
        (res)
        .issue();
}
TESTA_DEF_JUNIT_LIKE2(PP_OptionalNone, optional_none);

void optional_just(const string&)
{
    optional<int> just{1};
    string res = pp::prettyPrint(just);
    TESTA_ASSERT(res == "Just(1)")
        (res)
        .issue();
}
TESTA_DEF_JUNIT_LIKE2(PP_OptionalJust, optional_just);
#endif

void complex_positive_imag(const string&)
{
    complex<double> c(1, 2);
    string res = pp::prettyPrint(c);
    TESTA_ASSERT(res == "1.0000+2.0000i")
        (res)
        .issue();
}
TESTA_DEF_JUNIT_LIKE2(PP_ComplexPositiveImag, complex_positive_imag);

void complex_negative_imag(const string&)
{
    complex<double> c(1, -2);
    string res = pp::prettyPrint(c);
    TESTA_ASSERT(res == "1.0000-2.0000i")
        (res)
        .issue();
}
TESTA_DEF_JUNIT_LIKE2(PP_ComplexNegativeImag, complex_negative_imag);

void complex_real_zero(const string&)
{
    complex<double> c;
    string res = pp::prettyPrint(c);
    TESTA_ASSERT(res == "0.0000+0.0000i")
        (res)
        .issue();
}
TESTA_DEF_JUNIT_LIKE2(PP_ComplexRealZero, complex_real_zero);
