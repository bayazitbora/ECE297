/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/file.h to edit this template
 */

/* 
 * File:   globals_m2.h
 * Author: shahjay8
 *
 * Created on March 11, 2022, 8:14 PM
 */

#ifndef GLOBALS_M2_H
#define GLOBALS_M2_H

float cos_latitude_average;
double area_zoom_factor;

bool enable_food; 
bool enable_healthcare; 
bool enable_entertainment;
bool enable_highways;
bool enable_subway;
bool enable_navigate;
int instructions_just_once;

ezgl::surface *icon1;
ezgl::surface *icon2;
ezgl::surface *icon3;
ezgl::surface *icon4;
ezgl::surface *sourceIcon;
ezgl::surface *destinationIcon;

double initial_area;
double for_zoom;
double max_lat;
double min_lat;
double max_lon;
double min_lon;
int just_once;
double area; 
double initial_ratio; 



#endif /* GLOBALS_M2_H */

