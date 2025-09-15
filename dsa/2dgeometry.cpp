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
        struct face *fa;
    };

    struct face
    {
        struct half_edge *he;
    };

    std::vector<struct vertex> vx_list;
    std::vector<struct half_edge> he_list; // Will store just one of the half-edges pair
    std::vector<struct face> fa_list;

    void new_vertex(int x, int y)
    {
        
    }

    void new_face(int vx_idx)


};