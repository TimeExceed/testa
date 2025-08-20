#pragma once

#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <tuple>

#ifdef ENABLE_STD_FORMAT
#include <format>
#endif
#ifdef ENABLE_FMTLIB
#include <fmt/format.h>
#endif

#define TESTA_ASSERT(cond) \
    if (cond) {\
    } else ::std::move(::testa::_impl::CaseFailIssuer(#cond, __FILE__, __LINE__))

namespace testa::_impl {

using CaseMap = ::std::map<::std::string, ::std::function<void()>>;

::std::shared_ptr<CaseMap> get_case_map();

class CaseFailIssuer {
public:
    explicit CaseFailIssuer(const char* cond, const char* fn, int line);
    ~CaseFailIssuer();

    CaseFailIssuer(const CaseFailIssuer&) = delete;
    CaseFailIssuer& operator=(const CaseFailIssuer&) = delete;
    CaseFailIssuer(CaseFailIssuer&&) = default;
    CaseFailIssuer& operator=(CaseFailIssuer&&) = default;

    template<class... Args>
    requires (sizeof...(Args) > 0)
    CaseFailIssuer hint(::std::string_view fmt, Args&&... args) && {
#ifdef ENABLE_STD_FORMAT
        auto m = ::std::vformat(fmt, ::std::make_format_args(args...));
#endif
#ifdef ENABLE_FMTLIB
        auto m = ::fmt::vformat(fmt, ::fmt::make_format_args(args...));
#endif
        _hints.push_back(::std::move(m));
        return ::std::move(*this);
    }

    void issue(const ::std::string_view& msg) &&;
    void issue() &&;

private:
    ::std::string_view _condition;
    ::std::string_view _filename;
    int _line;
    ::std::deque<::std::string> _hints;
    bool _issued;
};

class EqCase {
public:
    template<class Res, class T>
    EqCase(
        const ::std::string& caseName,
        void (*tb)(const ::std::string&, ::std::function<void(const T&)>),
        Res (*trialFn)(const T&),
        Res (*oracleFn)(const T&)
    ) {
        ::std::function<void(T)> cs = ::std::bind(
            &EqCase::eq<Res, T>,
            trialFn,
            oracleFn,
            ::std::placeholders::_1);
        (*testa::_impl::get_case_map())[caseName] = ::std::bind(tb, caseName, cs);
    }

    template<class Res, class T>
    EqCase(
        const std::string& caseName,
        Res (*trialFn)(const T&),
        Res (*oracleFn)(const T&),
        const T& in
    ) {
        ::std::function<void()> cs = ::std::bind(
            &EqCase::eq<Res, T>,
            trialFn,
            oracleFn,
            in);
        (*testa::_impl::get_case_map())[caseName] = cs;
    }

    template<class Res, class T0, class T1>
    EqCase(
        const ::std::string& caseName,
        Res (*trialFn)(const ::std::tuple<T0, T1>&),
        Res (*oracleFn)(const ::std::tuple<T0, T1>&),
        const T0& in0,
        const T1& in1
    ) {
        ::std::function<void()> cs = ::std::bind(
            &EqCase::eq<Res, ::std::tuple<T0, T1> >,
            trialFn,
            oracleFn,
            ::std::make_tuple(in0, in1));
        (*testa::_impl::get_case_map())[caseName] = cs;
    }

    template<class Res, class T0, class T1, class T2>
    EqCase(
        const ::std::string& caseName,
        Res (*trialFn)(const ::std::tuple<T0, T1, T2>&),
        Res (*oracleFn)(const ::std::tuple<T0, T1, T2>&),
        const T0& in0,
        const T1& in1,
        const T2& in2
    ) {
        ::std::function<void()> cs = ::std::bind(
            &EqCase::eq<Res, ::std::tuple<T0, T1, T2> >,
            trialFn,
            oracleFn,
            ::std::make_tuple(in0, in1, in2));
        (*testa::_impl::get_case_map())[caseName] = cs;
    }

private:
    template<class Res, class T>
    static void eq(
        Res (*trialFn)(const T&),
        Res (*oracleFn)(const T&),
        const T& in
    ) {
        const Res& trialResult = trialFn(in);
        const Res& oracleResult = oracleFn(in);
        TESTA_ASSERT(trialResult == oracleResult)
            .hint("input={}", in)
            .hint("trial result={}", trialResult)
            .hint("oracle result={}", oracleResult)
            .issue();
    }
};

class VerifyCase {
public:
    template<class Res, class T>
    VerifyCase(
        const ::std::string& caseName,
        void (*tb)(const ::std::string&, ::std::function<void(const T&)>),
        void (*verifier)(const Res&, const T&),
        Res (*trialFn)(const T&)
    ) {
        ::std::function<void(T)> cs = ::std::bind(
            &VerifyCase::verify<Res, T>,
            verifier,
            trialFn,
            ::std::placeholders::_1);
        (*testa::_impl::get_case_map())[caseName] = ::std::bind(tb, caseName, cs);
    }

    VerifyCase(
        const ::std::string& caseName,
        void (*tbVerifier)(const std::string&)
    ) {
        (*testa::_impl::get_case_map())[caseName] = ::std::bind(tbVerifier, caseName);
    }

private:
    template<class Res, class T>
    static void verify(
        void (*verifier)(const Res&, const T&),
        Res (*trialFn)(const T&),
        const T& in
    ) {
        const Res& res = trialFn(in);
        verifier(res, in);
    }
};

}


#define TESTA_DEF_EQ_WITH_TB(caseName, caseTb, trialFn, oracleFn) \
    testa::_impl::EqCase cs##caseName(#caseName, (caseTb), (trialFn), (oracleFn))

#define TESTA_DEF_EQ_1(caseName, trialFn, oracleFn, in0)                 \
    testa::_impl::EqCase cs##caseName(#caseName, (trialFn), (oracleFn), (in0))

#define TESTA_DEF_EQ_2(caseName, trialFn, oracleFn, in0, in1)        \
    testa::_impl::EqCase cs##caseName(#caseName, (trialFn), (oracleFn), (in0), (in1))

#define TESTA_DEF_EQ_3(caseName, trialFn, oracleFn, in0, in1, in2)       \
    testa::_impl::EqCase cs##caseName(#caseName, (trialFn), (oracleFn), \
        (in0), (in1), (in2))

#define TESTA_DEF_VERIFY_WITH_TB(caseName, caseTb, caseVerfier, trialFn) \
    testa::_impl::VerifyCase cs##caseName(#caseName, (caseTb), (caseVerfier), (trialFn))

#define TESTA_DEF_JUNIT_LIKE2(caseName, tbVerifier) \
    testa::_impl::VerifyCase cs##caseName(#caseName, tbVerifier)

#define TESTA_DEF_JUNIT_LIKE1(caseName) \
    TESTA_DEF_JUNIT_LIKE2(caseName, caseName)

