/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/main.cc to edit this template
 */

/* 
 * File:   m3.cpp
 * Author: shahjay8
 *
 * Created on March 26, 2022, 10:28 p.m.
 */

#pragma once
#include "StreetsDatabaseAPI.h"
#include <vector>
#include <string>
#include <cstdlib>
#include "m1.h"
#include "m2.h"
#include <list>
#include <queue>

#define NO_EDGE -1
#define INF 2147483647

using namespace std;

typedef int Edge;

struct WaveElem {
   IntersectionIdx nodeID;
   StreetSegmentIdx edgeID;  // edge used to reach this node
   double travelTime; // Total travel time to reach node
   WaveElem (int n, int e, float time) {
         nodeID = n; 
         edgeID = e; 
         travelTime = time; 
   }
   WaveElem (int n, int e) {
         nodeID = n; 
         edgeID = e;
   }
};

//Declaring comparator struct
struct CompareTravelTime {
    bool operator ()(WaveElem const& w1, WaveElem const& w2) {
        return w1.travelTime > w2.travelTime;
    }
};

//Declaring node class
class Node {
   //Add more - Outgoing edges etc.
public: //CHANGE THIS
   StreetSegmentIdx reachingEdge; // edge used to reach node
   double bestTime;  // Shortest time found to this node so far
};

//Declaring functions
double computePathTravelTime(const double turn_penalty, const std::vector<StreetSegmentIdx>& path);
std::vector<StreetSegmentIdx> findPathBetweenIntersections(const double turn_penalty, const std::pair<IntersectionIdx, IntersectionIdx> intersect_ids);
bool bfsPath (int srcID, int destID, vector<Node>& nodes, double turn_penalty);
vector<Edge> bfsTraceBack (int destID, vector<Node> nodes);
vector<Edge> bfsTraceBack2(int destID, vector<Node> nodes);
bool bfsPath2 (int srcID, int destID, vector<Node>& nodes, const double turn_penalty);

/*
//This function returns true when a path is found and false otherwise
bool bfsPath (int srcID, int destID, vector<Node>& nodes, const double turn_penalty){
    
    //Create a priority queue which stores the information of the nodes
    priority_queue<WaveElem, vector<WaveElem>, CompareTravelTime> wavefront; 
    wavefront.push(WaveElem (srcID, NO_EDGE, 0));
    
    //Keep iterating until wavefront is not empty
    while(!wavefront.empty()){
      WaveElem wave = wavefront.top(); // Get next node
      wavefront.pop();  // Remove from wavefront
      
      int currNodeId = wave.nodeID;
      if(wave.travelTime<nodes[currNodeId].bestTime){
         // Was this a better path to this node? Update if so.
         nodes[currNodeId].reachingEdge = wave.edgeID;
         nodes[currNodeId].bestTime = wave.travelTime;
         
         //when destination intersection is found, return true
         if (currNodeId == destID){ 
             return true;
         }

        StreetSegmentInfo reachingEdgeInfo;
        if(currNodeId != srcID)reachingEdgeInfo = getStreetSegmentInfo(wave.edgeID);
        vector<StreetSegmentIdx> streetSegments = findStreetSegmentsOfIntersection(currNodeId);
        
        //Iterate through all street segments connected to the intersection
        for(int seg = 0; seg<streetSegments.size(); seg++){
            int outEdgeId = streetSegments[seg];
            StreetSegmentInfo streetSegInfo = getStreetSegmentInfo(outEdgeId);
            int toNodeId = (streetSegInfo.from == currNodeId) ? streetSegInfo.to : streetSegInfo.from;
            
            //When the current node is same as the to point of the segment
            if(streetSegInfo.to == currNodeId){
                //Make sure it is not one way
                if(!streetSegInfo.oneWay){
                    if(currNodeId == srcID) wavefront.push(WaveElem(toNodeId, outEdgeId, nodes[currNodeId].bestTime + findStreetSegmentTravelTime(outEdgeId)));
                    else if(reachingEdgeInfo.streetID == streetSegInfo.streetID)
                        wavefront.push(WaveElem(toNodeId, outEdgeId, nodes[currNodeId].bestTime + findStreetSegmentTravelTime(outEdgeId)));
                    else
                        wavefront.push(WaveElem(toNodeId, outEdgeId, turn_penalty + nodes[currNodeId].bestTime + findStreetSegmentTravelTime(outEdgeId)));
                }  
            }
            
            //When the current node is same as the from point of the segmnet
            else if(streetSegInfo.from == currNodeId){
                  if(currNodeId == srcID) wavefront.push(WaveElem(toNodeId, outEdgeId, nodes[currNodeId].bestTime + findStreetSegmentTravelTime(outEdgeId)));
                  else if(reachingEdgeInfo.streetID == streetSegInfo.streetID)
                      wavefront.push(WaveElem(toNodeId, outEdgeId, nodes[currNodeId].bestTime + findStreetSegmentTravelTime(outEdgeId)));
                  else
                      wavefront.push(WaveElem(toNodeId, outEdgeId, turn_penalty + nodes[currNodeId].bestTime + findStreetSegmentTravelTime(outEdgeId)));
            }
        }
      }
    }
    
    //return false if path is not found
    return false;
}

//This function backtracks the path found in the bfsPath() function
vector<Edge> bfsTraceBack (int destID, vector<Node> nodes){
    list<Edge> path;  // Want .push_front()
    int currNodeId = destID;
    Edge prevEdge = nodes[currNodeId].reachingEdge;
    
    //Keep iterating until there is no edge
    while (prevEdge != NO_EDGE) {
        //update current node based on to and from
        if(currNodeId == getStreetSegmentInfo(prevEdge).to){
            currNodeId = getStreetSegmentInfo(prevEdge).from;
        }
        else{
            currNodeId = getStreetSegmentInfo(prevEdge).to;
        }
        path.push_front (prevEdge);
        prevEdge = nodes[currNodeId].reachingEdge;
    }
    
    //convert list to vector
    vector<int> vec_path(path.size());
    copy(path.begin(), path.end(), vec_path.begin());
    return vec_path;
}*/



