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

#include "prettyprint.hpp"
#include <deque>
#include <cmath>

using namespace std;
#if __cplusplus < 201103L
using namespace std::tr1;
#endif


namespace pp {

FloatPointPrettyPrinter::FloatPointPrettyPrinter(double v, size_t prec)
:   mValue(v), mPrec(prec)
{}

void FloatPointPrettyPrinter::prettyPrint(string& out) const
{
    if (isnan(mValue)) {
        out.append("NaN");
        return;
    }

    if (isinf(mValue)) {
        if (mValue > 0) {
            out.push_back('+');
        } else {
            out.push_back('-');
        }
        out.append("inf");
        return;
    }

    double x = mValue;
    if (x < 0) {
        x = -x;
        out.push_back('-');
    }
    deque<size_t> digits;
    if (x < 1) {
        digits.push_back(0);
    } else {
        size_t integral = static_cast<size_t>(x);
        digits.push_back(integral);
        x -= integral;
    }
    for(size_t i = 0; i < mPrec + 1; ++i) {
        x *= 10;
        size_t integral = static_cast<size_t>(x);
        digits.push_back(integral);
        x -= integral;
    }
    {
        size_t carriage = digits.back();
        digits.pop_back();
        if (carriage >= 5) {
            for(size_t n = digits.size() - 1; n > 0; --n) {
                digits[n] += 1;
                if (digits[n] < 10) {
                    break;
                }
                digits[n] -= 10;
            }
        }
    }

    pp::prettyPrint(out, digits.front());
    out.push_back('.');
    for(digits.pop_front(); !digits.empty(); digits.pop_front()) {
        pp::prettyPrint(out, digits.front());
    }
}

} // namespace pp

