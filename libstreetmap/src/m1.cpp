/* 
 * Copyright 2022 University of Toronto
 *
 * Permission is hereby granted, to use this software and associated 
 * documentation files (the "Software") in course work at the University 
 * of Toronto, or for personal use. Other uses are prohibited, in 
 * particular the distribution of the Software either publicly or to third 
 * parties.
 *
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include <iostream>
#include <cctype>
#include "m1.h"
#include "OSMDatabaseAPI.h"
#include "StreetsDatabaseAPI.h"
#include <cmath>
#include <limits>
#include <unordered_map>
#include <string>
#include <boost/algorithm/string.hpp>
#include <helperfunction.h>
#include <globals.h>
#include <chrono>


// loadMap will be called with the name of the file that stores the "layer-2"
// map data accessed through StreetsDatabaseAPI: the street and intersection 
// data that is higher-level than the raw OSM data). 
// This file name will always end in ".streets.bin" and you 
// can call loadStreetsDatabaseBIN with this filename to initialize the
// layer 2 (StreetsDatabase) API.
// If you need data from the lower level, layer 1, API that provides raw OSM
// data (nodes, ways, etc.) you will also need to initialize the layer 1 
// OSMDatabaseAPI by calling loadOSMDatabaseBIN. That function needs the 
// name of the ".osm.bin" file that matches your map -- just change 
// ".streets" to ".osm" in the map_streets_database_filename to get the proper
// name.
bool loadMap(std::string map_streets_database_filename) {
  
    bool first = loadStreetsDatabaseBIN(map_streets_database_filename);  //Indicates whether the map has loaded 
    if(!first) return false;                              
    
    int length = map_streets_database_filename.length()-11; // 11 because "streets.bin"
    //changing ending of filename from .streets.bin to .osm.bin
    map_streets_database_filename = map_streets_database_filename.substr(0, length);    
    std::string str = "osm.bin";   
    map_streets_database_filename = map_streets_database_filename.append(str);   
    //loading OSM API
    
    
    bool second = loadOSMDatabaseBIN(map_streets_database_filename); 
    auto end = std::chrono::system_clock::now();
   
   
    if(!second) return false; //if both loads are successful return true 

       //Load intersection_street_segments
    int num_intersections = getNumIntersections();
    for(int i = 0; i < num_intersections; i++){ 
        std::vector <StreetSegmentIdx> segments;
        int num_segments = getNumIntersectionStreetSegment(i);
        for(int j = 0; j < num_segments; j++){
            segments.push_back(getIntersectionStreetSegment(j,i));
        }
        intersection_street_segments.push_back(segments);
    }
    
    // Load street_intersections
    for(int i = 0; i < getNumStreets(); i++){
        std::vector <IntersectionIdx> intersections;
        street_intersections.push_back(intersections);
    }
    for(int i = 0; i < num_intersections; i++){
        for(int j = 0; j < intersection_street_segments[i].size(); j++){
            StreetIdx street_id = getStreetSegmentInfo(intersection_street_segments[i][j]).streetID;
            street_intersections[street_id].push_back(i);
        }
    }
   
    // load unordered map with OSMid and LatLon class
    int num_osm = getNumberOfNodes(); 
    for (int i = 0; i < num_osm; i++){
        osm_nodes[getNodeByIndex(i)->id()]= getNodeCoords(getNodeByIndex(i));
    }
    
    // load refined_street_names
    std::string current_street_name;
    for(int i = 0; i < getNumStreets(); i++){
        current_street_name = getStreetName(i);
        streetNameRefiner(current_street_name);//refine the street name by removing spaces and converting to lowercase
        street_name_database.insert(std::pair<std::string, StreetIdx>(current_street_name,i)); //add the refined name and street id to the street_name_database
    }
    
    street_length_database.resize(getNumStreets(),0);
    //Load speed_limit_database, street_id_database, street_segment_length database, street_length_database
    for(int i = 0; i<getNumStreetSegments(); i++){
        StreetSegmentInfo street_seg_info = getStreetSegmentInfo(i);
        speed_limit_database.push_back(street_seg_info.speedLimit); //push to speed_limit_database
        street_id_database.push_back(street_seg_info.streetID); //push to street_id_database
        double dist = computeStreetSegmentLength(i); //calculate the lengths of each street segment and store in the database
        street_segment_length.push_back(dist); //push the distance to the street_segment_length database
        street_length_database[street_seg_info.streetID] += dist; //increment the length of the street to which the street segment belongs to
    }

    return true;
}

void closeMap() {
   
    street_length_database.clear(); //Clear the street_length_database to avoid duplicates
    street_name_database.clear(); //clear the street_name_database to avoid duplicates
    closeOSMDatabase();//closing OSM Database
    closeStreetDatabase();//closing Streets Database 

}


// Returns the distance between two (lattitude,longitude) coordinates in meters
// Speed Requirement --> moderate
double findDistanceBetweenTwoPoints(std::pair<LatLon, LatLon> points){
    double lat_average = (points.first.latitude() + points.second.latitude())*kDegreeToRadian/2; //calculate average latitude of the points
    
    //Call helper functions to convert the LatLon coordinates of the two points to xy coordinates
    double x1 = convertLatitudeToX(points.first, lat_average);
    double x2 = convertLatitudeToX(points.second, lat_average);
    double y1 = convertLongitudeToY(points.first);
    double y2 = convertLongitudeToY(points.second);
    
    //Calculate distance between two xy coordinates using the distance formula and return it
    double x_diff = (x1-x2)*(x1-x2);
    double y_diff = (y1-y2)*(y1-y2);
    return sqrt(x_diff+y_diff);
}

// Returns the length of the given street segment in meters
// Speed Requirement --> moderate
double findStreetSegmentLength(StreetSegmentIdx street_segment_id){
    //directly access the street_segment_length database created in load_map
    return street_segment_length[street_segment_id];
}

// Returns the travel time to drive from one end of a street segment
// to the other, in seconds, when driving at the speed limit
// Note: (time = distance/speed_limit)
// Speed Requirement --> high
double findStreetSegmentTravelTime(StreetSegmentIdx street_segment_id){
    
    //directly access the street_segment_length database created in load_map
    //you divide this length by the speed_limit which is found by directly accessing the speed_limit_database created in load map
    return street_segment_length[street_segment_id]/speed_limit_database[street_segment_id];
}

// Returns the geographically nearest intersection (i.e. as the crow flies) to the given position
// Speed Requirement --> none
IntersectionIdx findClosestIntersection(LatLon my_position){
    LatLon currentPoint = getIntersectionPosition(0);
    std::pair<LatLon, LatLon> points(my_position, currentPoint);
    double currentDistance = findDistanceBetweenTwoPoints(points); //calculate distance between my_position and 0th intersection
    double smallestDistance = currentDistance; //current smallestDistance will be distance previously calculated
    LatLon closestPoint;
    IntersectionIdx requiredId = 0;
    
    //Iterate through every intersection point
    for(int i = 1; i<getNumIntersections();i++){
        currentPoint = getIntersectionPosition(i);
        std::pair<LatLon, LatLon> points2(my_position, currentPoint);
        
        //find distance between the current intersection point and my_position
        currentDistance = findDistanceBetweenTwoPoints(points2);
        
        //update smallest distance if currentDistance is smaller than the smallestDistance
        if(currentDistance<=smallestDistance){
            smallestDistance = currentDistance;
            closestPoint = currentPoint;
            requiredId = i; //update the requiredId
        }
    }
    return requiredId;
}

// Returns the street segments that connect to the given intersection 
// Speed Requirement --> high
std::vector<StreetSegmentIdx> findStreetSegmentsOfIntersection(IntersectionIdx intersection_id){
    return intersection_street_segments[intersection_id]; //Returns the corresponding vector from 2d vector intersection_street_segments
}

// Returns true if the two intersections are directly connected, meaning you can legally 
// drive from the first intersection to the second using only one streetSegment.
// Speed Requirement --> moderate 
//Takes in a pair of intersection ID's as the parameter and returns a boolean value
bool intersectionsAreDirectlyConnected(std::pair<IntersectionIdx, IntersectionIdx> intersection_ids){
    std::vector<StreetSegmentIdx> first_segments = findStreetSegmentsOfIntersection(intersection_ids.first); //Vector that holds the street segment ID's of first intersection
    std::vector<StreetSegmentIdx> second_segments = findStreetSegmentsOfIntersection(intersection_ids.second);//Vector that holds the street segment ID's of first intersection
    for(int i = 0; i < first_segments.size(); i++){//Loop through both vectors and compare them to see if they have any matching elements
        for(int j = 0; j < second_segments.size(); j++){
            if(first_segments[i] == second_segments[j]){ //Vectors have a matching element, check for legality
                StreetSegmentIdx segment_id = first_segments[i];
                StreetSegmentInfo segment_info = getStreetSegmentInfo(segment_id);
                if(segment_info.from == intersection_ids.first || !segment_info.oneWay) return true;    
            } 
                
        }
    }
    return false; //if the loop is exited without returning, vectors have no matching elements
}

// Returns the street names at the given intersection (includes duplicate 
// street names in the returned vector)
// Speed Requirement --> high 
// Takes in an intersection ID as parameter and returns a vector of strings
std::vector<std::string> findStreetNamesOfIntersection(IntersectionIdx intersection_id){
    std::vector<std::string> street_names; //vector that holds street names
    std::vector<StreetSegmentIdx> segments = findStreetSegmentsOfIntersection(intersection_id); //vector that holds street segment ID's of the given intersection
    for(int i = 0; i < segments.size(); i++){//loop through the segments vector to add each of the corresponding street name to street_names
        street_names.push_back(getStreetName(getStreetSegmentInfo(segments[i]).streetID)); 
    }
    return street_names;
}

// Returns all intersections along the a given street.
// There should be no duplicate intersections in the returned vector.
// Speed Requirement --> high
// Takes in a street ID as parameter, returns a vector of intersection ID's
std::vector<IntersectionIdx> findIntersectionsOfStreet(StreetIdx street_id){
    std::vector<IntersectionIdx> intersections = street_intersections[street_id]; //use pre-loaded data structure street_intersections
    deleteDuplicates(intersections); //delete the duplicates
    return intersections;
}

// Returns all street ids corresponding to street names that start with the
// given prefix
// The function should be case-insensitive to the street prefix.
// The function should ignore spaces.
//  For example, both "bloor " and "BloOrst" are prefixes to
// "Bloor Street East".
// If no street names match the given prefix, this routine returns an empty
// (length 0) vector.
// You can choose what to return if the street prefix passed in is an empty
// (length 0) string, but your program must not crash if street_prefix is a
// length 0 string.
// Speed Requirement --> high
std::vector<StreetIdx> findStreetIdsFromPartialStreetName(std::string street_prefix){
    std::vector<StreetIdx> matchingStreets;
    if(street_prefix.empty()){
        return matchingStreets;
    }
    streetNameRefiner(street_prefix); //refine the street prefix by removing spaces and converting to lower case
    std::multimap<std::string, StreetIdx>::iterator it, it_low, it_high;
    it_low = street_name_database.lower_bound(street_prefix);
    
    std::string upperBound = street_prefix;
    upperBound.back() += 1; //Add one to the last character of the upper bound
    it_high = street_name_database.upper_bound(upperBound);
    
    //Iterate from the lower bound to the upper bound and every id that is contained in this interval
    for(it = it_low; it!=it_high; ++it){
        matchingStreets.push_back((*it).second); //push the street id to matchingStreets
    }
    return matchingStreets;
}

// Returns the length of a given street in meters
// Speed Requirement --> high
double findStreetLength(StreetIdx street_id){
    
    //directly access the street_length database created in load_maps
    double length = street_length_database[street_id];
    return length;
}

// Return all intersection ids at which the two given streets intersect
// This function will typically return one intersection id for streets
// that intersect and a length 0 vector for streets that do not. For unusual 
// curved streets it is possible to have more than one intersection at which 
// two streets cross.
// There should be no duplicate intersections in the returned vector.
// Speed Requirement --> high
// Takes a pair of street ID's as a parameter and returns a vector of intersection ID's
std::vector<IntersectionIdx> findIntersectionsOfTwoStreets(std::pair<StreetIdx, StreetIdx> street_ids){
    std::vector<IntersectionIdx> first_intersections = street_intersections[street_ids.first];  //vector that holds the intersections of the first street of the pair
    std::vector<IntersectionIdx> second_intersections = street_intersections[street_ids.second]; //vector that holds the intersections of the second street of the pair
    
    std::vector<IntersectionIdx> intersections; //vector that will hold the intersection ID's of common intersections
    for(int i = 0; i < first_intersections.size(); i++){ //loop through both vectors to find common elements
        for(int j = 0; j < second_intersections.size(); j++){
            if(first_intersections[i] == second_intersections[j]) 
                intersections.push_back(first_intersections[i]); //insert the common element into 'intersections'
        }
    }
    deleteDuplicates(intersections);  //remove duplicate elements
    return intersections;
}

//finds the closest point of Interest of a given type. 
POIIdx findClosestPOI(LatLon my_position, std::string POItype){
    int numpoi = getNumPointsOfInterest(); //Number of points of interest
    int id = 0; 
    double shortest = std::numeric_limits<double>::max(); //initializing shortest to the maximum possible value 
    double t;  
    
    //looping through all the points of interest, finding POIs that match the given and returning the POIID of the closest point. 
    for (int i = 0; i < numpoi; i++){
        if (getPOIType(i)==POItype){
            std::pair<LatLon, LatLon>coords2(my_position, getPOIPosition(i)); 
            t = findDistanceBetweenTwoPoints(coords2);
            if (t<shortest){
                shortest = t; 
                id = i; 
            }
        }
    }
    
    return id;  
}

//finds the feature area of a given feature type 
double findFeatureArea(FeatureIdx feature_id){
    int num = getNumFeaturePoints(feature_id);
    double area = 0.0;
    double x1, y1, x0 , y0; 
    bool closed_polygon; 
    
    if (getFeaturePoint(0,feature_id)==getFeaturePoint(num-1,feature_id)){closed_polygon = true;}
    else{closed_polygon = false;}
    
    double latavg = 0;
    
    //checking to see if feature is a closed polygon 
    if (closed_polygon){
        for (int i = 0; i < num; i++){
            latavg += getFeaturePoint(i,feature_id).latitude()*kDegreeToRadian;
        }}
        latavg = latavg/num; 
    
    //converting from lat lon to x and y 
    for (int i = 0; i< num -1; i++){
       
        x0 = convertLatitudeToX(getFeaturePoint(i,feature_id), latavg);
        x1 = convertLatitudeToX(getFeaturePoint(i+1,feature_id), latavg);
        y0 = convertLongitudeToY(getFeaturePoint(i,feature_id));
        y1 = convertLongitudeToY(getFeaturePoint(i+1,feature_id));
        
        area += (y1-y0)*((x0+x1)/2.0);}
 
    if (!closed_polygon){area = 0.0;} //area is 0 if its not a closed polygon
    if (area <0){ area = area * -1;} //ensuring area returned is the absolute value 
 return area; 
}

// Return the LatLon of an OSM node; will only be called with OSM nodes (not ways or relations)
//returns the latlon class of the given OSMid
LatLon findLatLonOfOSMNode (OSMID OSMid){
    auto got = osm_nodes.find(OSMid); //taking advantage of unordered map's random access
    return (got->second);
}   
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    