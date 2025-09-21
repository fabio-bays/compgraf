/**
 * @file main.cpp
 * @brief Ponto de entrada principal do programa de geometria 2D.
 *
 * Responsável por:
 * 1. Processar argumentos de linha de comando para obter o caminho do arquivo .obj.
 * 2. Chamar o leitor de .obj para carregar os dados do modelo.
 * 3. Instanciar e construir a estrutura de dados Half-Edge.
 * 4. Iniciar a interface de prompt de comando para o usuário.
 * 5. Iniciar o renderer do OpenGL para visualizar o objeto.
 */

#include <iostream>
#include <string>
#include <vector>
#include <map>

// Inclui o cabeçalho da classe principal da estrutura de dados.
#include "dsa/TwoDHalfEdgeGeometry.h"

// --- Protótipos de Funções ---
// Declaramos as funções que serão implementadas em outros arquivos (.cpp)
// para que o 'main' saiba que elas existem.

/**
 * @brief Lê um arquivo .obj e popula os vetores com dados de vértices e faces.
 * (Implementada em 'io/objreader.cpp')
 */
bool read_obj_file(const std::string& filepath, std::vector<double>& vxs_pos, std::map<int, std::vector<int>>& fa_vxs);

/**
 * @brief Inicia o loop de prompt de comando para interação com o usuário.
 * (Implementada em 'io/usr.cpp')
 */
void start_user_prompt(TwoDHalfEdgeGeometry& geometry);

/**
 * @brief Inicia e configura o ambiente OpenGL e a janela de renderização.
 * (Implementada em 'renderer/renderer.cpp')
 */
void start_renderer(int argc, char* argv[], TwoDHalfEdgeGeometry& geometry);


// --- Função Principal ---

int main(int argc, char* argv[]) {
    // Verifica se o usuário forneceu um caminho de arquivo como argumento.
    std::string obj_filepath;
    if (argc > 1) {
        obj_filepath = argv[1];
    } else {
        std::cout << "Nenhum arquivo especificado." << std::endl;
        std::cout << "Uso: " << argv[0] << " <caminho_para_o_arquivo.obj>" << std::endl;
        return 1; // Encerra se nenhum arquivo for fornecido
    }

    // Estruturas de dados para armazenar as informações lidas do arquivo .obj.
    std::vector<double> vertex_positions;
    std::map<int, std::vector<int>> face_vertices;

    // 1. Ler os dados do arquivo .obj.
    std::cout << "Lendo o arquivo: " << obj_filepath << "..." << std::endl;
    if (!read_obj_file(obj_filepath, vertex_positions, face_vertices)) {
        std::cerr << "Erro fatal: Nao foi possivel ler ou processar o arquivo .obj. Encerrando." << std::endl;
        return 1;
    }
    std::cout << "Arquivo lido com sucesso." << std::endl;


    // 2. Construir a estrutura de dados e iniciar as interfaces.
    std::cout << "Construindo a estrutura de dados Half-Edge..." << std::endl;
    try {
        TwoDHalfEdgeGeometry geometry(vertex_positions, face_vertices);
        std::cout << "Estrutura de dados construida com sucesso!" << std::endl;

        // 3. Iniciar o prompt de comando para o usuário fazer consultas.
        start_user_prompt(geometry);

        // 4. DEPOIS que o prompt terminar, iniciar o renderer do OpenGL.
        // ESTA É A LINHA QUE FALTAVA!
        start_renderer(argc, argv, geometry);

    } catch (const std::exception& e) {
        std::cerr << "Um erro ocorreu durante a construcao da geometria: " << e.what() << std::endl;
        return 1;
    }


    return 0; // Sucesso
}

