#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <utility>
#include <map>
#include <iostream>
#include "../dsa/TwoDHalfEdgeGeometry.h"

// read obj and save data
std::pair<std::vector<double>, std::map<int, std::vector<int>>>
data_from_obj(std::ifstream *obj_file)
{
    std::vector<double> vx_pos;
    std::map<int, std::vector<int>> fas_map;
    std::vector<int> fas_vxs;
    int fas_idx = 1;
    

    std::string line, word;
    double x, y;
    while(std::getline(*obj_file, line))
    {
        std::istringstream line_stream(line);
        line_stream >> word;
        // std::cout << word << std::endl;
        // std::cout << word[0] << std::endl;
        switch(word.at(0))
        {
            case '#':
                continue;
            case 'v':
                // Avoiding the Z position in vertexes positions.
                for(int i = 0; i < 2; i++){
                    line_stream >> word;
                    vx_pos.push_back(std::stod(word));
                }
                break;
            case 'f':
                while(line_stream >> word)
                    fas_vxs.push_back(std::stoi(word));
                fas_map[fas_idx++] = fas_vxs; 
                fas_vxs = {};
                break;
            default:
                exit(-1);
        }
    }
    return std::make_pair(vx_pos, fas_map);
}

int main(){
    std::ifstream file("./io/.testobj.obj");

    if(!file) exit(-1);

    std::pair<std::vector<double>, std::map<int, std::vector<int>>> p = data_from_obj(&file);

    // for(auto itr : p.first)
    // {
    //     std:: cout << itr << std::endl;
    // }
    // std::cout << "Faces:" << std::endl;
    // for(auto itr : p.second)
    // {
    //     std:: cout << itr.first << std::endl;
    // }
    
    
    TwoDHalfEdgeGeometry geo(p.first, p.second);
    std::map<std::pair<unsigned int, unsigned int>, unsigned int> conn = geo.get_vxs_conn_edges_id();

    for(auto itr : conn)
    {
        std::cout << "(" << itr.first.first + 1<< ", " << itr.first.second + 1<< ") : " << itr.second<< std::endl;
    }


    return 0;
}