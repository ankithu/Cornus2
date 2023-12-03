#ifndef CORNUS_TIMER_HPP
#define CORNUS_TIMER_HPP

#include <vector>
#include <chrono>
#include <string>
#include <ostream>

class EventTimer {
public:
    inline void record(std::string&& event){
        events.push_back({std::chrono::high_resolution_clock::now(), std::move(event)});
    }

    inline void dump(std::ostream& os){
        os << "EventSummary: {";
        for (size_t i = 1; i < events.size(); ++i){
            auto& [t_e, e_e] = events[i];
            auto& [t_s, e_s] = events[i - 1];
            auto diff = duration_cast<std::chrono::duration<double>>(t_e - t_s);
            os << "{" << e_s << " to " << e_e << "=" << diff.count() << "}";
        }
        os << "}";
    }

private:
    struct TimeEvent {
        std::chrono::time_point<std::chrono::high_resolution_clock> time;
        std::string event;
    };

    std::vector<TimeEvent> events;
};


#endif //CORNUS_TIMER_HPP
