/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
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


//Function that deletes duplicates in a vector
//Takes in an integer vector as parameter 
void deleteDuplicates(std::vector<int>& vect){
    for(std::vector<int>::iterator it1 = vect.begin(); it1 != vect.end(); it1++){ //loop through the vector to check for and delete duplicates
        for(std::vector<int>::iterator it2 = it1 + 1; it2 != vect.end();){
            if(*it1 == *it2) it2 = vect.erase(it2); //if the two elements are equal, delete the one pointed to by it2 
            else it2++; 
        }
    }
}

//This helper function will return the x coordinate for a given LatLon object
double convertLatitudeToX(LatLon coord, double lat_avg){
    double x = kEarthRadiusInMeters*cos(lat_avg)*coord.longitude()*kDegreeToRadian;
    return x;
}

//This helper function will return the y coordinate for a given LatLon object
double convertLongitudeToY(LatLon coord){
    return coord.latitude()*kEarthRadiusInMeters*kDegreeToRadian;
}

//Function that 'refines' a given string by removing spaces and converting all alphabetical characters to lowercase
//Takes a string as parameter
//This helper function will refine a given string by removing spaces and converting to lowercase
void streetNameRefiner(std::string & street_name){
    //Remove spaces first
    street_name.erase(std::remove(street_name.begin(), street_name.end(), ' '), street_name.end());
    //Convert to lowercase
    boost::to_lower(street_name);
}

//This helper function will compute the length of a street segment given a street segment id
double computeStreetSegmentLength(StreetSegmentIdx segment_id){
    StreetSegmentInfo street_seg_info = getStreetSegmentInfo(segment_id);
    IntersectionIdx from_intersection = street_seg_info.from;
    IntersectionIdx to_intersection = street_seg_info.to;
    LatLon first_coord = getIntersectionPosition(from_intersection); //obtain LatLon coordinates of the start point of the segment
    LatLon last_coord = getIntersectionPosition(to_intersection); //obtain LatLon coordinates of the end point of the segment
        
    //If there are no curve points, return the distance between the start point and end point
    if(street_seg_info.numCurvePoints == 0){
        std::pair<LatLon, LatLon> coords(first_coord, last_coord);
        return findDistanceBetweenTwoPoints(coords);
    }
    else{
        
        LatLon currentPoint = first_coord;
        LatLon nextPoint;
        double totalLength = 0;
        double currLength = 0;
        
        //Find the individual distances between each consecutive curve point and sum them up together
        for(int i = 0; i<street_seg_info.numCurvePoints; i++){
            nextPoint = getStreetSegmentCurvePoint(i, segment_id); //nextPoint will take coordinates of the second curve point in the pair
            std::pair<LatLon, LatLon> coordinates(currentPoint, nextPoint);
            currLength = findDistanceBetweenTwoPoints(coordinates);
            totalLength += currLength;
            currentPoint = nextPoint; //currentPoint is updated and takes the value of next curvePoint, so the nextPoint can be updated in next iteration
        }
        std::pair<LatLon, LatLon> coordinates2(currentPoint, last_coord);
        currLength = findDistanceBetweenTwoPoints(coordinates2); //find distance between last curve point and last point
        totalLength += currLength;
        return totalLength;
    }
}
