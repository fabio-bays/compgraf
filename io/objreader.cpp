#include <fstream>
#include <sstream>
#include <string>

// read obj and save data
{
    std::string line;
    double x, y;
    char c;
    while(std::getline(obj_file, line))
    {
        std::istringstream line_stream(line);
        line_stream >> c;
        if (c == 'v')
        {

        }
        else if (c == 'f')
        {

        }

    }
}