#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <utility>
#include <map>
#include <iostream>

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
    while(std::getline(*obj_file, line))
    {
        // Ignora linhas vazias ou com espaços em branco
        if (line.find_first_not_of(" \t\r\n") == std::string::npos) {
            continue;
        }

        std::istringstream line_stream(line);
        line_stream >> word;
        
        if (word.empty()) continue;

        switch(word.at(0))
        {
            case '#':
                continue;
            case 'v':
                // Evitando a coordenada Z.
                for(int i = 0; i < 2; i++){
                    if (line_stream >> word) {
                        vx_pos.push_back(std::stod(word));
                    }
                }
                break;
            case 'f':
                while(line_stream >> word) {
                    // Lida com formatos "f v1/vt1/vn1" pegando apenas o "v1"
                    std::string vertex_index = word.substr(0, word.find('/'));
                    fas_vxs.push_back(std::stoi(vertex_index));
                }
                fas_map[fas_idx++] = fas_vxs; 
                fas_vxs.clear(); // Usar clear() é mais eficiente que recriar o vetor.
                break;
            default:
                // Ignora linhas desconhecidas em vez de encerrar o programa.
                break;
        }
    }
    return std::make_pair(vx_pos, fas_map);
}

/**
 * @brief (Implementação) Lê um arquivo .obj e popula as estruturas de dados.
 * Esta é a função que o main.cpp irá chamar.
 */
bool read_obj_file(const std::string& filepath, std::vector<double>& vxs_pos, std::map<int, std::vector<int>>& fa_vxs)
{
    std::ifstream file(filepath);

    if (!file.is_open()) {
        std::cerr << "Erro: Nao foi possivel abrir o arquivo: " << filepath << std::endl;
        return false;
    }

    // Chama a função auxiliar para processar o arquivo.
    std::pair<std::vector<double>, std::map<int, std::vector<int>>> data = data_from_obj(&file);

    // Popula as variáveis passadas por referência com os dados lidos.
    vxs_pos = data.first;
    fa_vxs = data.second;

    file.close();
    return true;
}

// A função 'main' foi removida daqui. O único 'main' agora está em main.cpp.
