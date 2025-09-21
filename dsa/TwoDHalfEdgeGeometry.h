#ifndef TWOD_GEOMETRY_DEFINED_H
#define TWOD_GEOMETRY_DEFINED_H
#include <vector>
#include <unordered_map>
#include <map>
#include <utility>
#include <stdexcept> // Necessário para std::runtime_error
#include <string>    // Necessário para std::to_string
#include <iostream>  // Para depuração

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
        bool is_a_twin;
        struct TwoDHalfEdgeGeometry::vertex *vx; // Vértice para o qual a aresta aponta
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
    
    std::unordered_map<int, Vertex *> vx_unomap;
    std::unordered_map<int, HalfEdge *> he_unomap;
    std::unordered_map<int, Face *> fa_unomap;

    // --- MUDANÇA 1: Vetor para limpeza segura ---
    std::vector<HalfEdge*> all_hes_for_cleanup;

    void new_vertex(double x, double y)
    {
        Vertex *new_vx = new Vertex;
        new_vx->id = vx_id_ctr++;
        new_vx->x = x;
        new_vx->y = y;
        new_vx->he = nullptr;
        vx_unomap[new_vx->id] = new_vx; 
    }

    HalfEdge *new_half_edge(bool twin = false)
    {
        HalfEdge *new_he = new HalfEdge;
        // --- MUDANÇA 2: Adicionar TODA half-edge criada ao vetor de limpeza ---
        all_hes_for_cleanup.push_back(new_he);

        new_he->vx = nullptr;
        new_he->fa = nullptr;
        new_he->nexthe = nullptr;
        new_he->prevhe = nullptr;
        new_he->twin = nullptr;
        if (!twin) 
        {
            new_he->id = he_id_ctr++;
            he_unomap[new_he->id] = new_he; 
        } else {
            new_he->id = he_id_ctr++; 
        }
        return new_he;
    }

    void new_face()
    {
        Face *new_fa = new Face;
        new_fa->id = fa_id_ctr++;
        new_fa->he = nullptr;
        fa_unomap[new_fa->id] = new_fa; 
    }
    
    HalfEdge *get_vxconnection_he(Vertex *v1, Vertex *v2)
    {
        if(v1->he == nullptr) return nullptr;
        
        HalfEdge* start_he = v1->he;
        HalfEdge* current_he = start_he;
        do {
            if (current_he->vx == v2) return current_he;
            current_he = current_he->prevhe->twin;
        } while(current_he != start_he && current_he != nullptr);

        return nullptr;
    }

    public:

    TwoDHalfEdgeGeometry(std::vector<double> vxs_pos, std::map<int, std::vector<int>> fa_vxs)
    {
        vx_id_ctr = he_id_ctr = fa_id_ctr = 0;

        for (size_t i = 0; i < vxs_pos.size(); i += 2)
        {
            new_vertex(vxs_pos[i], vxs_pos[i + 1]);
        }
        
        std::map<std::pair<unsigned int, unsigned int>, HalfEdge*> edge_map;

        for (auto const& [face_raw_id, vertex_indices] : fa_vxs)
        {
            new_face();
            Face* current_face = fa_unomap[fa_id_ctr - 1];
            
            std::vector<HalfEdge*> current_face_hes;

            for (size_t i = 0; i < vertex_indices.size(); ++i)
            {
                unsigned int p1_idx = vertex_indices[i] - 1;
                unsigned int p2_idx = vertex_indices[(i + 1) % vertex_indices.size()] - 1;

                if (vx_unomap.find(p1_idx) == vx_unomap.end() || vx_unomap.find(p2_idx) == vx_unomap.end()) {
                    throw std::runtime_error("Indice de vertice invalido em uma face.");
                }

                Vertex* v1 = vx_unomap[p1_idx];
                Vertex* v2 = vx_unomap[p2_idx];
                
                HalfEdge* he = new_half_edge();
                he->vx = v2;
                he->fa = current_face;
                v1->he = he;

                edge_map[{p1_idx, p2_idx}] = he;
                current_face_hes.push_back(he);
            }

            for (size_t i = 0; i < current_face_hes.size(); ++i)
            {
                HalfEdge* current_he = current_face_hes[i];
                HalfEdge* next_he = current_face_hes[(i + 1) % current_face_hes.size()];
                current_he->nexthe = next_he;
                next_he->prevhe = current_he;
            }
            current_face->he = current_face_hes[0];
        }

        for (auto const& [edge_pair, he] : edge_map)
        {
            if (he->twin != nullptr) continue;

            unsigned int v1_idx = edge_pair.first;
            unsigned int v2_idx = edge_pair.second;

            auto twin_it = edge_map.find({v2_idx, v1_idx});
            if (twin_it != edge_map.end()) {
                HalfEdge* twin_he = twin_it->second;
                he->twin = twin_he;
                twin_he->twin = he;
            } else {
                HalfEdge* twin_he = new_half_edge(true);
                he->twin = twin_he;
                twin_he->twin = he;
                twin_he->vx = vx_unomap[v1_idx];
            }
        }
    }

    // --- MUDANÇA 3: Destrutor corrigido ---
    ~TwoDHalfEdgeGeometry()
    {
        // Deleta todas as half-edges de forma segura
        for (HalfEdge* he_ptr : all_hes_for_cleanup) {
            delete he_ptr;
        }
        // Deleta faces e vértices (isso já estava correto)
        for(auto const& [key, fa] : fa_unomap) delete fa;
        for(auto const& [key, vx] : vx_unomap) delete vx;
    }

    /* O resto das funções permanece igual */

    std::unordered_map<int, std::pair<double, double>> get_vertexes()
    {
        std::unordered_map<int, std::pair<double, double>> vxs;
        for(auto const& [id, vx_ptr] : vx_unomap)
        {
            vxs[id] = std::make_pair(vx_ptr->x, vx_ptr->y);
        }
        return vxs;
    }

    std::vector<unsigned int> get_vx_edges_id(unsigned int vx_id)
    {
        std::vector<unsigned int> vx_edges_id;
        if (vx_unomap.find(vx_id) == vx_unomap.end()) return vx_edges_id;
        HalfEdge *start_he = vx_unomap.at(vx_id)->he;
        if (start_he == nullptr) return vx_edges_id;
        HalfEdge *current_he = start_he;
        do
        {
            if (he_unomap.count(current_he->id)) {
                vx_edges_id.push_back(current_he->id);
            } else if (current_he->twin != nullptr && he_unomap.count(current_he->twin->id)) {
                vx_edges_id.push_back(current_he->twin->id);
            }
            current_he = current_he->prevhe->twin;
        } while (current_he != start_he && current_he != nullptr);
        return vx_edges_id;
    }

    std::vector<unsigned int> get_vx_faces_id(unsigned int vx_id)
    {
        std::vector<unsigned int> vx_faces_id;
        if (vx_unomap.find(vx_id) == vx_unomap.end()) return vx_faces_id;
        HalfEdge *start_he = vx_unomap.at(vx_id)->he;
        if (start_he == nullptr) return vx_faces_id;
        HalfEdge *current_he = start_he;
        do
        {
            if (current_he->fa != nullptr)
            {
                vx_faces_id.push_back(current_he->fa->id);
            }
            current_he = current_he->prevhe->twin;
        } while (current_he != start_he && current_he != nullptr);
        return vx_faces_id;
    }

    std::vector<unsigned int> face_get_adjacent_faces_ids(unsigned int fa_id)
    {
        std::vector<unsigned int> adj_fas_id;
        if (fa_unomap.find(fa_id) == fa_unomap.end()) return adj_fas_id;
        HalfEdge *start_he = fa_unomap.at(fa_id)->he;
        if (start_he == nullptr) return adj_fas_id;
        HalfEdge *current_he = start_he;
        do
        {
            if(current_he->twin != nullptr && current_he->twin->fa != nullptr)
            {
                adj_fas_id.push_back(current_he->twin->fa->id);
            }
            current_he = current_he->nexthe;
        } while (current_he != start_he);
        return adj_fas_id;
    }

    std::vector<unsigned int> edge_get_adjacent_faces_ids(unsigned int he_id)
    {
        std::vector<unsigned int> adj_fas_id;
        if (he_unomap.find(he_id) == he_unomap.end()) return adj_fas_id;
        HalfEdge *he = he_unomap.at(he_id);
        if (he->fa != nullptr) adj_fas_id.push_back(he->fa->id);
        if(he->twin != nullptr && he->twin->fa != nullptr)
        {
            adj_fas_id.push_back(he->twin->fa->id);
        }    
        return adj_fas_id;
    }

    std::map<std::pair<unsigned int, unsigned int>, unsigned int>
    get_vxs_conn_edges_id()
    {
        std::map<std::pair<unsigned int, unsigned int>, unsigned int> vxs_conn_edges_id;
        for (auto const& [key, he] : he_unomap) {
            if (he != nullptr && he->twin != nullptr && he->twin->vx != nullptr && he->vx != nullptr) {
                unsigned int from_id = he->twin->vx->id;
                unsigned int to_id = he->vx->id;
                vxs_conn_edges_id[std::make_pair(from_id, to_id)] = he->id;
            }
        }
        return vxs_conn_edges_id;
    }
};

#endif

