/**
 * @file renderer.cpp
 * @brief Implementa a renderização do objeto 2D com interface, labels e transformações.
 *
 * Esta versão permite ao usuário alternar entre os algoritmos de desenho de linha
 * usando as teclas numéricas.
 */

#include <GL/freeglut.h>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <cmath>
#include <algorithm>

#include "../glm/glm.hpp"
#include "../glm/gtc/matrix_transform.hpp"
#include "../glm/gtc/type_ptr.hpp"

#include "../dsa/TwoDHalfEdgeGeometry.h"

#define KEY_ESCAPE 27
#define KEY_ENTER 13
#define KEY_BACKSPACE 8

#define SHEAR_FACTOR 0.2f
#define TRANSLATE_FACTOR 0.5f
#define ROTATE_DEGREE 10.0f
#define SCALE_FACTOR 0.2f

extern void save_obj_file(const std::string& filepath, 
                        std::map<unsigned int, std::vector<unsigned int>> faces_map,
                        std::unordered_map<unsigned int, std::pair<double, double>> vxs_pos);

// --- Enum para seleção de algoritmo ---
enum DrawAlgorithm {
    PARAMETRIC,
    BRESENHAM,
    XIAOLIN_WU
};

// --- Variáveis Globais para o Estado da UI e Labels ---
TwoDHalfEdgeGeometry* g_geometry = nullptr;
std::string g_command_input = "";
std::string g_command_output = "Teclas: [L] Labels, [1] Parametrico, [2] Bresenham.";
bool g_show_labels = false;
std::pair<double, double> centroid;
DrawAlgorithm g_current_algorithm = PARAMETRIC; // Algoritmo padrão

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

    if (action == "AJUDA") {
        g_command_output = "Comandos: FACES_ADJACENTES[FACE/ARESTA] <id>, [FACES/ARESTAS]_DO_VERTICE <id>";
        g_command_input.clear();
        return;
    }
    if (action == "SALVAR") {
            std::string filename;
            iss >> filename;
            if (filename.size() == 0)
            {
                g_command_output = "Erro: comando invalido. Ex.: 'SALVAR NOME_ARQUIVO'";
                g_command_input.clear();
                return;
            }

            save_obj_file(filename + ".obj", g_geometry->get_faces_with_vertices(),
                        g_geometry->get_vertexes());
            g_command_output = "Arquivo " + filename + ".obj" + " salvo.";
            g_command_input.clear();
            return;
        }

    int id;
    if (!(iss >> id)) {
        g_command_output = "Erro: Comando invalido ou ID ausente. Ex.: 'FACES_DO_VERTICE 1';"
                            " 'FACES_ADJACENTES_FACE 5'; 'FACES_ADJACENTES_ARESTA 2'";
        g_command_input.clear();
        return;
    }

    try {
        if (action == "FACES_ADJACENTES_FACE") {
            auto ids = g_geometry->face_get_adjacent_faces_ids(id);
            result_stream << "Faces adjacentes a face " << id << ": ";
            for(auto i : ids) result_stream << i << " ";
        } else if (action == "FACES_ADJACENTES_ARESTA") {
            auto ids = g_geometry->edge_get_adjacent_faces_ids(id);
            result_stream << "Faces adjacentes a aresta " << id << ": ";
            for(auto i : ids) result_stream << i << " ";
        } else if (action == "FACES_DO_VERTICE") {
            auto ids = g_geometry->get_vx_faces_id(id);
            result_stream << "Faces que compartilham o vertice " << id << ": ";
            for(auto i : ids) result_stream << i << " ";
        } else if (action == "ARESTAS_DO_VERTICE") {
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


// --- Funções de Desenho de Linha ---

/**
 * @brief Desenha uma linha usando a Equação Paramétrica.
 */
void draw_line_parametric(double x1, double y1, double x2, double y2) {
    const int num_steps = 50;
    double dx = x2 - x1;
    double dy = y2 - y1;
    
    glBegin(GL_POINTS);
    for (int i = 0; i <= num_steps; ++i) {
        double t = static_cast<double>(i) / num_steps;
        glVertex2d(x1 + t * dx, y1 + t * dy);
    }
    glEnd();
}

/**
 * @brief Desenha uma linha usando o Algoritmo de Bresenham (para todos os octantes).
 */
void draw_line_bresenham(double x1d, double y1d, double x2d, double y2d) {
    // A lógica de Bresenham funciona melhor com passos discretos (inteiros).
    // Mas para desenhar no mundo (world-space), precisamos manter os doubles.
    
    double dx = x2d - x1d;
    double dy = y2d - y1d;

    const int num_steps = 50; // Usando o mesmo número de passos da paramétrica.


    // Se a linha for muito curta (ou um ponto), desenha um ponto e sai.
    // Usamos uma pequena tolerância (epsilon) para comparação de float.
    if (std::abs(dx) < 1e-6 && std::abs(dy) < 1e-6) {
        glBegin(GL_POINTS);
        glVertex2d(x1d, y1d);
        glEnd();
        return;
    }

    double x_inc = dx / static_cast<double>(num_steps);
    double y_inc = dy / static_cast<double>(num_steps);

    double x = x1d;
    double y = y1d;

    glBegin(GL_POINTS);
    // Plota o primeiro ponto
    glVertex2d(x, y);
    
    // O algoritmo de Bresenham/DDA calcula os próximos pontos
    // Arredondar aqui é a chave: decidimos qual *pixel* (ou passo)
    // está mais próximo, mas plotamos no mundo `double`.
    // Para este caso, um DDA simples é mais robusto que um Bresenham
    // de inteiros puro, pois o mundo não é de inteiros.
    
    // Plotamos os 'num_steps' pontos restantes (totalizando num_steps + 1)
    for (int i = 0; i < num_steps; ++i) {
        x += x_inc;
        y += y_inc;
        // Plotamos o ponto real em double, não um int arredondado.
        glVertex2d(x, y);
    }
    glEnd();
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
            
            // Loop principal de desenho de arestas
            for (const auto& [vertex_pair, edge_id] : edges) {
                try {
                    const auto& pos1 = vertices.at(vertex_pair.first);
                    const auto& pos2 = vertices.at(vertex_pair.second);
                    
                    // --- AQUI ESTÁ A LÓGICA DE SELEÇÃO ---
                    switch (g_current_algorithm) {
                        case PARAMETRIC:
                            draw_line_parametric(pos1.first, pos1.second, pos2.first, pos2.second);
                            break;
                        case BRESENHAM:
                            draw_line_bresenham(pos1.first, pos1.second, pos2.first, pos2.second);
                            break;
                        case XIAOLIN_WU:
                            // Ainda não implementado, mas desenha com o padrão
                            draw_line_parametric(pos1.first, pos1.second, pos2.first, pos2.second);
                            break;
                    }

                } catch(const std::out_of_range&) {}
            }
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

            // Labels das Faces (magenta)
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

void translate(double x, double y)
{
    auto vertices = g_geometry->get_vertexes();
    for (auto itr = vertices.begin(); itr != vertices.end(); itr++)
    {
        glm::vec4 vx(itr->second.first, itr->second.second, 0, 1.0f);
        glm::mat4 trans = glm::mat4(1.0f);
        trans = glm::translate(trans, glm::vec3(x, y, 0.0f));
        vx = trans * vx;
        
        g_geometry->update_vertex_pos(itr->first, vx.x, vx.y);
    }
    centroid.first += x;
    centroid.second += y;
}

void rotate(float angle)
{
    auto vertices = g_geometry->get_vertexes();
    glm::vec2 center(centroid.first, centroid.second);
    
    for (auto itr = vertices.begin(); itr != vertices.end(); itr++)
    {
        glm::vec4 vx((float)itr->second.first, (float)itr->second.second, 0, 1.0f);
        glm::mat4 trans = glm::mat4(1.0f);
        trans = glm::translate(trans, glm::vec3(center, 0.0f));
        trans = glm::rotate(trans, glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f));
        trans = glm::translate(trans, glm::vec3(-center, 0.0f));
        vx = trans * vx;
        g_geometry->update_vertex_pos(itr->first, vx.x, vx.y);
    }
}

void scale(float f)
{
    auto vertices = g_geometry->get_vertexes();
    glm::vec2 center(centroid.first, centroid.second);
    for (auto itr = vertices.begin(); itr != vertices.end(); itr++)
    {
        glm::vec4 vx(itr->second.first, itr->second.second, 0, 1.0f);
        glm::mat4 trans = glm::mat4(1.0f);
        trans = glm::scale(trans, glm::vec3(f, f, 0.0));
        vx = trans * vx;
        g_geometry->update_vertex_pos(itr->first, vx.x, vx.y);
    }
    centroid = g_geometry->get_centroid();
}

void reflect(float x, float y)
{
    auto vertices = g_geometry->get_vertexes();
    glm::vec2 center(centroid.first, centroid.second);
    for (auto itr = vertices.begin(); itr != vertices.end(); itr++)
    {
        glm::vec4 vx(itr->second.first, itr->second.second, 0, 1.0f);
        glm::mat4 trans = glm::mat4(1.0f);
        trans = glm::translate(trans, glm::vec3(center, 0.0f));
        trans = glm::rotate(trans, glm::radians(180.0f), glm::vec3(x, y, 0.0));
        trans = glm::translate(trans, glm::vec3(-center, 0.0f));
        vx = trans * vx;
        g_geometry->update_vertex_pos(itr->first, vx.x, vx.y);
    }
    centroid = g_geometry->get_centroid();
}

void shear(float x, float y)
{
        auto vertices = g_geometry->get_vertexes();
    for (auto itr = vertices.begin(); itr != vertices.end(); itr++)
    {
        glm::vec2 center(centroid.first, centroid.second);
        glm::vec4 vx(itr->second.first, itr->second.second, 0, 1.0f);
        glm::mat4 trans = glm::mat4(1.0f);
        trans = glm::shear(trans, glm::vec3(1.0f, 1.0f, 0.0f), 
                            glm::vec2(x, 0.0f),
                            glm::vec2(y, 0.0f),
                            glm::vec2(0.0f, 0.0f));
        vx = trans * vx;
        g_geometry->update_vertex_pos(itr->first, vx.x, vx.y);
    }
    centroid = g_geometry->get_centroid();
}

void keyboard(unsigned char key, int x, int y) {
    
    switch (key) {
        case KEY_ESCAPE: glutLeaveMainLoop(); break;
        case KEY_ENTER: process_command(); break;
        case KEY_BACKSPACE: if (!g_command_input.empty()) { g_command_input.pop_back(); } break;
        case 'd':
            translate(TRANSLATE_FACTOR, 0.0);
            break;
        case 'a':
            translate(-TRANSLATE_FACTOR, 0.0);
            break;
        case 'w':
            translate(0.0, TRANSLATE_FACTOR);
            break;
        case 's':
            translate(0.0, -TRANSLATE_FACTOR);
            break;
        case 'e':
            rotate(ROTATE_DEGREE);
            break;
        case 'q':
            rotate(-ROTATE_DEGREE);
            break;
        case '+':
            scale(1.0f + SCALE_FACTOR);
            break;
        case '-':
            scale(1.0f - SCALE_FACTOR);
            break;
        case 't':
            reflect(1.0f, 0.0f);
            break;
        case 'g':
            reflect(0.0f, 1.0f);
            break;
        case 'k':
            shear(SHEAR_FACTOR, 0.0f);
            break;
        case 'h':
            shear(-SHEAR_FACTOR, 0.0f);
            break;
        case 'u':
            shear(0.0f, SHEAR_FACTOR);
            break;
        case 'j':
            shear(0.0f, -SHEAR_FACTOR);
            break;
        case 'l':
            g_show_labels = !g_show_labels;
            g_command_output = g_show_labels ? "Labels de ID ativados." : "Labels de ID desativados.";
            break;

        // --- NOVAS TECLAS PARA MUDAR ALGORITMO ---
        case '1':
            g_current_algorithm = PARAMETRIC;
            g_command_output = "Algoritmo: Parametrico";
            break;
        case '2':
            g_current_algorithm = BRESENHAM;
            g_command_output = "Algoritmo: Bresenham";
            break;
        // case '3': // Futuramente para o Xiaolin Wu

        default:
            if (isprint(key) && (isupper(key) || ispunct(key) || isxdigit(key) || isblank(key)))
            {
                g_command_input += key;
            }
            break;
    }
    glutPostRedisplay();
}


void start_renderer(int argc, char* argv[], TwoDHalfEdgeGeometry& geometry) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_ALPHA);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Visualizador Interativo .obj");

    g_geometry = &geometry;
    centroid = g_geometry->get_centroid();

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

