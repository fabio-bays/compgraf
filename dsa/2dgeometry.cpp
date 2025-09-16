#include "2dgeometry.h"
#include <vector>
#include <map>

// Save the vertexes. Then, with the faces, generate the half-edges
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
        double x, y;
        struct TwoDHalfEdgeGeometry::half_edge *he;
    };

    struct half_edge
    {
        struct TwoDHalfEdgeGeometry::vertex *vx;
        struct TwoDHalfEdgeGeometry::half_edge *nexthe;
        struct TwoDHalfEdgeGeometry::half_edge *prevhe;
        struct TwoDHalfEdgeGeometry::half_edge *twin;
        struct TwoDHalfEdgeGeometry::face *fa;
    };

    struct face
    {       
        struct TwoDHalfEdgeGeometry::half_edge *he;
    };

    /* Half-edges vector stores just one of each half-edges pair. The vector indexes
    acts as the edge ID*/
    std::vector<Vertex *> vx_list;
    std::vector<HalfEdge *> he_list; 
    std::vector<Face *> fa_list;

    void new_vertex(int x, int y)
    {
        Vertex *new_vx = new Vertex;
        new_vx->x = x;
        new_vx->y = y;
        new_vx->he = nullptr;

        vx_list.push_back(new_vx);
    }

    void new_half_edge(struct TwoDHalfEdgeGeometry::vertex **point_to_thisvx, struct TwoDHalfEdgeGeometry::face **point_to_thisfa)
    {
        struct TwoDHalfEdgeGeometry::half_edge *new_he = new struct TwoDHalfEdgeGeometry::half_edge;
        new_he->vx = *point_to_thisvx;
        new_he->fa = *point_to_thisfa;
        new_he->nexthe = nullptr;
        new_he->prevhe = nullptr;
        new_he->twin = nullptr;

        he_list.push_back(new_he);
    }

    void new_face(int vx_idx)
    {
        struct face *new_fa = new struct face;
        new_fa->he = nullptr;

        fa_list.push_back(new_fa);
    }

    /* 1. Save the vertexes; 2. Access the vertex by indexing the vector;
    3. Create half-edge if not created;
    4. Create its twin if not created;
    5. If the half-edges are already exists, check if they need to be edited
    (eg.: a new face using a vertex whose half-edges were already created)
    6. Return to step 2 while there are remaining vertexes

    Observations:
    - Face N is mapped to key N-1 in faces map
    */
    TwoDHalfEdgeGeometry(std::vector<double> vxs_pos, std::map<int, std::vector<int>> fa_vxs)
    {
        for (auto itr = vxs_pos.begin(); itr != vxs_pos.end(); itr += 2)
        {
            new_vertex(*itr, *(itr + 1));
        }
        
        for (auto itr = fa_vxs.begin(); itr != fa_vxs.end(); itr++)
        {
            for (auto vxs_itr = itr->second.begin(); vxs_itr != itr->second.end(); vxs_itr++)
            {
                struct vertex *point_to = vx_list.at(
                    vxs_itr + 1 == itr->second.end() ? *itr->second.begin() : *(vxs_itr + 1) - 1);
                struct face *he_faces = fa_list.at(itr->first);
                struct vertex *actual_vx = vx_list.at(*vxs_itr - 1);

                new_half_edge(&point_to,
                             &he_faces);
                
                struct TwoDHalfEdgeGeometry::half_edge *last_he = he_list.back();
                actual_vx->he = last_he;
                // Newly created half-edge's twin:
                new_half_edge(&actual_vx, nullptr);
                
            }
        }

    }


};