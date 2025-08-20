#include "testa.hpp"
#include "prettyprint.hpp"

#include <functional>
#include <tuple>
#include <optional>
#include <limits>

using namespace std;

void tp0(const string&)
{
    tuple<> tp;
#ifdef ENABLE_STD_FORMAT
    auto res = std::format("{}", tp);
#endif
#ifdef ENABLE_FMTLIB
    auto res = fmt::format("{}", tp);
#endif
    TESTA_ASSERT(res == "()")
        .hint("{}", tp)
        .issue();
}
TESTA_DEF_JUNIT_LIKE2(PP_Tuple0, tp0);

void tp1(const string&)
{
    tuple<string> tp("abc");
#ifdef ENABLE_STD_FORMAT
    auto res = std::format("{}", tp);
#endif
#ifdef ENABLE_FMTLIB
    auto res = fmt::format("{}", tp);
#endif
    TESTA_ASSERT(res == "(abc)")
        .hint("{}", tp)
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
#ifdef ENABLE_STD_FORMAT
    auto res0 = std::format("{}", tp0);
#endif
#ifdef ENABLE_FMTLIB
    auto res0 = fmt::format("{}", tp0);
#endif
    TESTA_ASSERT(res0 == "(-128,-32768,-2147483648,-9223372036854775808)")
        .hint("fp0={}", tp0)
        .issue();

    tuple<int8_t, int16_t, int32_t, int64_t> tp1(
        numeric_limits<int8_t>::max(),
        numeric_limits<int16_t>::max(),
        numeric_limits<int32_t>::max(),
        numeric_limits<int64_t>::max());
#ifdef ENABLE_STD_FORMAT
    auto res1 = std::format("{}", tp1);
#endif
#ifdef ENABLE_FMTLIB
    auto res1 = fmt::format("{}", tp1);
#endif
    TESTA_ASSERT(res1 == "(127,32767,2147483647,9223372036854775807)")
        .hint("fp1={}", tp1)
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
#ifdef ENABLE_STD_FORMAT
    auto res0 = std::format("{}", tp0);
#endif
#ifdef ENABLE_FMTLIB
    auto res0 = fmt::format("{}", tp0);
#endif
    TESTA_ASSERT(res0 == "(0,0,0,0)")
        .hint("tp0={}", tp0)
        .issue();

    tuple<uint8_t, uint16_t, uint32_t, uint64_t> tp1(
        numeric_limits<uint8_t>::max(),
        numeric_limits<uint16_t>::max(),
        numeric_limits<uint32_t>::max(),
        numeric_limits<uint64_t>::max());
#ifdef ENABLE_STD_FORMAT
    auto res1 = std::format("{}", tp1);
#endif
#ifdef ENABLE_FMTLIB
    auto res1 = fmt::format("{}", tp1);
#endif
    TESTA_ASSERT(res1 == "(255,65535,4294967295,18446744073709551615)")
        .hint("tp1={}", tp1)
        .issue();
}
TESTA_DEF_JUNIT_LIKE2(PP_UnsignedIntegers, unsigned_integers);

void booleans(const string&)
{
    tuple<bool, bool> tp(true, false);
#ifdef ENABLE_STD_FORMAT
    auto res = std::format("{}", tp);
#endif
#ifdef ENABLE_FMTLIB
    auto res = fmt::format("{}", tp);
#endif
    TESTA_ASSERT(res == "(true,false)")
        .hint("tp={}", tp)
        .issue();
}
TESTA_DEF_JUNIT_LIKE2(PP_Booleans, booleans);

void optional_none(const string&)
{
    optional<int> none{};
#ifdef ENABLE_STD_FORMAT
    auto res = std::format("{}", none);
#endif
#ifdef ENABLE_FMTLIB
    auto res = fmt::format("{}", none);
#endif
    TESTA_ASSERT(res == "None")
        .hint("{}", none)
        .issue();
}
TESTA_DEF_JUNIT_LIKE2(PP_OptionalNone, optional_none);

void optional_just(const string&)
{
    optional<int> just{1};
#ifdef ENABLE_STD_FORMAT
    auto res = std::format("{}", just);
#endif
#ifdef ENABLE_FMTLIB
    auto res = fmt::format("{}", just);
#endif
    TESTA_ASSERT(res == "Just(1)")
        .hint("{}", just)
        .issue();
}
TESTA_DEF_JUNIT_LIKE2(PP_OptionalJust, optional_just);

namespace {
void reference_wrapper_(const string&)
{
    int x = 0;
    reference_wrapper<int> rx(x);
#ifdef ENABLE_STD_FORMAT
    auto res = std::format("{}", rx);
#endif
#ifdef ENABLE_FMTLIB
    auto res = fmt::format("{}", rx);
#endif
    TESTA_ASSERT(res == "&0")
        .hint("{}", rx)
        .issue();
}
} // namespace
TESTA_DEF_JUNIT_LIKE2(PP_RefWrap, reference_wrapper_);
