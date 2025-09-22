/**
 * @file main.cpp
 * @brief Ponto de entrada principal do programa de geometria 2D.
 *
 * Responsável por:
 * 1. Processar argumentos para obter o caminho do arquivo .obj.
 * 2. Chamar o leitor de .obj.
 * 3. Construir a estrutura de dados Half-Edge.
 * 4. Iniciar o renderizador, que gerencia toda a interatividade.
 */

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "dsa/TwoDHalfEdgeGeometry.h"

// Protótipos de Funções
bool read_obj_file(const std::string& filepath, std::vector<double>& vxs_pos, std::map<int, std::vector<int>>& fa_vxs);
void start_renderer(int argc, char* argv[], TwoDHalfEdgeGeometry& geometry);


int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Uso: " << argv[0] << " <caminho_para_o_arquivo.obj>" << std::endl;
        return 1;
    }
    std::string obj_filepath = argv[1];

    std::vector<double> vertex_positions;
    std::map<int, std::vector<int>> face_vertices;

    std::cout << "Lendo o arquivo: " << obj_filepath << "..." << std::endl;
    if (!read_obj_file(obj_filepath, vertex_positions, face_vertices)) {
        std::cerr << "Erro fatal: Nao foi possivel ler o arquivo .obj." << std::endl;
        return 1;
    }
    std::cout << "Arquivo lido com sucesso." << std::endl;

    std::cout << "Construindo a estrutura de dados Half-Edge..." << std::endl;
    try {
        TwoDHalfEdgeGeometry geometry(vertex_positions, face_vertices);
        std::cout << "Estrutura de dados construida com sucesso!" << std::endl;
        std::cout << "\nIniciando a janela grafica..." << std::endl;
        
        start_renderer(argc, argv, geometry);

    } catch (const std::exception& e) {
        std::cerr << "Um erro ocorreu durante a construcao da geometria: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "\nPrograma encerrado." << std::endl;
    return 0;
}

