/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   helperfunction.h
 * Author: vakhari9
 *
 * Created on February 11, 2022, 9:56 PM
 */

#ifndef HELPERFUNCTION_H
#define HELPERFUNCTION_H

#ifdef __cplusplus
extern "C" {
#endif

//helper function declarations for Milestone 1 
void deleteDuplicates(std::vector<int>& vect);
double convertLatitudeToX(LatLon coord, double lat_avg);
void streetNameRefiner(std::string & street_name);
double convertLongitudeToY(LatLon coord);
double computeStreetSegmentLength(StreetSegmentIdx segment_id);

#ifdef __cplusplus
}
#endif

#endif /* HELPERFUNCTION_H */

