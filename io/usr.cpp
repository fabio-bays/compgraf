/**
 * @file usr.cpp
 * @brief Implementa a interface de linha de comando para interação com o usuário.
 *
 * Este arquivo contém a lógica para exibir um menu de opções, ler a entrada
 * do usuário, invocar as consultas apropriadas na estrutura de dados Half-Edge
 * e exibir os resultados de forma legível.
 */

#include <iostream>
#include <string>
#include <vector>
#include <limits> // Necessário para std::numeric_limits
#include <stdexcept> // Necessário para std::out_of_range

// Inclui o cabeçalho da estrutura de dados.
// O caminho "../dsa/" assume que o 'usr.cpp' está na pasta 'io'.
#include "../dsa/TwoDHalfEdgeGeometry.h"

// Protótipo da função principal deste arquivo.
void start_user_prompt(TwoDHalfEdgeGeometry& geometry);

/**
 * @brief Exibe o menu de comandos disponíveis para o usuário.
 */
void display_menu() {
    std::cout << "\n--- Menu de Consultas ---" << std::endl;
    std::cout << "Por favor, use um dos seguintes comandos:" << std::endl;
    std::cout << "  faces_adjacentes_face <id_da_face>" << std::endl;
    std::cout << "  faces_adjacentes_aresta <id_da_aresta>" << std::endl;
    std::cout << "  faces_do_vertice <id_do_vertice>" << std::endl;
    std::cout << "  arestas_do_vertice <id_do_vertice>" << std::endl;
    std::cout << "  ajuda (para exibir este menu novamente)" << std::endl;
    std::cout << "  sair (para encerrar o programa)" << std::endl;
    std::cout << "--------------------------" << std::endl;
}

/**
 * @brief Inicia o loop de prompt de comando para interação com o usuário.
 * @param geometry Uma referência ao objeto de geometria Half-Edge já construído.
 */
void start_user_prompt(TwoDHalfEdgeGeometry& geometry) {
    display_menu();

    std::string command;
    while (true) {
        std::cout << "\n> ";
        std::cin >> command;

        if (command == "sair") {
            std::cout << "Encerrando o programa..." << std::endl;
            break;
        }

        if (command == "ajuda") {
            display_menu();
            continue;
        }

        // Tenta ler um ID numérico após o comando.
        int id;
        if (!(std::cin >> id)) {
            std::cerr << "Erro: ID invalido. Por favor, insira um numero inteiro." << std::endl;
            // Limpa o buffer de entrada para evitar loops infinitos em caso de erro.
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }

        // --- Execução das Consultas ---
        try {
            if (command == "faces_adjacentes_face") {
                std::vector<unsigned int> adjacent_faces_ids = geometry.face_get_adjacent_faces_ids(id);

                std::cout << "Faces adjacentes a face " << id << ":" << std::endl;
                if (adjacent_faces_ids.empty()) {
                    std::cout << "  Nenhuma face adjacente encontrada." << std::endl;
                } else {
                    for (const auto& face_id : adjacent_faces_ids) {
                        std::cout << "  - Face " << face_id << std::endl;
                    }
                }

            } else if (command == "faces_adjacentes_aresta") {
                std::vector<unsigned int> adjacent_faces_ids = geometry.edge_get_adjacent_faces_ids(id);

                std::cout << "Faces adjacentes a aresta " << id << ":" << std::endl;
                if (adjacent_faces_ids.empty()) {
                    std::cout << "  Nenhuma face adjacente encontrada (pode ser uma aresta de borda)." << std::endl;
                } else {
                    for (const auto& face_id : adjacent_faces_ids) {
                        std::cout << "  - Face " << face_id << std::endl;
                    }
                }

            } else if (command == "faces_do_vertice") {
                std::vector<unsigned int> shared_faces_ids = geometry.get_vx_faces_id(id);

                std::cout << "Faces que compartilham o vertice " << id << ":" << std::endl;
                if (shared_faces_ids.empty()) {
                    std::cout << "  Nenhuma face encontrada para este vertice." << std::endl;
                } else {
                    for (const auto& face_id : shared_faces_ids) {
                        std::cout << "  - Face " << face_id << std::endl;
                    }
                }

            } else if (command == "arestas_do_vertice") {
                std::vector<unsigned int> leaving_edges_ids = geometry.get_vx_edges_id(id);

                std::cout << "Arestas que partem do vertice " << id << ":" << std::endl;
                if (leaving_edges_ids.empty()) {
                    std::cout << "  Nenhuma aresta encontrada para este vertice." << std::endl;
                } else {
                    for (const auto& edge_id : leaving_edges_ids) {
                        std::cout << "  - Aresta " << edge_id << std::endl;
                    }
                }
            } else {
                std::cerr << "Erro: Comando '" << command << "' nao reconhecido. Digite 'ajuda' para ver as opcoes." << std::endl;
            }
        } catch (const std::out_of_range& e) {
            // Captura o erro se o usuário fornecer um ID que não existe no mapa.
            std::cerr << "Erro: O ID " << id << " nao foi encontrado na estrutura de dados." << std::endl;
        }
    }
}

