/**
 * @file renderer.cpp
 * @brief Implementa a renderização do objeto 2D com interface e labels completos.
 *
 * Esta versão adiciona a funcionalidade de exibir os IDs das faces, além dos
 * vértices e arestas, diretamente na tela.
 */

#include <GL/freeglut.h>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <cmath>
#include <algorithm>

#include "../dsa/TwoDHalfEdgeGeometry.h"

// --- Variáveis Globais para o Estado da UI e Labels ---
TwoDHalfEdgeGeometry* g_geometry = nullptr;
std::string g_command_input = "";
std::string g_command_output = "Digite 'ajuda' e pressione Enter. Pressione 'l' para ver os IDs.";
bool g_show_labels = false;

// --- Funções da Interface Gráfica ---

void render_text(float x, float y, const std::string& text) {
    glRasterPos2f(x, y);
    for (char c : text) {
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, c);
    }
}

void process_command() {
    if (g_command_input.empty()) return;
    std::stringstream result_stream;
    std::istringstream iss(g_command_input);
    std::string action;
    iss >> action;

    if (action == "ajuda") {
        g_command_output = "Comandos: faces_adjacentes_face/aresta <id>, faces/arestas_do_vertice <id>";
        g_command_input.clear();
        return;
    }

    int id;
    if (!(iss >> id)) {
        g_command_output = "Erro: Comando invalido ou ID ausente. Ex: 'faces_do_vertice 1'";
        g_command_input.clear();
        return;
    }

    try {
        if (action == "faces_adjacentes_face") {
            auto ids = g_geometry->face_get_adjacent_faces_ids(id);
            result_stream << "Faces adjacentes a face " << id << ": ";
            for(auto i : ids) result_stream << i << " ";
        } else if (action == "faces_adjacentes_aresta") {
            auto ids = g_geometry->edge_get_adjacent_faces_ids(id);
            result_stream << "Faces adjacentes a aresta " << id << ": ";
            for(auto i : ids) result_stream << i << " ";
        } else if (action == "faces_do_vertice") {
            auto ids = g_geometry->get_vx_faces_id(id);
            result_stream << "Faces que compartilham o vertice " << id << ": ";
            for(auto i : ids) result_stream << i << " ";
        } else if (action == "arestas_do_vertice") {
            auto ids = g_geometry->get_vx_edges_id(id);
            result_stream << "Arestas que partem do vertice " << id << ": ";
            for(auto i : ids) result_stream << i << " ";
        } else {
            result_stream << "Erro: Comando '" << action << "' nao reconhecido.";
        }
    } catch (const std::out_of_range& e) {
        result_stream << "Erro: O ID " << id << " nao foi encontrado.";
    }
    
    g_command_output = result_stream.str();
    g_command_input.clear();
}


// --- Funções de Callback do OpenGL ---

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    if (g_geometry) {
        auto vertices = g_geometry->get_vertexes();
        auto edges = g_geometry->get_vxs_conn_edges_id();

        glColor3f(1.0f, 1.0f, 1.0f);
        if (!vertices.empty() && !edges.empty()) {
            glPointSize(2.0f);
            glBegin(GL_POINTS);
            for (const auto& [vertex_pair, edge_id] : edges) {
                try {
                    const auto& pos1 = vertices.at(vertex_pair.first);
                    const auto& pos2 = vertices.at(vertex_pair.second);
                    const int num_steps = 100;
                    double dx = pos2.first - pos1.first;
                    double dy = pos2.second - pos1.second;
                    for (int i = 0; i <= num_steps; ++i) {
                        double t = static_cast<double>(i) / num_steps;
                        glVertex2d(pos1.first + t * dx, pos1.second + t * dy);
                    }
                } catch(const std::out_of_range&) {}
            }
            glEnd();
        }

        if (g_show_labels) {
            // Labels dos Vértices (amarelo)
            glColor3f(1.0f, 1.0f, 0.0f);
            for (const auto& [id, pos] : vertices) {
                render_text(pos.first + 0.1, pos.second + 0.1, std::to_string(id));
            }

            // Labels das Arestas (ciano)
            glColor3f(0.0f, 1.0f, 1.0f);
            for (const auto& [vertex_pair, edge_id] : edges) {
                 try {
                    const auto& pos1 = vertices.at(vertex_pair.first);
                    const auto& pos2 = vertices.at(vertex_pair.second);
                    float midX = (pos1.first + pos2.first) / 2.0f;
                    float midY = (pos1.second + pos2.second) / 2.0f;
                    render_text(midX + 0.1f, midY + 0.1f, std::to_string(edge_id));
                } catch(const std::out_of_range&) {}
            }

            // NOVO: Labels das Faces (magenta)
            glColor3f(1.0f, 0.0f, 1.0f);
            auto faces = g_geometry->get_faces_with_vertices();
            for (const auto& [face_id, vertex_ids] : faces) {
                if (vertex_ids.empty()) continue;

                double centroidX = 0.0, centroidY = 0.0;
                for (unsigned int vertex_id : vertex_ids) {
                    try {
                        const auto& pos = vertices.at(vertex_id);
                        centroidX += pos.first;
                        centroidY += pos.second;
                    } catch (const std::out_of_range&) {}
                }
                centroidX /= vertex_ids.size();
                centroidY /= vertex_ids.size();
                render_text(centroidX, centroidY, "f" + std::to_string(face_id));
            }
        }
    }

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, glutGet(GLUT_WINDOW_WIDTH), 0, glutGet(GLUT_WINDOW_HEIGHT));
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    glColor4f(0.0f, 0.0f, 0.0f, 0.7f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(glutGet(GLUT_WINDOW_WIDTH), 0);
    glVertex2f(glutGet(GLUT_WINDOW_WIDTH), 60);
    glVertex2f(0, 60);
    glEnd();

    glColor3f(0.0f, 1.0f, 0.0f);
    render_text(10, 40, "Comando: " + g_command_input);
    render_text(10, 15, "Resultado: " + g_command_output);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 27: glutLeaveMainLoop(); break;
        case 13: process_command(); break;
        case 8: if (!g_command_input.empty()) { g_command_input.pop_back(); } break;
        case 'l': case 'L':
            g_show_labels = !g_show_labels;
            g_command_output = g_show_labels ? "Labels de ID ativados." : "Labels de ID desativados.";
            break;
        default: if (isprint(key)) { g_command_input += key; } break;
    }
    glutPostRedisplay();
}


void start_renderer(int argc, char* argv[], TwoDHalfEdgeGeometry& geometry) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_ALPHA);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Visualizador Interativo .obj");

    g_geometry = &geometry;

    auto vertices = g_geometry->get_vertexes();
    double minX = 0, maxX = 0, minY = 0, maxY = 0;
    if (!vertices.empty()) {
        minX = maxX = vertices.begin()->second.first;
        minY = maxY = vertices.begin()->second.second;
        for (const auto& [id, pos] : vertices) {
            minX = std::min(minX, pos.first);
            maxX = std::max(maxX, pos.first);
            minY = std::min(minY, pos.second);
            maxY = std::max(maxY, pos.second);
        }
    }
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    double paddingX = (maxX - minX) * 0.1;
    double paddingY = (maxY - minY) * 0.1;
    double padding = std::max({1.0, paddingX, paddingY});
    glOrtho(minX - padding, maxX + padding, minY - padding, maxY + padding, -1.0, 1.0);
    
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);

    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glutMainLoop();
}