// Returns the time required to travel along the path specified, in seconds.
// The path is given as a vector of street segment ids, and this function can
// assume the vector either forms a legal path or has size == 0. The travel
// time is the sum of the length/speed-limit of each street segment, plus the
// given turn_penalty (in seconds) per turn implied by the path. If there is
// no turn, then there is no penalty. Note that whenever the street id changes
// (e.g. going from Bloor Street West to Bloor Street East) we have a turn.
double computePathTravelTime(const double turn_penalty, const std::vector<StreetSegmentIdx>& path){
    if(path.size() == 0) return 0; //if path is empty, return 0
    double total_time = 0; //return variable
    int turn_count = 0; 
    //calculate the time for the first segment before executing the loop
    StreetSegmentIdx past_id = path[0];
    StreetSegmentInfo past_info = getStreetSegmentInfo(past_id);
    double distance = findStreetSegmentLength(past_id);
    double time = distance/past_info.speedLimit; //time = distance/speedLimit
    total_time += time;//update the total time
    for(int i = 1; i < path.size(); i++){
        //get the next street segment
        StreetSegmentIdx curr_id = path[i];
        StreetSegmentInfo curr_info = getStreetSegmentInfo(curr_id);
        //compare it to the previous segment to check if there is a turn 
        if(curr_info.streetID != past_info.streetID) turn_count++;
        //compute the time for current street segment
        //distance = findStreetSegmentLength(curr_id);
        time = findStreetSegmentTravelTime(path[i]);
        total_time += time;
        //save the current street segment to compare with next segment
        past_id = path[i];
        past_info = getStreetSegmentInfo(past_id);
    }
    double penalty = turn_count * turn_penalty;//add the time penalties
    total_time += penalty;
    return total_time;
}

