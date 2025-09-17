#include "2dgeometry.h"
#include <vector>
#include <unordered_map>
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

    void new_half_edge(Vertex **point_to_thisvx, Face **point_to_thisfa)
    {
        HalfEdge *new_he = new HalfEdge;
        new_he->id = he_id_ctr++;
        new_he->vx = *point_to_thisvx;
        new_he->fa = *point_to_thisfa;
        new_he->nexthe = nullptr;
        new_he->prevhe = nullptr;
        new_he->twin = nullptr;
        he_list[new_he->id] = new_he; 
    }

    void new_face()
    {
        Face *new_fa = new Face;
        new_fa->id = fa_id_ctr++;
        new_fa->he = nullptr;
        fa_list[new_fa->id] = new_fa; 
    }

    /* 1. Save the vertexes; 2. Access the vertex by indexing the vector;
    3. Create half-edge if not created;
    4. Create its twin if not created;
    5. If the half-edges are already exists, check if they need to be edited
    (eg.: a new face using a vertex whose half-edges were already created)
    6. Return to step 2 while there are remaining vertexes

    Observations:
    - Vertex N position is given in indexes N-1 (x) and N (y) in vector
    */
    TwoDHalfEdgeGeometry(std::vector<double> vxs_pos, std::map<int, std::vector<int>> fa_vxs)
    {
        vx_id_ctr = he_id_ctr = fa_id_ctr = 0;

        for (auto itr = vxs_pos.begin(); itr != vxs_pos.end(); itr += 2)
        {
            new_vertex(*itr, *(itr + 1));
        }
        
        for (auto itr = fa_vxs.begin(); itr != fa_vxs.end(); itr++)
        {
            new_face();

            for(int i = 0; i < itr->second.size(); i++)
            {
                new_half_edge(nullptr, nullptr);
            }

            
            for (auto vxs_itr = itr->second.begin(); vxs_itr != itr->second.end(); vxs_itr++)
            {
                Vertex *point_to = vx_unomap.at(
                    vxs_itr + 1 == itr->second.end() ? *itr->second.begin() - 1 : *(vxs_itr + 1) - 1);
                Face *hes_face = fa_unomap.at(itr->first - 1);
                Vertex *actual_vx = vx_unomap.at(*vxs_itr - 1);

                // The half-edge that starts at the current iteration vertex
                HalfEdge *actual_vx_he = he_unomap.at(); // -> Use a half edge counter (0, 1, 2...)
            }

            // for (auto vxs_itr = itr->second.begin(); vxs_itr != itr->second.end(); vxs_itr++)
            // {
            //     Vertex *point_to = vx_list.at(
            //         vxs_itr + 1 == itr->second.end() ? *itr->second.begin() : *(vxs_itr + 1) - 1);
            //     Face *he_faces = fa_list.at(itr->first);
            //     Vertex *actual_vx = vx_list.at(*vxs_itr - 1);

            //     new_half_edge(&point_to,
            //                  &he_faces);
                
            //     HalfEdge *last_he = he_list.back();
            //     actual_vx->he = last_he;
            //     // Twin of the newly created half-edge:
            //     new_half_edge(&actual_vx, nullptr);
                
            // }
        }

    }


};