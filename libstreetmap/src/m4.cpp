/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/main.cc to edit this template
 */

/* 
 * File:   m4.cpp
 * Author: shahjay8
 *
 * Created on April 11, 2022, 12:25 a.m.
 */

#include <cstdlib>
#include "StreetsDatabaseAPI.h"
#include <vector>
#include <string>
#include <cstdlib>
#include <limits>
#include "m1.h"
#include "m2.h"
#include "m3.h"
#include "m4.h"

using namespace std;

//declarations

struct timeRemaining{
    IntersectionIdx id;
    double time;
};

struct trackPackageDelivery{
    IntersectionIdx pickUp;
    IntersectionIdx dropOff;
    int isPicked = 0;
    int isDropped = 0;
    int curr_index;
};

struct intersectionInf {
    IntersectionIdx intersectionID;
    double time;
    std::vector<StreetSegmentIdx> subpath;
    intersectionInf(int n){intersectionID = n;}
};

vector<trackPackageDelivery> deliveries_new;
vector<vector<intersectionInf>> matrix;

double findRemainingTime(int toNodeId, int destId);
int deliveriesLeft(vector<trackPackageDelivery> deliveries_new);
vector<vector<intersectionInf>> preComputeMatrix(const std::vector<DeliveryInf> & deliveries, const std::vector<IntersectionIdx> & depots);

vector<CourierSubPath> travelingCourier(const float turn_penalty, const std::vector<DeliveryInf>& deliveries, const std::vector<IntersectionIdx>& depots){
    
    vector<CourierSubPath> finalPath;
    CourierSubPath currSubPath;
    vector<trackPackageDelivery> deliveries_new;
    deliveries_new.resize(deliveries.size());
    int first_depot, count;
    double min_distance = numeric_limits<double>::max();
    for(int del = 0; del<deliveries.size(); del++){
        deliveries_new[del].pickUp = deliveries[del].pickUp;
        deliveries_new[del].dropOff = deliveries[del].dropOff;
        for(int depot = 0; depot<depots.size(); depot++){
            double curr_dist = findDistanceBetweenTwoPoints(make_pair(getIntersectionPosition(deliveries[del].pickUp), getIntersectionPosition(depots[depot])));
            if(min_distance>curr_dist){
                min_distance = curr_dist;
                first_depot = depots[depot];
                count = del;
            }
        }
    }
    deliveries_new[count].isPicked = 1;
    deliveries_new[count].curr_index = deliveries[count].pickUp;
    currSubPath.start_intersection = first_depot;
    currSubPath.end_intersection = deliveries_new[count].pickUp;
    currSubPath.subpath = findPathBetweenIntersections(turn_penalty, make_pair(currSubPath.start_intersection, currSubPath.end_intersection));
    finalPath.push_back(currSubPath);   
    int new_inter = deliveries_new[count].pickUp;
    
    while(!deliveriesLeft(deliveries_new)){
        currSubPath.start_intersection = new_inter;
        
        int count2, next_delivery_loc;
        double min_dist2 = numeric_limits<double>::max();
        for(int del = 0; del<deliveries_new.size(); del++){
            if(!deliveries_new[del].isDropped){
                if(!deliveries_new[del].isPicked){
                    double curr_dist = findDistanceBetweenTwoPoints(make_pair(getIntersectionPosition(deliveries[del].pickUp), getIntersectionPosition(new_inter)));
                    if(min_dist2>curr_dist){
                        min_dist2 = curr_dist;
                        count2 = del;
                        next_delivery_loc = deliveries_new[del].pickUp;
                    }
                }
                else{
                    double curr_dist = findDistanceBetweenTwoPoints(make_pair(getIntersectionPosition(deliveries[del].dropOff), getIntersectionPosition(new_inter)));
                    if(min_dist2>curr_dist){
                        min_dist2 = curr_dist;
                        count2 = del;
                        next_delivery_loc = deliveries_new[del].dropOff;
                    }
                }
            }
        }
        
        if(deliveries_new[count2].pickUp != next_delivery_loc){
            deliveries_new[count2].isDropped = 1;
        }
        else{
            deliveries_new[count2].isPicked = 1;
        }
        new_inter = next_delivery_loc;
        currSubPath.end_intersection = next_delivery_loc;
        currSubPath.subpath = findPathBetweenIntersections(turn_penalty, make_pair(currSubPath.start_intersection, currSubPath.end_intersection));
        finalPath.push_back(currSubPath); 
    }
    currSubPath.start_intersection = new_inter;
    double min_dist3 = numeric_limits<double>::max();
    for(int depot = 0; depot<depots.size(); depot++){
        double curr_dist = findDistanceBetweenTwoPoints(make_pair(getIntersectionPosition(new_inter), getIntersectionPosition(depots[depot])));
        if(min_dist3>curr_dist){
            min_dist3 = curr_dist;
            currSubPath.end_intersection = depots[depot];
        }
    }
    currSubPath.subpath = findPathBetweenIntersections(turn_penalty, make_pair(currSubPath.start_intersection, currSubPath.end_intersection));
    finalPath.push_back(currSubPath); 
    return finalPath;
}

int deliveriesLeft(vector<trackPackageDelivery> deliveries_new){
    for(int del = 0; del<deliveries_new.size(); del++){
        if(!deliveries_new[del].isDropped || !deliveries_new[del].isPicked){
            return 0;
        }
    }
    return 1;
}

double findRemainingTime(int toNodeId, int destId){
    double rem_distance = findDistanceBetweenTwoPoints(make_pair(getIntersectionPosition(toNodeId), getIntersectionPosition(destId)));
    return ((rem_distance)/25);
}

vector<vector<intersectionInf>> preComputeMatrix(const std::vector<DeliveryInf> & deliveries, const std::vector<IntersectionIdx> & depots){
    
    matrix.resize(deliveries.size()*2 + depots.size());
    
    for(int i = 0; i < matrix.size(); i++){
        
        for(int j = 0; j < deliveries.size(); j++){
            intersectionInf temp(deliveries[j].pickUp);
            matrix[i].push_back(temp);

            intersectionInf temp2(deliveries[j].dropOff);

            matrix[i].push_back(temp2);
        }
        
        for(int j = 0; j < depots.size(); j++){
            intersectionInf temp(depots[j]);
            matrix[i].push_back(temp);
        }       
    }   
    return matrix;  
}

