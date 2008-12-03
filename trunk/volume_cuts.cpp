#include <boost/config.hpp>

//STL includes
#include <vector>
#include <iostream>
#include <cassert>
#include <cmath>

//Boost stuff
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/edmunds_karp_max_flow.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/tuple/tuple.hpp>

//Project
#include "misc.h"
#include "view.h"
#include "volume.h"
#include "volume_cuts.h"
#include "photohull.h"

using namespace std;
using namespace blitz;
using namespace boost;


//Lambda coefficient
#define LAMBDA             0.1f
#define INF                (1LL<<62LL)
#define PREC               20LL
#define MU                 0.05f
#define VIEW_COUNT         6
#define RAY_SAMPLES_MAX    512
#define EPSILON            0.001f

//Aliases for boost graph data type
typedef adjacency_list_traits < vecS, vecS, directedS > Traits;
typedef adjacency_list < vecS, vecS, directedS, no_property,
    property < edge_capacity_t, long long,
    property < edge_residual_capacity_t, long long,
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


//Normalized cross correlation (not implemented)
float ncc(View * v0, View* v1, vec3 x)
{
    return 0;
}

//Filters the samples (not implemented)
void filterSamples(
    float result[RAY_SAMPLES_MAX],
    float samples[RAY_SAMPLES_MAX][VIEW_COUNT],
    int n_samples)
{
}

//Compute vote from camera i
float vote(size_t n, vector<View*> views, vec3 x, Volume * hull)
{
    //Must have at least N+1 views total
    assert(VIEW_COUNT + 1 <= views.size());
    
    //Sort views by distance to view i
    vec3 c = views[n]->center;
    
    //Find the M closest views (not very efficient)
    View  *closest_views[VIEW_COUNT+1];
    float dist[VIEW_COUNT+1];
    
    for(int i=0; i<VIEW_COUNT+1; i++)
        dist[i] = 1e60;
    
    for(size_t i=0; i<views.size(); i++)
    {
        //Find distance to base image
        vec3 delta = views[i]->center;
        delta -= c;
        float d = delta(0) * delta(0) + delta(1) * delta(1) + delta(2) * delta(2);
        
        //Search through existing images and sort
        for(size_t j=0; j<VIEW_COUNT+1; j++)
        {
            if(d < dist[j])
            {
                for(size_t k=VIEW_COUNT; k>j; k--)
                {
                    closest_views[k] = closest_views[k-1];
                    dist[k] = dist[k-1];
                }
                
                closest_views[j] = views[i];
                dist[j] = j;
                break;
            }
        }
    }
    
    //The closest view should always be the base view
    assert(closest_views[0] == views[n]);
    
    //Find optical ray
    Ray r = Ray(x, c);
    r.d -= x;
    float l = sqrt(r.d(0) * r.d(0) + r.d(1) * r.d(1) + r.d(2) * r.d(2));
    
    //Trace samples of C(d)
    static float samples[RAY_SAMPLES_MAX][VIEW_COUNT], filter_samples[RAY_SAMPLES_MAX];
    int n_samples = 0;
    for(float t=0.0f; t<=1.0f; t+=1.0f/l)
    {
        vec3 p = r(t);
        
        if( p(0) < 0 || p(0) >= grid_dim(0) ||
            p(1) < 0 || p(1) >= grid_dim(1) ||
            p(2) < 0 || p(2) >= grid_dim(2) ||
            n_samples >= RAY_SAMPLES_MAX )
            break;
        
        for(int i=0; i<VIEW_COUNT; i++)
            samples[n_samples][i] = ncc(views[n], closest_views[i], p);
        n_samples++;
    }
    
    //Filter samples
    filterSamples(filter_samples, samples, n_samples);
    
    //Locate max samples
    float max_sample = filter_samples[1];
    for(int i=2; i<n_samples; i++)
    {
        if(filter_samples[i] > max_sample)
            max_sample = filter_samples[i];
    }
    
    //If base point is not a maximum, no vote!
    return filter_samples[0] >= max_sample - EPSILON ? filter_samples[0] : 0;
}

//Evaluate photoconsistency of a given grid point
float evaluatePhotoConsistency(vector<View*> views, vec3 p, Volume * hull)
{
    //Formula 3 from Vogiatzis
    float sum = 0.0f;
    for(size_t i=0; i<views.size(); i++)
        sum += vote(i, views, p, hull);
    return exp(-MU * sum);
}

//Adds the edge to the volume graph 
//  (shouldn't this be shorter in boost?  I am probably doing this wrong...)
void addEdge(int start, int end, double w)
{
    //Locate vertices
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
    long long wl = w * (double)(1LL<<PREC);
    capacity[e1] = wl;
    capacity[e2] = wl;
    residual_capacity[e1] = wl;
    residual_capacity[e2] = wl;
    rev[e2] = e1;
    rev[e1] = e1;
}


//Perform volumetric graph cuts
Volume* volumetricGraphCuts(
    std::vector<View*> views,
    vec3 interior_grid,
    Volume * photo_hull)
{
    //Set grid dimensions
    grid_dim = ivec3(
        photo_hull->xRes, 
        photo_hull->yRes, 
        photo_hull->zRes);

    //Allocate graph
    g = new Graph(grid_dim(0) * grid_dim(1) * grid_dim(2) + 1);
    
    //Find source
    sourceNode = getNode(interior_grid(0), interior_grid(1), interior_grid(2));
    
    //Find sink
    sinkNode = -1;
    
    
    //Build graph
    cout << "Building graph..." << endl;
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
            
            double rho = evaluatePhotoConsistency(views, pt, photo_hull);
            double weight = 4.0f / 3.0f * M_PI * rho;
            int u = getNode(i+DN[d](0), j+DN[d](1), k+DN[d](2));
            addEdge(v, u, weight);
        }
        
        
        //Add inflation term
        if(v != sourceNode)
        {
            float w_b = LAMBDA;
            addEdge(v, sourceNode, w_b);
        }
    }
    
    //Allocate volume
    Volume * result = new Volume(grid_dim(0), grid_dim(1), grid_dim(2));

    //Allocate predecessor list
    vector<Traits::edge_descriptor> pred(num_vertices(*g));
    
    //write_graphviz(cout, *g);
    
    //Do min-cuts on graph to get result
    cout << "Running network flow" << endl;
    edmunds_karp_max_flow(
        *g, 
        sourceNode, 
        sinkNode, 
        capacity, 
        residual_capacity, 
        rev,
        result->data,
        &pred[0]);
    
    cout << "done." << endl;
    
    //Release graph
    delete g;
    
    //Rescale
    for(size_t i=0; i<result->xRes; i++)
    for(size_t j=0; j<result->xRes; j++)
    for(size_t k=0; k<result->xRes; k++)
    {
        if((*result)(i,j,k))
            (*result)(i,j,k) = 255;
    }
    
    //Return result
    return result;
}

