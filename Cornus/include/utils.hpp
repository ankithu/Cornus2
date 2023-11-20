#ifndef CORNUS_UTILS_HPP
#define CORNUS_UTILS_HPP

#include <future>

template <typename T>
void resolveFutures(std::vector<std::future<T>>& futs){
    for (auto& f : futs){
        f.get();
    }
}

#endif //CORNUS_UTILS_HPP
