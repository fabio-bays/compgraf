#include "2dgeometry.h"
#include <vector>

// Save the vertexes. Then, with the faces, generate the half-edges
class TwoDHalfEdgeGeometry
{
    struct vertex
    {
        int x, y;
        struct half_edge *he;
    };

    struct half_edge
    {
        struct vertex *ve;
        struct half_edge *nexthe;
        struct half_edge *prevhe;
        struct half_edge *twin;
        struct face *fa;
    };

    struct face
    {
        struct half_edge *he;
    };

    /* Half-edges vector stores just one of each half-edges pair. The vector indexes
    acts as the edge ID*/
    std::vector<struct vertex *> vx_list;
    std::vector<struct half_edge *> he_list; 
    std::vector<struct face *> fa_list;

    void new_vertex(int x, int y)
    {
        struct vertex *new_vx = new struct vertex;
        new_vx->x = x;
        new_vx->y = y;
        new_vx->he = nullptr;

        vx_list.push_back(new_vx);
    }

    void new_half_edge(struct vertex **point_to_this)
    {
        struct half_edge *new_he = new struct half_edge;
        new_he->ve = *point_to_this;
    }

    void new_face(int vx_idx)
    {
        struct face *new_fa = new struct face;
        new_fa->he = nullptr;

        fa_list.push_back(new_fa);
    }

    /* 1. Save the vertexes; 2. Access the vertex by indexing the vector;
    3. Create half-edge if not already created and save it; 4. Create its twin
    5. Return to step 2 while there are remaining vertexes*/
    TwoDHalfEdgeGeometry()
    {

    }


};