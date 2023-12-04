#ifndef CORNUS_UTILS_HPP
#define CORNUS_UTILS_HPP

#include <future>

std::mutex global_cout_mutex;

template <typename T>
void resolveFutures(std::vector<std::future<T>>& futs){
    for (auto& f : futs){
        f.get();
    }
}

#include <type_traits>
#include <typeinfo>
#ifndef _MSC_VER
#   include <cxxabi.h>
#endif
#include <memory>
#include <string>
#include <cstdlib>

//taken from:
//https://stackoverflow.com/questions/81870/is-it-possible-to-print-a-variables-type-in-standard-c
template <class T>
std::string
type_name()
{
    typedef typename std::remove_reference<T>::type TR;
    std::unique_ptr<char, void(*)(void*)> own
            (
#ifndef _MSC_VER
            abi::__cxa_demangle(typeid(TR).name(), nullptr,
                                nullptr, nullptr),
#else
            nullptr,
#endif
            std::free
    );
    std::string r = own != nullptr ? own.get() : typeid(TR).name();
    if (std::is_const<TR>::value)
        r += " const";
    if (std::is_volatile<TR>::value)
        r += " volatile";
    if (std::is_lvalue_reference<T>::value)
        r += "&";
    else if (std::is_rvalue_reference<T>::value)
        r += "&&";
    return r;
}


#endif //CORNUS_UTILS_HPP
