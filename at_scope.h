#pragma once
#include <boost/preprocessor/cat.hpp>

template<class Lambda>
class AtScopeExit {
    Lambda& lambda;
public:
    AtScopeExit(Lambda& action) : lambda(action) {}
    ~AtScopeExit() { lambda(); }
};


#define AT_SCOPE_IMPL1(lname, aname, ...) \
    auto lname = [&] { __VA_ARGS__; }; \
    AtScopeExit<decltype(lname)> aname(lname);

#define AT_SCOPE_IMPL2(ctr, ...) \
    AT_SCOPE_IMPL1(BOOST_PP_CAT(_scp_func_, ctr), BOOST_PP_CAT(_scp_inst_, ctr), __VA_ARGS__)

#define AT_SCOPE(...) AT_SCOPE_IMPL2(__LINE__, __VA_ARGS__)