//This function returns true when a path is found and false otherwise
bool bfsPath2 (int srcID, int destID, vector<Node>& nodes, const double turn_penalty){
    
    //Create a priority queue which stores the information of the nodes
    priority_queue<WaveElem, vector<WaveElem>, CompareTravelTime> wavefront; 
    wavefront.push(WaveElem (srcID, NO_EDGE, 0));
    
    //Keep iterating until wavefront is not empty
    while(!wavefront.empty()){
      WaveElem wave = wavefront.top(); // Get next node
      wavefront.pop();  // Remove from wavefront
      
      int currNodeId = wave.nodeID;
      if(wave.travelTime<nodes[currNodeId].bestTime){
         // Was this a better path to this node? Update if so.
         nodes[currNodeId].reachingEdge = wave.edgeID;
         nodes[currNodeId].bestTime = wave.travelTime;

        StreetSegmentInfo reachingEdgeInfo;
        if(currNodeId != srcID)reachingEdgeInfo = getStreetSegmentInfo(wave.edgeID);
        vector<StreetSegmentIdx> streetSegments = findStreetSegmentsOfIntersection(currNodeId);
        
        //Iterate through all street segments connected to the intersection
        for(int seg = 0; seg<streetSegments.size(); seg++){
            int outEdgeId = streetSegments[seg];
            StreetSegmentInfo streetSegInfo = getStreetSegmentInfo(outEdgeId);
            int toNodeId = (streetSegInfo.from == currNodeId) ? streetSegInfo.to : streetSegInfo.from;
            double travelTime = findStreetSegmentTravelTime(outEdgeId);
            //When the current node is same as the from point of the segmnet
            if(streetSegInfo.from == currNodeId){
                wavefront.push(WaveElem(toNodeId, outEdgeId, turn_penalty + nodes[currNodeId].bestTime + travelTime));
            }
            
            //When the current node is same as the to point of the segment
            else if(streetSegInfo.to == currNodeId){
                //Make sure it is not one way
                if(!streetSegInfo.oneWay){
                    wavefront.push(WaveElem(toNodeId, outEdgeId, turn_penalty + nodes[currNodeId].bestTime + travelTime));
                }  
            }    
        }
     }
    }
    
    return true;
}
    
//This function backtracks the path found in the bfsPath() function
vector<Edge> bfsTraceBack2(int destID, vector<Node> nodes){
    list<Edge> path;  // Want .push_front()
    int flag = 0;
    int currNodeId = destID;
    Edge prevEdge = nodes[currNodeId].reachingEdge;
    
    //Keep iterating until there is no edge
    while (prevEdge != NO_EDGE) {
        flag++;
        
        //update current node based on to and from
        currNodeId = (currNodeId == getStreetSegmentInfo(prevEdge).from)?getStreetSegmentInfo(prevEdge).to: getStreetSegmentInfo(prevEdge).from;
        
        path.push_front (prevEdge);
        prevEdge = nodes[currNodeId].reachingEdge;
        if(65000<flag){
            vector<Edge> nothing;
            return nothing;
        }
    }
    
    //convert list to vector
    vector<int> vec_path(path.size());
    copy(path.begin(), path.end(), vec_path.begin());
    return vec_path;
}

// Returns a path (route) between the start intersection (1st object in intersect_ids pair)
// and the destination intersection(2nd object in intersect_ids pair),
// if one exists. This routine should return the shortest path
// between the given intersections, where the time penalty to turn right or
// left is given by turn_penalty (in seconds). If no path exists, this routine
// returns an empty (size == 0) vector. If more than one path exists, the path
// with the shortest travel time is returned. The path is returned as a vector
// of street segment ids; traversing these street segments, in the returned
// order, would take one from the start to the destination intersection.
std::vector<StreetSegmentIdx> findPathBetweenIntersections(const double turn_penalty, const std::pair<IntersectionIdx, IntersectionIdx> intersect_ids){
    vector<Node> nodes;
    nodes.resize(getNumIntersections());
    for(int i = 0; i < nodes.size(); i++){
        nodes[i].reachingEdge = NO_EDGE;
        nodes[i].bestTime = INF;
    }
    bool found = bfsPath2(intersect_ids.first, intersect_ids.second, nodes, turn_penalty);
    vector<Edge> path;
    if(found){
        path = bfsTraceBack2(intersect_ids.second, nodes);
        return path;
    }
    return path;    
}
