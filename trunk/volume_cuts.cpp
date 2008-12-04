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
#define WINDOW_SIZE        0.25f
#define NCC_SIZE           5

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
    //cout << "Calculating ncc @ " << x << endl;
    
    //Compute image positions
    vec3 fp = hgmult(v0->cam, x),
         tp = hgmult(v1->cam, x);
    
    int fx = fp(0), fy = fp(1),
        tx = tp(0), ty = tp(1);
    
    /*
    cout << "fx,fy = " << fx << "," << fy << endl;
    cout << "tx,ty = " << tx << "," << ty << endl;
    */
    
    //Find averages
    vec3 f_avg = 0.0f, t_avg = 0.0f;
    
    for(int dx=-NCC_SIZE; dx<=NCC_SIZE; dx++)
    for(int dy=-NCC_SIZE; dy<=NCC_SIZE; dy++)
    {
        f_avg += v0->readPixel(fx + dx, fy + dy);
        t_avg += v1->readPixel(tx + dx, ty + dy);
    }
    
    float s = 2 * NCC_SIZE + 1;
    s *= s * 255.0f;
    f_avg /= s;
    t_avg /= s;
    
    /*
    cout << "favg = " << f_avg << endl;
    cout << "tavg = " << t_avg << endl;
    */
    
    //Find numerator/denomenators
    float num = 0.0f, denom_f = 0.0f, denom_t = 0.0f;
    
    for(int dx=-NCC_SIZE; dx<=NCC_SIZE; dx++)
    for(int dy=-NCC_SIZE; dy<=NCC_SIZE; dy++)
    {
        vec3 f = v0->readPixel(fx+dx, fy+dy);
        f /= 255.0f;
        f -= f_avg;
        vec3 t = v1->readPixel(tx+dx, ty+dy);
        t /= 255.0f;
        t -= t_avg;
        
        //Accumulate variables
        num     += f(0) * t(0) + f(1) * t(1) + f(2) * t(2);
        denom_f += f(0) * f(0) + f(1) * f(1) + f(2) * f(2);
        denom_t += t(0) * t(0) + t(1) * t(1) + t(2) * t(2);
    }
    
    /*
    cout << "n  = " << num << endl
         << "df = " << denom_f << endl
         << "dt = " << denom_t << endl;
    */
    
    float denom = denom_f * denom_t;
    if(denom <= 1e-10)
        return 0.0f;
    
    //Compute total correlation
    float cor = num / sqrtf(denom_f * denom_t);
    
    //cout << "cor = " << cor << endl;
    
    return cor;
}

//Finds the maxima in a list of samples
void findMaxima(int * maxima, float * samples, int n_samples)
{
    int last_max = (samples[0] > samples[1] ? 0 : -n_samples);
    
    //Do forward pass
    maxima[0] = last_max;
    for(int i=1; i<n_samples-1; i++)
    {
        if(samples[i] > samples[i-1] && samples[i] > samples[i+1])
            last_max = i;
        
        maxima[i] = last_max;
    }
    
    //Do backward pass
    last_max = (samples[n_samples-1] > samples[n_samples - 2] ? n_samples - 1 : n_samples + 10000);
    
    for(int i=n_samples-1; i>=0; i--)
    {
        if(i == maxima[i])
            last_max = i;
        
        if(last_max - i < i - maxima[i])
            maxima[i] = last_max;
    }
}

//Gaussian kernel
float kernel(int d)
{
    return exp(-WINDOW_SIZE * (float)(d * d)) / sqrtf(2.0 * M_PI * WINDOW_SIZE);
}

//Parzen filters a signal & sums into result
void parzenFilter(float * result, float * samples, int n_samples)
{
    static int maxima[RAY_SAMPLES_MAX];
    
    findMaxima(maxima, samples, n_samples);
    
    //cout << "filtering..." << endl;
    for(int i=0; i<n_samples; i++)
    {
        float v = kernel(i - maxima[i]) * samples[maxima[i]];
        result[i] += v;
          
        //cout << "f(" << i << ")=" << samples[i] << ", maxima = " << maxima[i] << ", parz = " << v << endl;
      
    }
}

//Filters the samples using a Parzen window (not implemented)
void filterSamples(
    float result[RAY_SAMPLES_MAX],
    float samples[VIEW_COUNT][RAY_SAMPLES_MAX],
    int n_samples)
{
    for(int i=0; i<n_samples; i++)
        result[i] = 0.0f;
    for(int i=0; i<VIEW_COUNT; i++)
        parzenFilter(result, samples[i], n_samples);
}

//Compute vote from camera i
float vote(size_t n, vector<View*> views, vec3 x, Volume * hull)
{
    //cout << "Checking vote: " << n << ", " << x << endl;
    
    //Must have at least N+1 views total
    assert(VIEW_COUNT + 1 <= views.size());
    
    //Sort views by distance to view i
    vec3 c = views[n]->center;
    
    //Find the M closest views (not very efficient)
    View  *closest_views[VIEW_COUNT];
    float dist[VIEW_COUNT];
    
    for(int i=0; i<VIEW_COUNT; i++)
        dist[i] = 1e60;
    
    for(size_t i=0; i<views.size(); i++)
    {
        //Skip camera image
        if(i == n) continue;
        
        //Find distance to base image
        vec3 delta = views[i]->center;
        delta -= c;
        float d = delta(0) * delta(0) + delta(1) * delta(1) + delta(2) * delta(2);
        
        //Search through existing images and sort
        for(size_t j=0; j<VIEW_COUNT; j++)
        {
            if(d < dist[j])
            {
                for(size_t k=VIEW_COUNT-1; k>j; k--)
                {
                    closest_views[k] = closest_views[k-1];
                    dist[k] = dist[k-1];
                }
                
                closest_views[j] = views[i];
                dist[j] = d;
                break;
            }
        }
    }
    
    /*
    cout << "Center view = " << views[n]->center << endl;
    cout << "Closest views = " << endl;
    for(int i=0; i<VIEW_COUNT; i++)
    {
        cout << "d = " << dist[i] << ", c = " << closest_views[i]->center << endl;
    }
    */
    
    
    //Find optical ray
    Ray r = Ray(x, c);
    r.d -= x;
    float l = sqrt(r.d(0) * r.d(0) + r.d(1) * r.d(1) + r.d(2) * r.d(2));
    
    //cout << "Optical ray = " << r.o << ", " << r.d << endl;
    
    //Trace samples of C(d)
    static float samples[VIEW_COUNT][RAY_SAMPLES_MAX], filter_samples[RAY_SAMPLES_MAX];
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
        {
            samples[i][n_samples] = ncc(views[n], closest_views[i], p);
            assert(!isnan(samples[i][n_samples]));
        }
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
    {
        float v = vote(i, views, p, hull);
        //cout << "vote(" << i << ") = " << v << endl;
        sum += v;
    }
    return exp(-sum);
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
            
            cout << "x = " << pt << endl;
            cout << "rho = " << rho << endl;
            
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

