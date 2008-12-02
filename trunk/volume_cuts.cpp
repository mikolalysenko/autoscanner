#include <boost/config.hpp>

//STL includes
#include <vector>
#include <iostream>
#include <cassert>
#include <cmath>

//Boost stuff
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/edmunds_karp_max_flow.hpp>
#include <boost/tuple/tuple.hpp>

//Project
#include "misc.h"
#include "view.h"
#include "volume.h"
#include "volume_cuts.h"

using namespace std;
using namespace blitz;
using namespace boost;

//Includes from photohull
extern const vec3 DN[6], DU[6], DV[6];
extern const int N_DIM[6], U_DIM[6], V_DIM[6];

//Lambda coefficient
#define LAMBDA          0.1f
#define INF             1.0e20f

//Aliases for boost graph data type
typedef adjacency_list_traits < vecS, vecS, directedS > Traits;
typedef adjacency_list < vecS, vecS, directedS, no_property,
    property < edge_capacity_t, float,
    property < edge_residual_capacity_t, float,
    property < edge_reverse_t, Traits::edge_descriptor > > > > Graph;


//Allocate flow network graph
Graph *g;

//Graph property accessors
property_map < Graph, edge_capacity_t >::type
    capacity = get(edge_capacity, *g);
property_map < Graph, edge_reverse_t >::type
    rev = get(edge_reverse, *g);
property_map < Graph, edge_residual_capacity_t >::type
    residual_capacity = get(edge_residual_capacity, *g);


//Volume dimensions
ivec3 grid_dim;

//Node coordinates
int sourceNode, sinkNode;
int getNode(int x, int y, int z)
{
    assert(x >= 0 && x < grid_dim(0) &&
        y >= 0 && x < grid_dim(1) &&
        z >= 0 && x < grid_dim(2));
    
    return x + grid_dim(0) * (y + grid_dim(1) * z);
}

//Evaluate photoconsistency of a given grid point
float evaluatePhotoConsistency(vector<View*> views, vec3 p, Volume * hull)
{
    return 1.0f;
}

void addEdge(int start, int end, float w)
{
    Traits::vertex_descriptor u, v;
    u = vertex(start, *g);
    v = vertex(end, *g);
 
    //Check for edge existence
    Traits::edge_descriptor e1, e2;
    bool found;
    tie(e1, found) = edge(u, v, *g);
    if(found)
        return;
    
    //Add edge to graph
    add_edge(u, v, *g);
    add_edge(v, u, *g);
    
    //Locate edges
    tie(e1, found) = edge(u, v, *g);
    tie(e2, found) = edge(v, u, *g);
    
    //Set properties
    capacity[e1] = w;
    capacity[e2] = w;
    residual_capacity[e1] = w;
    residual_capacity[e2] = w;
    rev[e2] = e1;
    rev[e1] = e1;
}
    


//Perform volumetric graph cuts
Volume* volumetricGraphCuts(
    std::vector<View*> views,
    vec3 box_min,
    vec3 box_max,
    ivec3 grid_res,
    vec3 interior_point,
    Volume * photo_hull)
{
    //Compute grid cell dimensions
    vec3 h = box_max;
    h -= box_min;
    h /= grid_res;

    
    //Set grid dimensions
    grid_dim = grid_res;

    //Allocate graph
    g = new Graph(grid_dim(0) * grid_dim(1) * grid_dim(2) + 1);
    
    //Find source
    ivec3 interior_grid;
    for(int i=0; i<3; i++)
    {
        interior_grid(i) = 
            grid_res(i) * (interior_point(i) - box_min(i)) / (box_max(i) - box_min(i));
    }
    sourceNode = getNode(interior_grid(0), interior_grid(1), interior_grid(2));
    
    //Find sink
    sinkNode = -1;
    
    //Build graph
    for(int i=0; i<grid_dim(0); i++)
    for(int j=0; j<grid_dim(1); j++)
    for(int k=0; k<grid_dim(2); k++)
    {
        int v = getNode(i, j, k);
        
        //Check for edge cells
        if(photo_hull->on_surface(ivec3(i, j, k)))
        {
            if(sinkNode == -1)
                sinkNode = v;
            else
                addEdge(v, sinkNode, INF);
            
            continue;
        }
        
        //Add photoconsistency criteria
        for(int d=0; d<6; d++)
        {
            vec3 pt = DN[d];
            pt *= 0.5;
            pt += vec3(i,j,k);
            
            float rho = evaluatePhotoConsistency(views, pt, photo_hull);
            
            int u = getNode(i+DN[d](0), j+DN[d](1), k+DN[d](2));
            
            float weight = 4.0f / 3.0f * M_PI * h(U_DIM[d]) * h(V_DIM[d]) * rho;
            addEdge(v, u, weight);
        }
        
        
        //Add inflation term
        if(v != sourceNode)
        {
            float w_b = LAMBDA * h(0) * h(1) * h(2);
            addEdge(v, sourceNode, w_b);
        }
    }
    
    //Allocate volume
    Volume * result = new Volume(grid_dim(0), grid_dim(1), grid_dim(2));

    //Allocate predecessor list
    vector<Traits::edge_descriptor> pred(num_vertices(*g));
    
    //Do min-cuts on graph to get result
    edmunds_karp_max_flow(
        *g, 
        sourceNode, 
        sinkNode, 
        capacity, 
        residual_capacity, 
        rev,
        result->data,
        &pred[0]);
    
    
    //Release graph
    delete g;
    
    //Return result
    return result;
}

