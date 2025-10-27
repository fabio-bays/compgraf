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
// ATUALIZADO para incluir a tecla 3
std::string g_command_output = "Teclas: [L] Labels, [1] Parametrico, [2] Bresenham, [3] Xiaolin Wu."; 
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

// --- Funções Auxiliares para Xiaolin Wu (Baseadas no exemplo do professor) ---

/**
 * @brief Desenha um pixel com intensidade de cor (para Xiaolin Wu).
 * (Baseado no 'putPixel' do professor, mas usa Alfa)
 */
void putPixel(int x, int y, float c) {
  //glPointSize(4.0); // Opcional: pode deixar os pontos maiores
  
  // Usamos glColor4f para definir a cor (Branca) e a intensidade (c) no Alfa.
  // Isso requer que GL_BLEND esteja ativado (feito em start_renderer).
  glColor4f(1.0f, 1.0f, 1.0f, c); // Cor Branca, com intensidade 'c' no Alfa
  
  glBegin(GL_POINTS);
  glVertex2i(x, y);
  glEnd();
}

/**
 * @brief Retorna a parte fracionária (exemplo do professor).
 */
inline float fpart(float x)  { 
  return x - floorf(x); 
}    

/**
 * @brief Retorna a parte fracionária inversa (exemplo do professor).
 */
inline float rfpart(float x) { 
  return 1.0f - (x - floorf(x)); 
}
// --- Fim das Funções Auxiliares ---


/**
 * @brief Desenha uma linha usando a Equação Paramétrica.
 */
void draw_line_parametric(double x1, double y1, double x2, double y2) {
    const int num_steps = 100; 
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
    
    // 1. Converte/Arredonda as coordenadas de mundo (double) para
    //    a grade de inteiros mais próxima.
    int x0 = static_cast<int>(std::round(x1d));
    int y0 = static_cast<int>(std::round(y1d));
    int x1 = static_cast<int>(std::round(x2d));
    int y1 = static_cast<int>(std::round(y2d));

    // 2. Lógica de inteiros pura (do bresenham.c)
    
    // define os deltas para x e y
    int dx = x1 - x0;
    int dy = y1 - y0;

    // definir os movimentos de eixo para x e y 
    int sx = (dx >= 0) ? 1 : -1;
    int sy = (dy >= 0) ? 1 : -1;

    dx = std::abs(dx);
    dy = std::abs(dy);

    // definir os pontos iniciais
    int x = x0;
    int y = y0;
    
    glBegin(GL_POINTS);
    
    // ponto inicial
    glVertex2i(x,y); 
    
    // definir o movimento  
    if (dx > dy) {
        // definir nabla (delta inverso) ou Pk (parâmetro de decisão)
        int d_inv = 2 * dy - dx; // Corrigido do exemplo (Pk inicial)
        
        for (int i = 0; i < dx; i++) {      
            x += sx;
            if (d_inv < 0) {
                d_inv += 2 * dy;
            } else {
                y += sy;
                d_inv += 2 * (dy - dx);
            }
            // demais pontos
            glVertex2i(x,y);
        }  
    }  else {    
        // definir nabla (delta inverso) ou Pk
        int d_inv = 2 * dx - dy; // Corrigido do exemplo (Pk inicial)

        for (int i = 0; i < dy; i++) {
            y += sy;
            if (d_inv < 0) {
                d_inv += 2 * dx;
            } else {
                x += sx;
                d_inv += 2 * (dx - dy);
            }
            // demais pontos
            glVertex2i(x,y);
        }
    }
    
    glEnd();
}


/**
 * @brief Implementação interna do Xiaolin Wu com inteiros (baseado no exemplo).
 * (Esta é a implementação padrão/corrigida, pois o exemplo do professor
 * tinha um bug ao calcular a intensidade dos pontos finais).
 */
