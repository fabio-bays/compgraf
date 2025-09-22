#ifndef TWOD_GEOMETRY_DEFINED_H
#define TWOD_GEOMETRY_DEFINED_H
#include <vector>
#include <unordered_map>
#include <map>
#include <utility>
#include <stdexcept>
#include <algorithm>
#include <string>

class TwoDHalfEdgeGeometry
{
private:
    struct vertex;
    struct half_edge;
    struct face;
    typedef struct TwoDHalfEdgeGeometry::vertex Vertex;
    typedef struct TwoDHalfEdgeGeometry::face Face;
    typedef struct TwoDHalfEdgeGeometry::half_edge HalfEdge;

    struct vertex
    {
        unsigned int id;
        double x, y;
        struct TwoDHalfEdgeGeometry::half_edge *he;
    };

    struct half_edge
    {
        unsigned int id;
        struct TwoDHalfEdgeGeometry::vertex *vx;
        struct TwoDHalfEdgeGeometry::half_edge *nexthe;
        struct TwoDHalfEdgeGeometry::half_edge *prevhe;
        struct TwoDHalfEdgeGeometry::half_edge *twin;
        struct TwoDHalfEdgeGeometry::face *fa;
    };

    struct face
    {
        unsigned int id;
        struct TwoDHalfEdgeGeometry::half_edge *he;
    };

    unsigned int vx_id_ctr, he_id_ctr, fa_id_ctr;

    std::unordered_map<unsigned int, Vertex *> vx_unomap;
    std::unordered_map<unsigned int, HalfEdge *> he_unomap;
    std::unordered_map<unsigned int, Face *> fa_unomap;

    // Vetor para guardar todos os ponteiros de half-edge para o destrutor (essencial para evitar memory leaks)
    std::vector<HalfEdge*> all_half_edges;

public:
    TwoDHalfEdgeGeometry(const std::vector<double>& vxs_pos, const std::map<int, std::vector<int>>& fa_vxs)
    {
        vx_id_ctr = he_id_ctr = fa_id_ctr = 0;

        // 1. Criar todos os vértices
        for (size_t i = 0; i < vxs_pos.size(); i += 2)
        {
            Vertex *new_vx = new Vertex;
            new_vx->id = vx_id_ctr++;
            new_vx->x = vxs_pos[i];
            new_vx->y = vxs_pos[i + 1];
            new_vx->he = nullptr;
            vx_unomap[new_vx->id] = new_vx;
        }

        // Mapa para rastrear arestas já criadas e encontrar gêmeas (twins)
        std::map<std::pair<unsigned int, unsigned int>, HalfEdge*> edge_map;

        // 2. Iterar sobre as faces para criar arestas e faces
        for (const auto& [face_idx, vertex_indices] : fa_vxs)
        {
            if (vertex_indices.size() < 3) continue;

            Face *new_fa = new Face;
            new_fa->id = fa_id_ctr++;
            new_fa->he = nullptr;
            fa_unomap[new_fa->id] = new_fa;

            std::vector<HalfEdge*> face_hes;

            // 3. Criar ou encontrar as half-edges para a face atual
            for (size_t i = 0; i < vertex_indices.size(); ++i)
            {
                unsigned int v1_id = vertex_indices[i] - 1;
                unsigned int v2_id = vertex_indices[(i + 1) % vertex_indices.size()] - 1;

                if (vx_unomap.find(v1_id) == vx_unomap.end() || vx_unomap.find(v2_id) == vx_unomap.end()) {
                     throw std::runtime_error("Erro: Face " + std::to_string(face_idx) + " referencia um vertice invalido.");
                }

                std::pair<unsigned int, unsigned int> edge_key = {std::min(v1_id, v2_id), std::max(v1_id, v2_id)};
                
                auto it = edge_map.find(edge_key);
                HalfEdge* he1_ptr = nullptr;

                if (it == edge_map.end()) // Aresta não existe, criar nova
                {
                    HalfEdge* he1 = new HalfEdge();
                    HalfEdge* he2 = new HalfEdge();
                    all_half_edges.push_back(he1);
                    all_half_edges.push_back(he2);

                    he1->id = he_id_ctr++;
                    he2->id = he_id_ctr++;
                    he_unomap[he1->id] = he1;
                    he_unomap[he2->id] = he2;

                    he1->twin = he2; he2->twin = he1;
                    
                    he1->vx = vx_unomap.at(v2_id);
                    he2->vx = vx_unomap.at(v1_id);

                    if (vx_unomap.at(v1_id)->he == nullptr) vx_unomap.at(v1_id)->he = he1;
                    if (vx_unomap.at(v2_id)->he == nullptr) vx_unomap.at(v2_id)->he = he2;

                    he1->fa = nullptr; he2->fa = nullptr;
                    
                    edge_map[edge_key] = he1;
                    he1_ptr = he1;
                }
                else {
                    he1_ptr = it->second;
                }
                
                // Adiciona a half-edge correta para esta face (a que aponta de v1 para v2)
                if(he1_ptr->vx->id == v2_id) {
                    face_hes.push_back(he1_ptr);
                } else {
                    face_hes.push_back(he1_ptr->twin);
                }
            }
            
            // 4. Ligar os ponteiros next/prev e atribuir a face
            for(size_t i = 0; i < face_hes.size(); ++i)
            {
                HalfEdge* he_curr = face_hes[i];
                HalfEdge* he_prev = face_hes[(i + face_hes.size() - 1) % face_hes.size()];
                
                he_curr->prevhe = he_prev;
                he_prev->nexthe = he_curr;
                he_curr->fa = new_fa;
            }
            new_fa->he = face_hes[0];
        }
    }

