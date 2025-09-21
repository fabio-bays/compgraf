#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <utility>
#include <map>

/*Read a OBJ file and returns a pair of vector and a map.

The vector contains vertexes positions, with the Nth vertex's
position in indexes N-1 (x) and N (y).

The map keys are faces IDs, and their corresponding values are the face vertexes.*/
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