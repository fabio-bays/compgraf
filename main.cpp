/**
 * @file main.cpp
 * @brief Ponto de entrada principal do programa de geometria 2D.
 *
 * Responsável por:
 * 1. Processar argumentos de linha de comando para obter o caminho do arquivo .obj.
 * 2. Chamar o leitor de .obj para carregar os dados do modelo.
 * 3. Instanciar e construir a estrutura de dados Half-Edge.
 * 4. Iniciar a interface de prompt de comando para o usuário.
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
 * @param filepath O caminho para o arquivo .obj.
 * @param vxs_pos Vetor para armazenar as posições dos vértices (x1, y1, x2, y2, ...).
 * @param fa_vxs Mapa para armazenar os índices de vértices para cada face.
 * @return true se a leitura for bem-sucedida, false caso contrário.
 *
 * (Esta função deve ser implementada em 'io/objreader.cpp')
 */
bool read_obj_file(const std::string& filepath, std::vector<double>& vxs_pos, std::map<int, std::vector<int>>& fa_vxs);

/**
 * @brief Inicia o loop de prompt de comando para interação com o usuário.
 * @param geometry Uma referência ao objeto de geometria já construído.
 *
 * (Esta função é implementada em 'io/usr.cpp')
 */
void start_user_prompt(TwoDHalfEdgeGeometry& geometry);


// --- Função Principal ---

int main(int argc, char* argv[]) {
    // Verifica se o usuário forneceu um caminho de arquivo como argumento.
    // Se não, usa um nome padrão "objeto.obj".
    std::string obj_filepath;
    if (argc > 1) {
        obj_filepath = argv[1];
    } else {
        obj_filepath = "objeto.obj";
        std::cout << "Nenhum arquivo especificado. Usando '" << obj_filepath << "' por padrao." << std::endl;
        std::cout << "Uso: " << argv[0] << " <caminho_para_o_arquivo.obj>" << std::endl;
    }

    // Estruturas de dados para armazenar as informações lidas do arquivo .obj.
    // Elas precisam corresponder ao que o construtor da sua classe espera.
    std::vector<double> vertex_positions;
    std::map<int, std::vector<int>> face_vertices;

    // 1. Ler os dados do arquivo .obj chamando a função do objreader.
    std::cout << "Lendo o arquivo: " << obj_filepath << "..." << std::endl;
    if (!read_obj_file(obj_filepath, vertex_positions, face_vertices)) {
        std::cerr << "Erro fatal: Nao foi possivel ler ou processar o arquivo .obj. Encerrando." << std::endl;
        return 1; // Retorna um código de erro.
    }
    std::cout << "Arquivo lido com sucesso." << std::endl;


    // 2. Construir a estrutura de dados Half-Edge.
    // O construtor da classe TwoDHalfEdgeGeometry faz todo o trabalho pesado.
    std::cout << "Construindo a estrutura de dados Half-Edge..." << std::endl;
    try {
        TwoDHalfEdgeGeometry geometry(vertex_positions, face_vertices);
        std::cout << "Estrutura de dados construida com sucesso!" << std::endl;

        // 3. Iniciar o prompt de comando para o usuário fazer consultas.
        start_user_prompt(geometry);

    } catch (const std::exception& e) {
        std::cerr << "Um erro ocorreu durante a construcao da geometria: " << e.what() << std::endl;
        return 1;
    }


    return 0; // Sucesso
}
