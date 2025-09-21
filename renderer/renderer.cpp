/**
 * @file renderer.cpp
 * @brief Implementa a renderização do objeto 2D com câmera fixa.
 *
 * Utiliza uma equação de reta paramétrica para desenhar as arestas com
 * uma densidade de pontos consistente, garantindo que pareçam linhas sólidas.
 */

#include <GL/freeglut.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

#include "../dsa/TwoDHalfEdgeGeometry.h"

// Variáveis Globais para o Renderer
TwoDHalfEdgeGeometry* g_geometry = nullptr;

// **CORREÇÃO**: Algoritmo de desenho de linha substituído por equação paramétrica.
// Isso garante uma densidade constante de pontos, fazendo as arestas parecerem sólidas.
void draw_segment_by_line_equation(double x1, double y1, double x2, double y2) {
    const int num_steps = 100; // Desenha 100 pontos por segmento de reta
    double dx = x2 - x1;
    double dy = y2 - y1;

    for (int i = 0; i <= num_steps; ++i) {
        double t = static_cast<double>(i) / num_steps;
        double x = x1 + t * dx;
        double y = y1 + t * dy;
        glVertex2d(x, y);
    }
}


// --- Funções de Callback do OpenGL ---

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glColor3f(1.0f, 1.0f, 1.0f);

    if (!g_geometry) {
        glutSwapBuffers();
        return;
    }

    // Busca os dados da geometria
    auto vertices = g_geometry->get_vertexes();
    auto edges = g_geometry->get_vxs_conn_edges_id();

    if (vertices.empty() || edges.empty()) {
        glutSwapBuffers();
        return;
    }
    
    // Desenha cada aresta ponto a ponto
    glPointSize(2.0f);
    glBegin(GL_POINTS);
    for (const auto& [vertex_pair, edge_id] : edges) {
        try {
            const auto& pos1 = vertices.at(vertex_pair.first);
            const auto& pos2 = vertices.at(vertex_pair.second);
            draw_segment_by_line_equation(pos1.first, pos1.second, pos2.first, pos2.second);
        } catch(const std::out_of_range& e) {
            // Ignora caso haja alguma inconsistência nos IDs, evitando que o programa quebre
            continue;
        }
    }
    glEnd();

    glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y) {
    if (key == 27) { // Tecla ESC
        glutLeaveMainLoop();
    }
}


// --- Função Principal de Inicialização ---

void start_renderer(int argc, char* argv[], TwoDHalfEdgeGeometry& geometry) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Visualizador de Objeto .obj - Equacao da Reta");

    g_geometry = &geometry;

    // Calcula os limites do objeto para enquadrar a câmera
    auto vertices = g_geometry->get_vertexes();
    double minX = 0, maxX = 0, minY = 0, maxY = 0;

    if (!vertices.empty()) {
        minX = maxX = vertices.begin()->second.first;
        minY = maxY = vertices.begin()->second.second;
        for (const auto& [id, pos] : vertices) {
            if (pos.first < minX) minX = pos.first;
            if (pos.first > maxX) maxX = pos.first;
            if (pos.second < minY) minY = pos.second;
            if (pos.second > maxY) maxY = pos.second;
        }
    }
    
    // Configuração da Câmera
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    double paddingX = (maxX - minX) * 0.1;
    double paddingY = (maxY - minY) * 0.1;
    double padding = std::max(paddingX, paddingY);
    if (padding < 1.0) padding = 1.0; // Garante um padding mínimo
    glOrtho(minX - padding, maxX + padding, minY - padding, maxY + padding, -1.0, 1.0);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);

    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);

    std::cout << "\nJanela do renderer iniciada. Pressione 'ESC' para fechar." << std::endl;
    glutMainLoop();
}

