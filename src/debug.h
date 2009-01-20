//Debugging methods go here 
//(eg PLY saving, loading from intermediate formats etc.)
#ifndef DEBUG_H
#define DEBUG_H

#include <vector>
#include <string>

#include <Eigen/Core>

#include "view.h"
#include "system.h"

//Saves a pile of views to an interchange format
void saveTempViews(
    const std::string& directory, 
    const std::string& filename, 
    const std::vector<View>& views);
    
//Restores some temporary views
std::vector<View> loadTempViews(const std::string& filename);

//Saves a PLY file
void savePLY(
    const std::string& filename, 
    const std::vector<Eigen::Vector3d>& points, 
    const std::vector<Eigen::Vector3i>& color);


//Save cameras to PLY
void saveCameraPLY(
    const std::string& filename, 
    const std::vector<View>& views);


#endif
