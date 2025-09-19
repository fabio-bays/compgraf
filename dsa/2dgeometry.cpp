// #include "2dgeometry.h"
#include <vector>
#include <unordered_map>
#include <map>
#include <utility>
#include <iostream>

// Save the vertexes. Then, with faces, generate half-edges
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
        vx_unomap[new_vx->id] = new_vx; 
    }

    HalfEdge *new_half_edge(bool twin = false)
    {
        HalfEdge *new_he = new HalfEdge;
        new_he->vx = nullptr;
        new_he->fa = nullptr;
        new_he->nexthe = nullptr;
        new_he->prevhe = nullptr;
        new_he->twin = nullptr;
        if (!twin) 
        {
            new_he->id = he_id_ctr++;
            he_unomap[new_he->id] = new_he; 
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

    /* Check if vertex v1 half-edge points to v2. If not, checks if
    vertex v1 half-edge's twin's next points to v2. */
    HalfEdge *get_vxconnection_he(Vertex *v1, Vertex *v2)
    {
        if(v1->he == nullptr) return nullptr;
        else if(v1->he->vx == v2) return v1->he;
        else if (v1->he->twin->nexthe->vx == v2) return v1->he->twin->nexthe;
        else return nullptr;
    }

    public:

    /*
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
                int vx_id_point_to = 
                    vxs_itr + 1 == itr->second.end() ? *itr->second.begin() - 1 : *(vxs_itr + 1) - 1;
                Vertex *point_to = vx_unomap.at(vx_id_point_to);
                if((!get_vxconnection_he(actual_vx, point_to)))
                {
                    // The half-edge that begins at the current iteration vertex
                    HalfEdge *actual_vx_he = new_half_edge(); 
                    actual_vx_he->fa = hes_face;
                    actual_vx_he->vx = point_to;

                    actual_vx_he->twin = new_half_edge(true);
                    actual_vx_he->twin->vx = actual_vx;
                    actual_vx_he->twin->twin = actual_vx_he;

                    actual_vx->he = actual_vx_he;
                    
                    std::pair<int, int> p = std::pair<int, int>(*vxs_itr - 1, vx_id_point_to);
                    face_new_hes[p] = actual_vx_he;
                } else {
                    // The twin half-edge which has a null face will now point to a face.
                    actual_vx->he->twin->fa = hes_face;
                }
                
            }
            for (auto vxs_itr = itr->second.begin(); vxs_itr != itr->second.end();
                vxs_itr++)
            {
               int actual_vx_id, vx1_id, vx2_id;
               if (vxs_itr + 1 == itr->second.end())
               {
                    vx1_id = *itr->second.begin() - 1;
                    vx2_id = *(itr->second.begin() + 1) - 1;
               }
               else if(vxs_itr + 2 == itr->second.end())
               {
                    vx1_id = *(vxs_itr + 1) - 1;
                    vx2_id = *itr->second.begin() - 1;
               } else {
                    vx1_id = *(vxs_itr + 1) - 1;
                    vx2_id = *(vxs_itr + 2) - 1;
               }
               actual_vx_id = *vxs_itr - 1;

               HalfEdge *h1 = get_vxconnection_he(vx_unomap.at(actual_vx_id), vx_unomap.at(vx1_id));
               HalfEdge *h2 = get_vxconnection_he(vx_unomap.at(vx1_id), vx_unomap.at(vx2_id));

               if (h1->nexthe == nullptr)
               {
                    h1->nexthe = h2;
                    if (h2->prevhe == nullptr)
                    {
                        h2->prevhe = h1;
                        h1->twin->prevhe = h2->twin;
                        h2->twin->nexthe = h1->twin;
                    } else{
                        h1->twin->prevhe = h2->prevhe;
                        h2->prevhe->nexthe = h1->twin;
                        h2->prevhe = h1;
                    }
               } else{
                    h2->twin->nexthe = h1->nexthe;
                    h1->nexthe->prevhe = h2->twin;
                    h1->nexthe = h2;
                    h2->prevhe = h1;
               }
                
            }

            hes_face->he = face_new_hes.begin()->second; // Face half-edge reference is arbitrary
        }
    }

    ~TwoDHalfEdgeGeometry()
    {
        for(auto fa_itr = fa_unomap.begin(); fa_itr != fa_unomap.end(); fa_itr++)
        {
            delete fa_itr->second;
        }
        for(auto he_itr = he_unomap.begin(); he_itr != he_unomap.end(); he_itr++)
        {
            delete he_itr->second->twin;
            delete he_itr->second;
        }
        for(auto vx_itr = vx_unomap.begin(); vx_itr != vx_unomap.end(); vx_itr++)
        {
            delete vx_itr->second;
        }
    }

    /* 
    Returns an unordered map with keys being the vertexes IDs and values being their position (x, y).
    */
    std::unordered_map<int, std::pair<int, int>> get_vertexes()
    {
        std::unordered_map<int, std::pair<int, int>> vxs;

        for(auto itr = vx_unomap.begin(); itr != vx_unomap.end(); itr++)
        {
            vxs[itr->second->id] = std::make_pair(itr->second->x, itr->second->y);
        }

        return vxs;
    }

};

int main(){
    /*obj:
    v 1 1
    v 2 1
    v 3 2
    v 5 1
    f 2 1 3
    f 2 3 4*/
    std::vector<double> vxpos = {1, 1, 2, 1, 3, 2, 5, 1};
    std::map<int, std::vector<int>> famap;
    famap[1] = {2, 1, 3};
    famap[2] = {2, 3, 4};
    TwoDHalfEdgeGeometry geo = TwoDHalfEdgeGeometry(vxpos, famap);

    auto vxs = geo.get_vertexes();
    for(auto i = vxs.begin(); i != vxs.end(); i++)
    {
        std::cout << i->first << " pos: (" << i->second.first << ", " << i->second.second << ")\n";
    }
    std::cout << "deu bom!";

    return 0;
}