#include "2dgeometry.h"
#include <vector>
#include <unordered_map>
#include <map>
#include <utility>

// Save the vertexes. Then, with faces, generate half-edges
class TwoDHalfEdgeGeometry
{
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
    
    /* Half-edges unordered map stores just one of each half-edges pair of an edge.*/
    std::unordered_map<int, Vertex *> vx_unomap;
    std::unordered_map<int, HalfEdge *> he_unomap;
    std::unordered_map<int, Face *> fa_unomap;

    void new_vertex(int x, int y)
    {
        Vertex *new_vx = new Vertex;
        new_vx->id = vx_id_ctr++;
        new_vx->x = x;
        new_vx->y = y;
        new_vx->he = nullptr;
        vx_list[new_vx->id] = new_vx; 
    }

    HalfEdge *new_half_edge(Vertex **point_to_thisvx, Face **point_to_thisfa)
    {
        HalfEdge *new_he = new HalfEdge;
        new_he->id = he_id_ctr++;
        new_he->vx = *point_to_thisvx;
        new_he->fa = *point_to_thisfa;
        new_he->nexthe = nullptr;
        new_he->prevhe = nullptr;
        new_he->twin = nullptr;
        he_list[new_he->id] = new_he; 

        return new_he;
    }

    void new_face()
    {
        Face *new_fa = new Face;
        new_fa->id = fa_id_ctr++;
        new_fa->he = nullptr;
        fa_list[new_fa->id] = new_fa; 
    }

    /* Check if vertex v1 half-edge points to v2. If not, checks if
    vertex v1 half-edge's twin's next points to v2. */
    bool are_vx_connected(Vertex *v1, Vertex *v2)
    {
        if(v1->he == nullptr) return false;
        else if(v1->he->vx == v2 or
                v1->he->twin->nexthe->vx == v2) return true;
    }

    /* 1. Save the vertexes; 2. For each face, 

    Observations:
    - Vertex N position is given in indexes N-1 (x) and N (y) in vector
    */
    TwoDHalfEdgeGeometry(std::vector<double> vxs_pos, std::map<int, std::vector<int>> fa_vxs)
    {
        vx_id_ctr = he_id_ctr = fa_id_ctr = 0; // Used to associate the components with an ID

        for (auto itr = vxs_pos.begin(); itr != vxs_pos.end(); itr += 2)
        {
            new_vertex(*itr, *(itr + 1));
        }
        
        for (auto itr = fa_vxs.begin(); itr != fa_vxs.end(); itr++)
        {
            new_face();
            Face *hes_face = fa_unomap.at(itr->first - 1);
            std::map<std::pair<int, int>, HalfEdge*> face_new_hes;
            
            for (auto vxs_itr = itr->second.begin(); vxs_itr != itr->second.end();
                vxs_itr++)
            {
                Vertex *actual_vx = vx_unomap.at(*vxs_itr - 1);
                if(!are_vx_connected(actual_vx, vx_unomap.at(*vxs_itr)))
                {
                    int vx_id_point_to = 
                        vxs_itr + 1 == itr->second.end() ? *itr->second.begin() - 1 : *(vxs_itr + 1) - 1;
                    Vertex *point_to = vx_unomap.at(vx_id_point_to);

                    // The half-edge that begins at the current iteration vertex
                    HalfEdge *actual_vx_he = new_half_edge(nullptr, nullptr); 
                    actual_vx_he->fa = hes_face;
                    actual_vx_he->vx = point_to;

                    actual_vx_he->twin = new_half_edge(&actual_vx, nullptr);
                    actual_vx_he->twin->twin = actual_vx_he;
                    actual_vx_he->twin->vx = actual_vx;

                    actual_vx->he = actual_vx_he;
                    
                    std::pair<int, int> p = std::pair<int, int>(*vxs_itr - 1, vx_id_point_to);
                    face_new_hes.insert(std::make_pair(p, actual_vx_he));
                } else {
                    // The twin half edge which has a null face will now point to a face.
                    actual_vx->he->twin->fa = hes_face;
                }
                
            }
            for (auto vxs_itr = itr->second.begin(); vxs_itr != itr->second.end();
                vxs_itr++)
            {
                // Set the previous and next half-edges for each newly created half-edge.
                Vertex *actual_vx = vx_unomap.at(*vxs_itr - 1);
                HalfEdge *actual_he = actual_vx->he;
                
                if(actual_he->nexthe == nullptr)
                {
                    // If nexthe was unset, also was prevhe 
                    actual_he->nexthe = actual_he->vx->he;
                    actual_he->prevhe = actual_he->twin->vx->he;
                }
            }
        }
    }
};