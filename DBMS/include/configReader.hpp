#ifndef DBMS_CONFIGREADER_HPP
#define DBMS_CONFIGREADER_HPP

#include <string>
#include <fstream>
#include <sstream>

class Config {
public:
    Config(std::string configFile){
        std::ifstream stream(configFile);
        std::string line;
        while (std::getline(stream, line)){
            std::stringstream s(line);
            std::string var, val;
            std::getline(s, var, '=');
            std::getline(s, val, '=');
            if (var == "failure_rate"){
                failureRate = std::stod(val);
            }
            else if (var == "dbms_save_file"){
                saveFile = val;
            }
            else{
                throw std::runtime_error("unknown parameter");
            }
        }
    }

    double failureRate;
    std::string saveFile;
};
#endif //DBMS_CONFIGREADER_HPP
