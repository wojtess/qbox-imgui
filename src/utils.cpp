#include "utils.h"
#include <iostream>
#include <vector>

GLFWmonitor* getMonitor(int i) {
    int count;
    GLFWmonitor** monitors = glfwGetMonitors(&count);
    if(count >= i && monitors != 0) {
        return monitors[i - 1];
    }
    return 0;
}

std::vector<std::string> split(const std::string& s, char seperator) {
   std::vector<std::string> output;

    std::string::size_type prev_pos = 0, pos = 0;

    while((pos = s.find(seperator, pos)) != std::string::npos)
    {
        std::string substring( s.substr(prev_pos, pos-prev_pos) );

        output.push_back(substring);

        prev_pos = ++pos;
    }

    output.push_back(s.substr(prev_pos, pos-prev_pos)); // Last word

    return output;
}