/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   globals.h
 * Author: vakhari9
 *
 * Created on February 11, 2022, 10:03 PM
 */

#ifndef GLOBALS_H
#define GLOBALS_H

//ALL MILESTONE 1 GLOBAL DATA
std::vector<std::vector<StreetSegmentIdx>> intersection_street_segments; //vector that holds the vectors containing street segments for each intersection
std::multimap<std::string, StreetIdx> street_name_database; //multimap of size getNumStreets() which contains the refined street name and street id for every street
std::vector<std::vector<StreetSegmentIdx>> street_streetSegments;      //vector that holds the vectors containing street segments for each street
std::vector<std::vector<IntersectionIdx>> street_intersections;      //2d vector that holds the intersection id's of each street
std::unordered_map< OSMID,LatLon> osm_nodes;     //unordered map that holds the OSMID as key and its corresponding LatLon class is maped to it 
std::vector<float> speed_limit_database; //vector of size getNumStreetSegments() which contains the speed_limit of all street segments, sorted by the street segment ids
std::vector<StreetIdx> street_id_database; //vector of size getNumStreetSegments() which contains the street_id for every street segment
std::vector<double> street_segment_length; //vector of size getNumStreetSegments() which contains the lengths of all street segments
std::vector<double> street_length_database; //vector of size getNumStreets() which contains the lengths of all streets

#endif /* GLOBALS_H */

