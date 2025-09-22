/**
 * @file usr.cpp
 * @brief Implementa a interface de linha de comando para interação com o usuário.
 *
 * Esta versão é projetada para ser chamada de forma não-bloqueante,
 * processando um único comando por vez para permitir a execução simultânea
 * com a janela do OpenGL.
 */

#include <iostream>
#include <string>
#include <vector>
#include <limits>
#include <stdexcept>
#include <sstream>

#include "../dsa/TwoDHalfEdgeGeometry.h"

// Mantém o estado da interface
bool g_first_run = true;

/**
 * @brief Exibe o menu de comandos disponíveis para o usuário.
 */
void display_menu() {
    std::cout << "\n--- Menu de Consultas ---" << std::endl;
    std::cout << "Digite um comando no terminal e pressione Enter." << std::endl;
    std::cout << "  faces_adjacentes_face <id>" << std::endl;
    std::cout << "  faces_adjacentes_aresta <id>" << std::endl;
    std::cout << "  faces_do_vertice <id>" << std::endl;
    std::cout << "  arestas_do_vertice <id>" << std::endl;
    std::cout << "  ajuda (para exibir este menu)" << std::endl;
    std::cout << "  (Pressione ESC na janela para sair)" << std::endl;
    std::cout << "--------------------------" << std::endl;
    std::cout << "> " << std::flush;
}

/**
 * @brief Processa um único comando do usuário de forma não-bloqueante.
 * @param geometry Uma referência ao objeto de geometria Half-Edge.
 */
void process_user_command(TwoDHalfEdgeGeometry& geometry) {
    if (g_first_run) {
        display_menu();
        g_first_run = false;
    }

    // Verifica se há algo para ler no buffer de entrada
    // Esta parte é um pouco complexa e depende do sistema operacional,
    // a forma mais simples é apenas tentar ler.
    // Para uma solução mais robusta, seriam necessárias APIs específicas.
    std::string command;
    
    // Vamos usar um truque simples: ler uma linha inteira
    if (std::getline(std::cin, command)) {
        if (command.empty()) {
             std::cout << "> " << std::flush;
            return; // Ignora linhas vazias
        }

        std::istringstream iss(command);
        std::string action;
        iss >> action;

        if (action == "ajuda") {
            display_menu();
            return;
        }

        int id;
        if (!(iss >> id)) {
            std::cerr << "Erro: Comando invalido ou ID ausente. Tente 'comando <numero>'." << std::endl;
            std::cout << "> " << std::flush;
            return;
        }

        try {
            if (action == "faces_adjacentes_face") {
                auto ids = geometry.face_get_adjacent_faces_ids(id);
                std::cout << "Faces adjacentes a face " << id << ": ";
                for(auto i : ids) std::cout << i << " ";
                std::cout << std::endl;
            } else if (action == "faces_adjacentes_aresta") {
                auto ids = geometry.edge_get_adjacent_faces_ids(id);
                std::cout << "Faces adjacentes a aresta " << id << ": ";
                for(auto i : ids) std::cout << i << " ";
                std::cout << std::endl;
            } else if (action == "faces_do_vertice") {
                auto ids = geometry.get_vx_faces_id(id);
                std::cout << "Faces que compartilham o vertice " << id << ": ";
                for(auto i : ids) std::cout << i << " ";
                std::cout << std::endl;
            } else if (action == "arestas_do_vertice") {
                auto ids = geometry.get_vx_edges_id(id);
                std::cout << "Arestas que partem do vertice " << id << ": ";
                for(auto i : ids) std::cout << i << " ";
                std::cout << std::endl;
            } else {
                std::cerr << "Erro: Comando '" << action << "' nao reconhecido." << std::endl;
            }
        } catch (const std::out_of_range& e) {
            std::cerr << "Erro: O ID " << id << " nao foi encontrado." << std::endl;
        }
        std::cout << "> " << std::flush;
    }
}

