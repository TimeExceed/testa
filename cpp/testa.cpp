#include "testa.hpp"
#include <iterator>
#include <stdexcept>
#include <string_view>
#include <cstdlib>
#include <cstdio>

using namespace std;

#ifdef ENABLE_STD_FORMAT
using std::format_to;
#endif
#ifdef ENABLE_FMTLIB
using fmt::format_to;
#endif

namespace testa::_impl {

shared_ptr<CaseMap> get_case_map()
{
    static shared_ptr<CaseMap> result(new CaseMap());
    return result;
}

CaseFailIssuer::CaseFailIssuer(const char* cond, const char* fn, int line)
:   _condition(cond),
    _filename(fn),
    _line(line),
    _issued(false)
{}

CaseFailIssuer::~CaseFailIssuer()
{
    if (!_issued) {
        fprintf(stderr, "TESTA_ASSERT @ %s:%d does not invoke issue()\n",
            _filename.data(), _line);
        abort();
    }
}

void CaseFailIssuer::issue() &&
{
    std::move(*this).issue(string());
}

void CaseFailIssuer::issue(const string_view& msg) &&
{
    _issued = true;
    string full_msg;
    auto it = std::back_inserter(full_msg);
    it = format_to(it, "Assertion @ {}:{} fail: {}\n",
        _filename, _line, _condition);
    if (!msg.empty()) {
        it = format_to(it, "Message: {}\n", msg);
    }
    auto h_it = _hints.begin();
    if (h_it != _hints.end()) {
        it = format_to(it, "Hints: {}\n", *h_it);
        ++h_it;
    }
    for (; h_it != _hints.end(); ++h_it) {
        it = format_to(it, "       {}\n", *h_it);
    }
    throw std::logic_error(full_msg);
}

}
