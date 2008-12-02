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
extern const vec3 DN[], DU[], DV[];
extern const int N_DIM[], U_DIM[], V_DIM[];

//Lambda coefficient
#define LAMBDA          0.1f
#define INFINITY        1.0e20f

//Aliases for boost graph data type
typedef adjacency_list_traits < vecS, vecS, directedS > Traits;
typedef adjacency_list < vecS, vecS, directedS,
    property < edge_capacity_t, float,
    property < edge_residual_capacity_t, float,
    property < edge_reverse_t, Traits::edge_descriptor > > > > Graph;


//Allocate flow network graph
Graph *g;

//Graph property accessors
property_map < Graph, edge_capacity_t >::type
    capacity = get(edge_capacity, g);
property_map < Graph, edge_reverse_t >::type
    rev = get(edge_reverse, g);
property_map < Graph, edge_residual_capacity_t >::type
    residual_capacity = get(edge_residual_capacity, g);


//Volume dimensions
ivec3 grid_dim;

//Node coordinates
int source, sink;
int getNode(int x, int y, int z)
{
    assert(x >= 0 && x < grid_dim(0) &&
        y >= 0 && x < grid_dim(1) &&
        z >= 0 && x < grid_dim(2));
    
    return x + grid_dim(0) * (y + grid_dim(1) * z);
}

//Evaluate photoconsistency of a given grid point
float evaluatePhotoConsistency(vector<View*> views, vec4 p, Volume * hull)
{
    return 1.0f;
}

void addEdge(int start, int end, float w)
{
    for(Graph::out_edge_iterator it = 
    
    graph_traits<Graph>::edge_descriptor 
        e0 = add_edge(start, end, *g),
        e1 = add_edge(end, start, *g);
    
    capacity[e0] = capacity[e1] = w;
    residual_capacity[e0] = residual_capacity[e1] = w;
    rev[e0] = e1;
    rev[e1] = e0;
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
        interior_gird(i) = 
            grid_res(i) * (interior_point(i) - box_min(i)) / (box_max(i) - box_min(i));
    }
    source = getNode(interior_grid(0), interior_grid(1), interior_grid(2));
    
    //Find sink
    sink = -1;
    
    //Build graph
    for(int i=0; i<grid_dim(0); i++)
    for(int j=0; j<grid_dim(1); j++)
    for(int k=0; k<grid_dim(2); k++)
    {
        int v = getNode(i, j, k);
        
        //Check for edge cells
        if(photo_hull->on_surface(i, j, k))
        {
            if(sink == -1)
                sink = v;
            else
                addEdge(v, sink, INFINITY);
            
            continue;
        }
        
        //Add photoconsistency criteria
        for(int d=0; d<6; d++)
        {
            vec3 pt = DN[d];
            pt *= 0.5;
            pt += vec3(i,j,k);
            
            float rho = evaluatePhotoConsistency(views, pt, hull);
            
            int u = getNode(i+DN[d](0), j+DN[d](1), k+DN[d](2));
            
            float weight = 4.0f / 3.0f * M_PI * h(U_DIMS[d]) * h(V_DIMS[d]) * rho;
            addEdge(v, u, weight);
        }
        
        
        //Add inflation term
        if(v != SOURCE)
        {
            float w_b = LAMBDA * h(0) * h(1) * h(2);
            addEdge(v, source, w_b);
        }
    }
    
    //Allocate volume
    Volume * result = new Volume(grid_dim(0), grid_dim(1), grid_dim(2));

    //Allocate predecessor list
    vector<Traits::edge_descriptor> pred(num_vertices(*g));
    
    //Do min-cuts on graph to get result
    float flow = edmunds_karp_max_flow(
        *g, 
        source, 
        sink, 
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

