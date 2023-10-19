#ifndef CORNUS_MESSAGEHANDLER_HPP
#define CORNUS_MESSAGEHANDLER_HPP

#include "httplib.hpp"

//onRequest function, in class or in global namespace, takes HTTP request object

class GlobalMessageHandler {
public:
    void onRequest(httplib::Request& request){

    }
};

#endif //CORNUS_MESSAGEHANDLER_HPP
