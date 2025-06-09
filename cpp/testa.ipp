#pragma once
/*
This file is picked from project testa [https://github.com/TimeExceed/testa.git]
Copyright (c) 2017, Taoda (tyf00@aliyun.com)
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice, this
  list of conditions and the following disclaimer in the documentation and/or
  other materials provided with the distribution.

* Neither the name of the {organization} nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <string_view>
#include <memory>
#include <functional>
#include <tuple>
#include <deque>
#include <map>
#include <string>

#ifdef ENABLE_STD_FORMAT
#include <format>
#endif
#ifdef ENABLE_FMTLIB
#include <fmt/core.h>
#endif

namespace testa {

using CaseMap = ::std::map<::std::string, ::std::function<void()> >;

::std::shared_ptr<CaseMap> getCaseMap();

class CaseFailIssuer
{
public:
    explicit CaseFailIssuer(bool disable, const char* cond, const char* fn, int line);
    ~CaseFailIssuer();

    template<class... Args>
    requires (sizeof...(Args) > 0)
    CaseFailIssuer& append(::std::string_view fmt, Args&&... args) {
#ifdef ENABLE_STD_FORMAT
        mHints.push_back(::std::vformat(fmt, ::std::make_format_args(args...)));
#endif
#ifdef ENABLE_FMTLIB
        mHints.push_back(::fmt::vformat(fmt, ::fmt::make_format_args(args...)));
#endif
        return *this;
    }

    CaseFailIssuer& append(::std::string hint) {
        mHints.push_back(::std::move(hint));
        return *this;
    }

    void issue(const ::std::string_view& msg);
    void issue();

private:
    bool mTrigger;
    ::std::string mCondition;
    ::std::string mFilename;
    int mLine;
    ::std::deque<::std::string> mHints;
    bool mIssued;

public:
    CaseFailIssuer& TESTA_PINGPONG_A;
    CaseFailIssuer& TESTA_PINGPONG_B;
};

class EqCase
{
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
        (*testa::getCaseMap())[caseName] = ::std::bind(tb, caseName, cs);
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
        (*testa::getCaseMap())[caseName] = cs;
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
        (*testa::getCaseMap())[caseName] = cs;
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
        (*testa::getCaseMap())[caseName] = cs;
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
        CaseFailIssuer issr(trialResult == oracleResult,
            "trial result == oracle result",
            __FILE__, __LINE__);
        issr.append("input={}", in)
            .append("trial result={}", trialResult)
            .append("oracle result={}", oracleResult)
            .issue();
    }
};

class VerifyCase
{
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
        (*testa::getCaseMap())[caseName] = ::std::bind(tb, caseName, cs);
    }

    VerifyCase(
        const ::std::string& caseName,
        void (*tbVerifier)(const std::string&)
    ) {
        (*testa::getCaseMap())[caseName] = ::std::bind(tbVerifier, caseName);
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

} // namespace testa