    ~TwoDHalfEdgeGeometry()
    {
        for (auto he : all_half_edges) { delete he; }
        for (auto const& [id, fa] : fa_unomap) { delete fa; }
        for (auto const& [id, vx] : vx_unomap) { delete vx; }
    }

    /* Retorna um mapa com IDs de vértices e suas posições (x, y).
    */
    std::unordered_map<unsigned int, std::pair<double, double>> get_vertexes()
    {
        std::unordered_map<unsigned int, std::pair<double, double>> vxs;
        for (const auto& [id, vertex_ptr] : vx_unomap)
        {
            vxs[id] = std::make_pair(vertex_ptr->x, vertex_ptr->y);
        }
        return vxs;
    }

    /* Dado um ID de vértice, retorna os IDs das arestas incidentes.
    */
    std::vector<unsigned int> get_vx_edges_id(unsigned int vx_id)
    {
        std::vector<unsigned int> vx_edges_id;
        HalfEdge *he = vx_unomap.at(vx_id)->he;
        if (!he) return vx_edges_id;
        HalfEdge *start_he = he;
        do {
            vx_edges_id.push_back(he->id);
            he = he->twin->nexthe;
        } while (he != start_he && he != nullptr);
        return vx_edges_id;
    }

    /*Dado um ID de vértice, retorna os IDs das faces que o compartilham.*/
    std::vector<unsigned int> get_vx_faces_id(unsigned int vx_id)
    {
        std::vector<unsigned int> vx_faces_id;
        HalfEdge *he = vx_unomap.at(vx_id)->he;
        if (!he) return vx_faces_id;
        HalfEdge *start_he = he;
        do {
            if (he->fa != nullptr) {
                vx_faces_id.push_back(he->fa->id);
            }
            he = he->twin->nexthe;
        } while (he != start_he && he != nullptr);
        return vx_faces_id;
    }

    /*Dado um ID de face, retorna os IDs das faces adjacentes.*/
    std::vector<unsigned int> face_get_adjacent_faces_ids(unsigned int fa_id)
    {
        std::vector<unsigned int> adj_fas_id;
        HalfEdge *he = fa_unomap.at(fa_id)->he;
        if (!he) return adj_fas_id;
        HalfEdge *start_he = he;
        do {
            if (he->twin != nullptr && he->twin->fa != nullptr) {
                adj_fas_id.push_back(he->twin->fa->id);
            }
            he = he->nexthe;
        } while (he != start_he && he != nullptr);
        return adj_fas_id;
    }

    /*Dado um ID de aresta, retorna os IDs das faces adjacentes.*/
    std::vector<unsigned int> edge_get_adjacent_faces_ids(unsigned int he_id)
    {
        std::vector<unsigned int> adj_fas_id;
        HalfEdge *he = he_unomap.at(he_id);
        if (he->fa) { adj_fas_id.push_back(he->fa->id); }
        if (he->twin && he->twin->fa) { adj_fas_id.push_back(he->twin->fa->id); }
        return adj_fas_id;
    }

    /*Retorna um mapa entre um par de IDs de vértices (chave) e o ID da aresta de conexão (valor).*/
    std::map<std::pair<unsigned int, unsigned int>, unsigned int>
    get_vxs_conn_edges_id()
    {
        std::map<std::pair<unsigned int, unsigned int>, unsigned int> vxs_conn_edges_id;
        for (const auto& he : all_half_edges) {
            // Garante que a aresta só seja adicionada uma vez
            if (he->twin->vx->id < he->vx->id) {
                vxs_conn_edges_id[std::make_pair(he->twin->vx->id, he->vx->id)] = he->id;
            }
        }
        return vxs_conn_edges_id;
    }

    std::map<unsigned int, std::vector<unsigned int>> get_faces_with_vertices()
    {
        std::map<unsigned int, std::vector<unsigned int>> faces_map;
        for (const auto& [face_id, face_ptr] : fa_unomap) {
            if (!face_ptr || !face_ptr->he) continue;

            std::vector<unsigned int> vertex_ids;
            half_edge *start_he = face_ptr->he;
            half_edge *current_he = start_he;

            do {
                if (current_he && current_he->vx) {
                     vertex_ids.push_back(current_he->vx->id);
                }
                current_he = current_he->nexthe;
            } while (current_he != start_he && current_he != nullptr);

            faces_map[face_id] = vertex_ids;
        }
        return faces_map;
    }
};

#endif

