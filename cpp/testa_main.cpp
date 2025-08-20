#include "testa.hpp"
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <cstdio>

using namespace std;

namespace {

void print_usage(string exe)
{
    printf("%s [--help|-h] [--show-cases] [CASENAME]\n", exe.c_str());
    printf("CASENAME\ta case name that will be executed\n");
    printf("--show-cases\ta list of case names, one name per line\n");
    printf("--help,-h\tthis help message\n");
}

}

int main(int argv, char** args)
{
    if (argv < 1) {
        abort();
    }
    string exe = args[0];
    if (argv != 2) {
        print_usage(exe);
        return 1;
    }
    string act = args[1];
    if (act == "--help" || act == "-h") {
        print_usage(exe);
        return 0;
    } else if (act == "--show-cases") {
        shared_ptr<testa::_impl::CaseMap> cases = testa::_impl::get_case_map();
        printf("[\n");
        auto case_it = cases->begin();
        if (case_it != cases->end()) {
            printf("{\"name\":\"%s\"}", case_it->first.c_str());
            ++case_it;
        }
        for (; case_it != cases->end(); ++case_it) {
            printf(",\n");
            printf("{\"name\":\"%s\"}", case_it->first.c_str());
        }
        printf("]\n");
        return 0;
    } else {
        shared_ptr<testa::_impl::CaseMap> cases = testa::_impl::get_case_map();
        auto cit = cases->find(act);
        if (cit == cases->end()) {
            abort();
        }
        cit->second();
        return 0;
    }
}