void xiaolin_wu_integer_impl(int x0, int y0, int x1, int y1) {
    
    bool steep = std::abs(y1 - y0) > std::abs(x1 - x0);
  
    if (steep) {
        std::swap(x0, y0);
        std::swap(x1, y1);
    }
  
    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    float dx = x1 - x0;
    float dy = y1 - y0;    
    float m = (dx < 1e-9) ? 1.0 : dy / dx; // Evita divisão por zero

    // Ponto inicial
    // (Arredonda x0 para o centro do pixel mais próximo)
    int xend = static_cast<int>(std::round(x0));
    // Calcula y correspondente
    float yend = y0 + m * (xend - x0);
    // Distância do centro do pixel (gap)
    float xgap = rfpart(x0 + 0.5f);
    int xpxl1 = xend;
    int ypxl1 = static_cast<int>(std::floor(yend));
    
    if (steep) {
        putPixel(ypxl1,     xpxl1, rfpart(yend) * xgap);
        putPixel(ypxl1 + 1, xpxl1,  fpart(yend) * xgap);
    } else {
        putPixel(xpxl1, ypxl1,     rfpart(yend) * xgap);
        putPixel(xpxl1, ypxl1 + 1,  fpart(yend) * xgap);
    }
    float intery = yend + m; // Primeira intersecção y

    // Ponto final
    xend = static_cast<int>(std::round(x1));
    yend = y1 + m * (xend - x1);
    xgap = fpart(x1 + 0.5f);
    int xpxl2 = xend;
    int ypxl2 = static_cast<int>(std::floor(yend));
    
    if (steep) {
        putPixel(ypxl2,     xpxl2, rfpart(yend) * xgap);
        putPixel(ypxl2 + 1, xpxl2,  fpart(yend) * xgap);
    } else {
        putPixel(xpxl2, ypxl2,     rfpart(yend) * xgap);
        putPixel(xpxl2, ypxl2 + 1,  fpart(yend) * xgap);
    }
    
    // Loop principal (interpolação entre os pontos)
    if (steep) {
        for (int x = xpxl1 + 1; x <= xpxl2 - 1; ++x) {    
            putPixel(static_cast<int>(std::floor(intery)), x, rfpart(intery));
            putPixel(static_cast<int>(std::floor(intery))+1, x, fpart(intery));
            intery += m;
        }
    } else {
         for (int x = xpxl1 + 1; x <= xpxl2 - 1; ++x) {    
            putPixel(x, static_cast<int>(std::floor(intery)), rfpart(intery));
            putPixel(x, static_cast<int>(std::floor(intery))+1, fpart(intery));
            intery += m;
        }
    }
}

/**
 * @brief Função "Wrapper" que arredonda as coordenadas para o Xiaolin Wu.
 */
void draw_line_xiaolin_wu(double x1d, double y1d, double x2d, double y2d) {
    // 1. Arredonda as coordenadas de mundo (double) para
    //    a grade de inteiros mais próxima.
    int x0 = static_cast<int>(std::round(x1d));
    int y0 = static_cast<int>(std::round(y1d));
    int x1 = static_cast<int>(std::round(x2d));
    int y1 = static_cast<int>(std::round(y2d));

    // 2. Chama a implementação de inteiros
    xiaolin_wu_integer_impl(x0, y0, x1, y1);
}


// --- Funções de Callback do OpenGL ---

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    if (g_geometry) {
        auto vertices = g_geometry->get_vertexes();
        auto edges = g_geometry->get_vxs_conn_edges_id();

        // Cor padrão definida aqui (Branco), mas Xiaolin Wu define a sua própria
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
                            // Define a cor sólida (sem alfa)
                            glColor3f(1.0f, 1.0f, 1.0f); 
                            draw_line_parametric(pos1.first, pos1.second, pos2.first, pos2.second);
                            break;
                        case BRESENHAM:
                            // Define a cor sólida (sem alfa)
                            glColor3f(1.0f, 1.0f, 1.0f); 
                            draw_line_bresenham(pos1.first, pos1.second, pos2.first, pos2.second);
                            break;
                        case XIAOLIN_WU:
                            // A cor/alfa é definida DENTRO do 'putPixel'
                            // Não precisa definir glColor3f aqui.
                            draw_line_xiaolin_wu(pos1.first, pos1.second, pos2.first, pos2.second);
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

    // Desenha a Interface (Barra de Comando)
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, glutGet(GLUT_WINDOW_WIDTH), 0, glutGet(GLUT_WINDOW_HEIGHT));
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // Fundo da barra
    glColor4f(0.0f, 0.0f, 0.0f, 0.7f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(glutGet(GLUT_WINDOW_WIDTH), 0);
    glVertex2f(glutGet(GLUT_WINDOW_WIDTH), 60);
    glVertex2f(0, 60);
    glEnd();

    // Texto da barra
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
        // A função glm::shear não existe. Vamos usar uma matriz de cisalhamento manual.
        
        // Matriz de cisalhamento em X
        if (std::abs(x) > 1e-9) {
            trans[1][0] = x; // Coluna 1, Linha 0 (glm é column-major)
        }
        // Matriz de cisalhamento em Y
        if (std::abs(y) > 1e-9) {
            trans[0][1] = y; // Coluna 0, Linha 1
        }

        // Aplicar em relação ao centroide
        glm::mat4 moveToOrigin = glm::translate(glm::mat4(1.0f), glm::vec3(-center, 0.0f));
        glm::mat4 moveBack = glm::translate(glm::mat4(1.0f), glm::vec3(center, 0.0f));
        
        vx = moveBack * trans * moveToOrigin * vx;

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
        case '3': // Habilitado para Xiaolin Wu
            g_current_algorithm = XIAOLIN_WU;
            g_command_output = "Algoritmo: Xiaolin Wu (AA)";
            break;

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
    // ATUALIZADO para incluir GLUT_ALPHA para o Xiaolin Wu
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
    
    // --- Configurações de BLEND para Xiaolin Wu ---
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Dica para o OpenGL priorizar qualidade no anti-aliasing (opcional)
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_POINT_SMOOTH); // Habilita anti-aliasing para pontos (bom para o Wu)
    // --- Fim das Configurações de BLEND ---

    glutMainLoop();
}

