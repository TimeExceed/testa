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

#ifndef TABLESTORE_UTIL_PRETTYPRINT_HPP
#define TABLESTORE_UTIL_PRETTYPRINT_HPP

#include "metaprogramming.hpp"
#include <string>

namespace pp {

namespace impl {

template<class T, class E=void>
struct PrettyPrinter;

} // namespace impl

template<class T>
void prettyPrint(std::string& out, const T& x)
{
    impl::PrettyPrinter<typename mp::RemoveCvref<T>::Type> p;
    p(out, x);
}

template<class T>
std::string prettyPrint(const T& x)
{
    std::string res;
    prettyPrint<T>(res, x);
    return res;
}

struct FloatPointPrettyPrinter
{
    explicit FloatPointPrettyPrinter(double v, size_t prec = 4);
    void prettyPrint(std::string&) const;

private:
    double mValue;
    size_t mPrec;
};

} // namespace pp

#include "prettyprint.ipp"

#endif
