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
#include "m1.h"
#include "m2.h"
#include "m3.h"
#include "helperfunction.h"
#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"
#include "StreetsDatabaseAPI.h"
#include "OSMDatabaseAPI.h"
#include "math.h"
#include <string>
#include <chrono>
#include "OSMID.h"
#include "LatLon.h"
#include "OSMEntity.h"
#include "OSMNode.h"
#include "OSMWay.h"
#include "OSMRelation.h"
#include "globals_m2.h"

typedef int Edge;

//Function Declarations 
void draw_main_canvas(ezgl::renderer *g);
void display_name(ezgl::renderer *g, POIIdx POInum, int font_size);
void display_street_names(ezgl::renderer *g);
void display_feature_names(ezgl::renderer *g);
void display_subway_station_names(ezgl::renderer *g);
void find_button(GtkWidget */*widget*/, ezgl::application *application);
void POI_button(GtkWidget */*widget*/, ezgl::application *application);
void Healthcare_button(GtkWidget */*widget*/, ezgl::application *application);
void Entertainment_button(GtkWidget */*widget*/, ezgl::application *application);
void Highways_button(GtkWidget */*widget*/, ezgl::application *application);
void subway_button(GtkWidget */*widget*/, ezgl::application *application);
void act_on_mouse_click(ezgl::application* app, GdkEventButton* event, double x , double y);
void draw_main_canvas_highlight_centered(ezgl::renderer *g);
void initial_setup(ezgl::application* application, bool /*new window*/);
void display_poi_names(ezgl::renderer *g);
void subroutine_display_street_name(int seg, ezgl::renderer *g);
int get_zoom_level(ezgl::renderer *g);
float x_from_lon(float longitude); 
float y_from_lat(float latitude);
float lat_from_y(float y);
float lon_from_x(float x);
int get_num_of_segments(StreetIdx street_id);
void on_dialog_response(GtkDialog *dialog, gint response_id, gpointer user_data);
void test_button(GtkWidget */*widget*/, ezgl::application *application);
void Navigate_button(GtkWidget */*widget*/, ezgl::application *application);
double findAngleBetweenTwoSegmentVectors(IntersectionIdx point1, IntersectionIdx point2, IntersectionIdx point3);
int determineDirectionFromAngle(double angle);
void displayInstructions(ezgl::application* app);
void drawPath(ezgl::renderer *g, std::vector<StreetSegmentIdx> path);
int roundSecond(int sec);

GtkWidget* gtk_dialog_new_with_buttons(
const gchar *title,
GtkWindow *parent,
GtkDialogFlags flags,
const gchar *first_button_text,
GtkDialogFlags flags2,
const gchar *third_button_text
);

bool draw_path = false; 

using namespace std;

string getInitialDirection(IntersectionIdx src, StreetSegmentIdx seg0);

//MAKE NEW FILE FOR ALL STRUCTS!

//Struct for intersection data to create a vector database
struct Inter_data{
    ezgl::point2d xy_loc;
    std::string name;
    //!!!! need to initialize this to false in loadmap, not here 
    bool highlight = false;
};

//Struct for POI data to create a vector database
struct POI_data{
    ezgl::point2d xy_loc;
    std::string POI_type;
    std::string POI_name;
};

//Struct for feature data to create a vector database
struct Feature_data{
    double feature_area;
    FeatureType feature_type;
    FeatureIdx id;
    std::vector <ezgl::point2d> feature_vertices_coordinates;
};

//Struct to help sort the features vector based on feature area in descending order
//This struct will be a parameter for the std::sort function
struct greater_than_area{
    inline bool operator() (const Feature_data& feature1, const Feature_data& feature2){
        return feature1.feature_area>feature2.feature_area;
    }
};

//Struct that contains street segment data
struct street_seg_data{
    ezgl::point2d from;
    ezgl::point2d to;
};

struct streetInfo{
    int id;
    double pathDistance;
};

//MAKE NEW FILE FOR ALL GLOBAL VARIABLES AND MAGIC NUMBERS
std::vector<bool> street_seg_highlight;
std::vector<bool> feature_highlight;
std::vector<bool> poi_highlight;
std::vector<Feature_data> features;
std::vector <Inter_data> intersections;
std::vector <POI_data> pointsOfInterests;
std::vector <const OSMRelation*> osm_subway_stations;
std::vector<ezgl::point2d> line;
std::vector <string> names;
std::vector <StreetSegmentIdx> segment_ids;
std::vector <const OSMRelation*> osm_subway_lines;
std::vector <street_seg_data> segments;
std::vector <const OSMRelation*> osm_highway_lines;
std::vector<int> segment_types; 

std::pair <int,int> source_and_destination;
int clicked_intersection = 0;
std::vector<IntersectionIdx> intersections_in_path;

void drawPOIs(ezgl::renderer *g); 
void drawFeatures(ezgl::renderer *g);
void drawStreets(ezgl::renderer *g);
void drawIntersections(ezgl::renderer *g);
void loadMap();
std::vector<StreetSegmentIdx> path; 
void loadMap(){

    //Defining the icons for the POIs using the external PNGS in the resources folder
    icon1 = ezgl::renderer::load_png("libstreetmap/resources/food2.png");
    icon2 = ezgl::renderer::load_png("libstreetmap/resources/healthcare.png");
    icon3 = ezgl::renderer::load_png("libstreetmap/resources/small_image.png");
    icon4 = ezgl::renderer::load_png("libstreetmap/resources/entertainment.png");
    sourceIcon = ezgl::renderer::load_png("libstreetmap/resources/source.png");
    destinationIcon = ezgl::renderer::load_png("libstreetmap/resources/destination.png");
    
    enable_food = false; 
    enable_healthcare = false; 
    enable_entertainment = false;
    enable_highways = false;
    enable_subway = false;
    
    area_zoom_factor = 2.7777777777778;
    instructions_just_once = 0;
    just_once = 0;
   
    
    //Initializing max_lat, min_lat, max_lon, min_lon to LatLon coordinates of the first intersection point
    max_lat = getIntersectionPosition(0).latitude();
    min_lat = max_lat;
    max_lon = getIntersectionPosition(0).longitude();
    min_lon = max_lon;
    
    
    //Resizing intersections vector
    intersections.resize(getNumIntersections());
    for(int id = 1; id <getNumIntersections(); id++){
        max_lat = std::max(max_lat, getIntersectionPosition(id).latitude());
        min_lat = std::min(min_lat, getIntersectionPosition(id).latitude());
        max_lon = std::max(max_lon, getIntersectionPosition(id).longitude());
        min_lon = std::min(min_lon, getIntersectionPosition(id).longitude());
    }
    
    //defining the cos_latitude_average
    cos_latitude_average = cos(((max_lat + min_lat)*kDegreeToRadian/2));
    
    source_and_destination.first = -1;
    source_and_destination.second = -1;
    
    //initializing and loading the intersections vector
    for(int id = 0; id <getNumIntersections(); id++){
        intersections[id].xy_loc.x = x_from_lon(getIntersectionPosition(id).longitude()); //converting longitude to x
        intersections[id].xy_loc.y = y_from_lat(getIntersectionPosition(id).latitude());  //converting latitude to y
        intersections[id].name = getIntersectionName(id);
    }

    //loading data of POIs into this vector 
    int num_poi = getNumPointsOfInterest();
    pointsOfInterests.resize(num_poi); //resizing pointsOfInterests vector
    for(POIIdx POI_id = 0; POI_id < num_poi; POI_id++){
        pointsOfInterests[POI_id].xy_loc.x = x_from_lon(getPOIPosition(POI_id).longitude());
        pointsOfInterests[POI_id].xy_loc.y = y_from_lat(getPOIPosition(POI_id).latitude());
        pointsOfInterests[POI_id].POI_type = getPOIType(POI_id);
        bool highlight = false;
        poi_highlight.push_back(highlight);
    
    }
    
    segments.resize(getNumStreetSegments()); //resizing segments vector
    
    
    //Initializing segments and street_seg_highlight databases
    for(int seg_id = 0; seg_id < getNumStreetSegments(); seg_id++){
        bool highlight = false;
        street_seg_highlight.push_back(highlight);
        
        //store xy coordinates of from and to points of each street segment
        float one_x = x_from_lon(getIntersectionPosition(getStreetSegmentInfo(seg_id).from).longitude());
        float one_y = y_from_lat(getIntersectionPosition(getStreetSegmentInfo(seg_id).from).latitude());
        float two_x = x_from_lon(getIntersectionPosition(getStreetSegmentInfo(seg_id).to).longitude());
        float two_y = y_from_lat(getIntersectionPosition(getStreetSegmentInfo(seg_id).to).latitude());
        segments[seg_id].from.x = one_x;
        segments[seg_id].from.y = one_y;
        segments[seg_id].to.x = two_x;
        segments[seg_id].to.x = two_y;
    }
    
    features.resize(getNumFeatures()); //resizing the features vector
    
    //Initializing the features vector database
    for(FeatureIdx feature_id = 0; feature_id < getNumFeatures(); feature_id++){
        bool highlight = false;
        feature_highlight.push_back(highlight);
        features[feature_id].feature_area = findFeatureArea(feature_id); //callig findFeatureArea function in milestone 1
        features[feature_id].id = feature_id;
        features[feature_id].feature_type = getFeatureType(feature_id);
        if(getNumFeaturePoints(feature_id)>1){
            
            //Creating a vector for each feature which stores the xy coordinates of the vertices
                for(int feature_vertex = 0; feature_vertex < getNumFeaturePoints(feature_id); feature_vertex++){
                    ezgl::point2d coordinates;
                    coordinates.x = x_from_lon(getFeaturePoint(feature_vertex,feature_id).longitude());
                    coordinates.y = y_from_lat(getFeaturePoint(feature_vertex,feature_id).latitude());
                    features[feature_id].feature_vertices_coordinates.push_back(coordinates);                
                }
        }
    }
    
    //sorting the features vector database in descending order based on feature area
    sort(features.begin(), features.end(), greater_than_area()); 
    
    //loading Subway station data 
    //loop through all subway relations
    for (unsigned i = 0; i < getNumberOfRelations(); i++){
        const OSMRelation *current_relation = getRelationByIndex(i);
        for (unsigned j = 0; j < getTagCount(current_relation);j++){
            pair <string, string> tagPair = getTagPair(current_relation, j);
            //push relation with the tag route=subway in the vector 
            if (tagPair.first == "route" && tagPair.second == "subway"){
                osm_subway_lines.push_back(current_relation);
                break;
            }
        }
    }

    std::map<OSMID, const OSMWay*> vis;
     for (int i = 0; i < getNumberOfWays();i++){
         vis[getWayByIndex(i)->id()] = getWayByIndex(i);
 
        }
    segment_types.resize(getNumStreetSegments());
    
     for(int street_segment_id = 0; street_segment_id < getNumStreetSegments(); street_segment_id++){
       
        StreetSegmentInfo streetSegInfo = getStreetSegmentInfo(street_segment_id);
        auto currId = streetSegInfo.wayOSMID;
        const OSMWay* current_way = vis[currId];
        
        //OSMWay* current_way = getWayByIndex((OSMID)currId);
        
        for (unsigned j = 0; j < getTagCount(current_way);j++){
            pair <string, string> tagPair = getTagPair(current_way, j);
            //push relation with the tag route=subway in the vector 
            if (tagPair.first == "highway"){// && tagPair.second == "highway"){
                if(tagPair.second == "motorway" || tagPair.second == "trunk" ){
                    segment_types[street_segment_id]=0;
                }
                else if( tagPair.second == "secondary"){
                    segment_types[street_segment_id]=1;
                }
                else{
                    segment_types[street_segment_id]=2;
                }
                break;
            }
        }
}
}

void drawMap() {
    
    //Initialize all databases
    loadMap();
    ezgl::rectangle initial_world({x_from_lon(min_lon),y_from_lat(min_lat)}, {x_from_lon(max_lon),y_from_lat(max_lat)}); //define initial world
    
    initial_area = initial_world.area();

    // nconfigure everything required to draw the canvas and window
    ezgl::application::settings settings;
    settings.main_ui_resource = "libstreetmap/resources/main.ui";
    settings.window_identifier = "MainWindow";
    settings.canvas_identifier = "MainCanvas";
    
    ezgl::application application(settings);
    application.add_canvas("MainCanvas", draw_main_canvas, initial_world, ezgl::PLATINUM); 
    
    application.run(initial_setup, act_on_mouse_click, nullptr, nullptr);    
}

void draw_main_canvas(ezgl::renderer *g){
    
    //Draw components in the correct order to have right layers
    drawFeatures(g);
    
    if (enable_navigate&&clicked_intersection == 2){
        path = findPathBetweenIntersections(15,source_and_destination);
        clicked_intersection = 0;
    }
    drawStreets(g); 
    
    drawIntersections(g);
    drawPOIs(g);
    
    //draw street_names when zoom level is 4 or higher or when highways are enabled
    if (get_zoom_level(g)>=4||enable_highways){
    display_street_names(g); }
   
    display_feature_names(g);
    
    //Decide when to draw POI names
    /*if (enable_food||enable_healthcare|| enable_entertainment){
        display_poi_names(g);
    }*/
    
    //Decide when to draw subways
    if (enable_subway){
        cout<< "Subway button clicked"<< endl;
        display_subway_station_names(g);
    }    
    //Perform all necessary calculations required for the zoom feature
    if (just_once == 0){
        area = (g->get_visible_world().right()-g->get_visible_world().left())*(g->get_visible_world().top()-g->get_visible_world().bottom()); 
        initial_ratio = initial_area/area;
        just_once = 1;
    }
    
    
    
    
}

//Initialize all buttons
void initial_setup(ezgl::application* application, bool /*new window*/){
    application->create_button("Find", 6, find_button);
    application->create_button("Highways", 8, Highways_button);
    application->create_button("Subways", 9, subway_button);
    application->create_button("Food", 10, POI_button);
    application->create_button("Healthcare", 11, Healthcare_button);
    application->create_button("Entertainment", 12, Entertainment_button);
    application->create_button("Help", 13, test_button);
    application->create_button("Navigate", 7, Navigate_button);   

}

//This function draws all the intersections
void drawIntersections(ezgl::renderer *g){
    for(size_t inter_id =0; inter_id < intersections.size(); inter_id++){       
        double width = 2;
        double height = width;
        ezgl:: point2d inter_loc = intersections[inter_id].xy_loc - ezgl::point2d{width/2, height/2};
        
        //Only display intersections when zoom level is 7 or higher
        if(get_zoom_level(g)>=7 || inter_id == source_and_destination.first || inter_id == source_and_destination.second){   
            
            //Draw icon for source location
            if(inter_id == source_and_destination.first){
                g->draw_surface(sourceIcon, intersections[inter_id].xy_loc, 0.1);
            }
            
            //Draw icon for destination location
            else if(inter_id == source_and_destination.second){
                g->draw_surface(destinationIcon, intersections[inter_id].xy_loc, 0.1);
            }
            
            //Draw a big red square for highlighted intersections
            else if(intersections[inter_id].highlight) {
                width = 10;
                height = width;
                g->set_color(ezgl::RED);
                g->fill_rectangle(inter_loc, width, height);
            }
            
            //Draw a small white rectangle for all intersections
            else {
                g->set_color(ezgl::WHITE);
                g->fill_rectangle(inter_loc, width, height);
            }
        }            
    }
} 

//This function draws all the POIs (icons, stars, or a blue rectangle depending on the case)
void drawPOIs(ezgl::renderer *g){  
    float width = 10;
    float height = width;
    double scale = 0.05;
    if(get_zoom_level(g)>=9){
        scale = 0.1;
    }
    
    //Only display POIs when zoom level is 7 or higher
    if (get_zoom_level(g)>=7){    
        for(POIIdx POI_id = 0; POI_id < getNumPointsOfInterest(); POI_id++){
            //if(poi_highlight[POI_id] == true ) g->set_color(ezgl::BLUE);
            //else g->set_color(ezgl::PINK);
            ezgl :: point2d POI_coord;
            POI_coord.x = pointsOfInterests[POI_id].xy_loc.x;
            POI_coord.y = pointsOfInterests[POI_id].xy_loc.y;
            std::string poi_type = pointsOfInterests[POI_id].POI_type;
            
            if(enable_food){
                
                if(poi_highlight[POI_id] == true){
                    g->set_color(ezgl::BLUE);
                    g-> fill_rectangle(POI_coord, {POI_coord.x + width, POI_coord.y + height});
                }
                
                if(poi_type == "restaurant"||poi_type == "fast_food" ){
                    g->draw_surface(icon1, POI_coord, scale);
                }
            }
            if(enable_healthcare){
                
                if(poi_highlight[POI_id] == true){
                    g->set_color(ezgl::BLUE);
                    g-> fill_rectangle(POI_coord, {POI_coord.x + width, POI_coord.y + height});
                }
                
                if(poi_type == "doctors"||poi_type == "pharmacy"||poi_type == "clinic" ){
                    g->draw_surface(icon2, POI_coord, scale);
                }
            }
            if(enable_entertainment){
                
                if(poi_highlight[POI_id] == true){
                    g->set_color(ezgl::BLUE);
                    g-> fill_rectangle(POI_coord, {POI_coord.x + width, POI_coord.y + height});
                }
                
                if (poi_type == "night_club"||poi_type == "events_venue"||poi_type == "community_centre" ||poi_type == "bar"){
                    g->draw_surface(icon4, POI_coord, scale);
                }
            }
            if(!enable_food && !enable_healthcare && !enable_entertainment){
                //If POI is highlighted then draw a blue rectangle for it
                if(poi_highlight[POI_id] == true){
                    g->set_color(ezgl::BLUE);
                    g-> fill_rectangle(POI_coord, {POI_coord.x + width, POI_coord.y + height});
                }

                //Display the healthcare icon for this case
                else if(poi_type == "doctors"||poi_type == "pharmacy"||poi_type == "clinic" ){
                    g->draw_surface(icon2, POI_coord, scale);
                }

                //Display the food icon for this case
                else if(poi_type == "restaurant"||poi_type == "fast_food" ){
                    g->draw_surface(icon1, POI_coord, scale);
                }
                
                //Display entertainment icon for this case
                else if (poi_type == "night_club"||poi_type == "events_venue"||poi_type == "community_centre" ||poi_type == "bar"){
                    g->draw_surface(icon4, POI_coord, scale);
                }

                //draw a blue star for the rest of the icons
                else {
                    g->draw_surface(icon3, POI_coord, 0.7);
                }   
                //g-> fill_rectangle(POI_coord, {POI_coord.x + width, POI_coord.y + height});
            }     
        }
    }
}

//This function draws all the features and gives it the relevant colour
void drawFeatures(ezgl::renderer *g){
 //Draw features from biggest to smallest using the database created in loadMaps()
 //This will account for overlapping features
 for(FeatureIdx feature_count = 0; feature_count < features.size(); feature_count++){
        FeatureIdx currentFeatureId = features[feature_count].id;
        if(getNumFeaturePoints(currentFeatureId)>1){ 

            //Decide colour according to feature type
            int currentFeatureType = features[feature_count].feature_type;
            switch(currentFeatureType){
                case 1: //park
                    g->set_color(ezgl::TEA_GREEN);
                    break;
                case 2: //beach
                    g->set_color(ezgl::BANANA_MANIA);
                    break;
                case 3: //lake
                    g->set_color(ezgl::OCEAN_BLUE);
                    break;
                case 4: //river
                    g->set_color(ezgl::OCEAN_BLUE);
                    break;
                case 5: //island
                    g->set_color(ezgl::PLATINUM);
                    break;
                case 6: //building
                    g->set_color(ezgl::GREY_75);
                    break;
                case 7: //greenspace
                    g->set_color(ezgl::TEA_GREEN);
                    break;
                case 8: //golfcourse
                    g->set_color(ezgl::LIME_GREEN);
                    break;
                case 9: //stream
                    g->set_color(ezgl::OCEAN_BLUE);
                    break;
                case 10: //glacier
                    g->set_color(ezgl::OCEAN_BLUE);
                    break;
                default:
                    break;
            }
            
            
            //Dont draw buildings when zoom level is 3 or lesser
            //Draw everything when zoom level is 4 or higher
            //Highlighted features should be drawn all the time
            if((get_zoom_level(g) <= 3 && currentFeatureType != 6) || get_zoom_level(g) >= 4 || feature_highlight[currentFeatureId] == true){
                if(feature_highlight[currentFeatureId] == true) g->set_color(ezgl::BLUE);
                int last_point_index = getNumFeaturePoints(currentFeatureId)-1;
                
                //When the feature's first and last vertex are the same
                if(getFeaturePoint(0,currentFeatureId) ==  getFeaturePoint(last_point_index,currentFeatureId)){   
                    g->fill_poly(features[feature_count].feature_vertices_coordinates);
                }
                
                //When the feature's first and last vertex aren't the same
                if(!(getFeaturePoint(0,currentFeatureId) ==  getFeaturePoint(last_point_index,currentFeatureId))){
                    for(int vertex = 0; vertex < features[feature_count].feature_vertices_coordinates.size()-1; vertex++){
                        g->draw_line(features[feature_count].feature_vertices_coordinates[vertex], features[feature_count].feature_vertices_coordinates[vertex+1]);   
                    }
                }
            }
        }
    }
}

//This function draws all the streets and colour codes them based on speed limit
void drawStreets(ezgl::renderer *g){
   
    for(int street_segment_id = 0; street_segment_id < getNumStreetSegments(); street_segment_id++){
         std::vector<ezgl::point2d> street_points;
        StreetSegmentInfo streetSegInfo = getStreetSegmentInfo(street_segment_id);
        street_points.push_back(intersections[streetSegInfo.from].xy_loc);
        g->set_line_cap(ezgl::line_cap::round); //rounding line edges to have smooth roads
        
        //If speed limit for the street segment is more than 25mps then colour it yellow
        if(segment_types[street_segment_id]==0){
            g->set_color(ezgl::YELLOW);
            g->set_line_width(5);
        }
        
        //If speed limit for the street segment is between 22.22222mps and 25mps then colour it red
        if(segment_types[street_segment_id]==1){
            g->set_color(ezgl::WHITE);
            g->set_line_width(2);
        }

        //If speed limit for the street segment is less than 22.22222 then colour it white
        if(segment_types[street_segment_id]==2){
            g->set_color(ezgl::WHITE);
            g->set_line_width(2);
        }
        
        //if the segment should be highlighted then colour it blue
        
        if(street_seg_highlight[street_segment_id] == true){
            g->set_color(ezgl::GREEN);
            g->set_line_width(5);
            
        }
        
        if (enable_navigate){
            for (int path_id = 0; path_id<path.size(); path_id++){
                if(street_segment_id==path[path_id]){
                    g->set_color(ezgl::MAGENTA);
                    g->set_line_width(5);
                
                    if(streetSegInfo.numCurvePoints == 0){
                g->draw_line(intersections[streetSegInfo.to].xy_loc, intersections[streetSegInfo.from].xy_loc);           
            }
            
            //Code to draw street segments when there are curve points
            else {
                ezgl::point2d coordinates;
                LatLon firstCurvePoint = getStreetSegmentCurvePoint(0, street_segment_id);
                coordinates.x = x_from_lon(firstCurvePoint.longitude());
                coordinates.y = y_from_lat(firstCurvePoint.latitude());
                
                //Draw each line between consecutive curve points one by one
                
                g->draw_line(intersections[streetSegInfo.from].xy_loc, coordinates);
                for(int point = 0; point < streetSegInfo.numCurvePoints -1; point++){
                    ezgl::point2d coordinates2;
                    LatLon currentCurvePoint, nextCurvePoint;
                    currentCurvePoint = getStreetSegmentCurvePoint(point, street_segment_id);
                    nextCurvePoint = getStreetSegmentCurvePoint(point+1, street_segment_id);
                    coordinates.x = x_from_lon(currentCurvePoint.longitude());
                    coordinates.y = y_from_lat(currentCurvePoint.latitude());
                    coordinates2.x = x_from_lon(nextCurvePoint.longitude());
                    coordinates2.y = y_from_lat(nextCurvePoint.latitude());

                    g->draw_line(coordinates,coordinates2);

                }
                LatLon lastCurvePoint = getStreetSegmentCurvePoint(streetSegInfo.numCurvePoints-1, street_segment_id);
                coordinates.x = x_from_lon(lastCurvePoint.longitude());
                coordinates.y = y_from_lat(lastCurvePoint.latitude());  
                g->draw_line(intersections[streetSegInfo.to].xy_loc, coordinates); 
            }
                    
                }
            }
        }
        
        
        //Only display highways when zoom level is less than or equal to 3 and the highways button is pressed
        //Smaller streets wont be displayed when zoom level is low
        if((get_zoom_level(g)<=3 && segment_types[street_segment_id]==0 && enable_highways) || (get_zoom_level(g)>=4)){
            
            //Code to draw street segments when there are no curve points
            if(streetSegInfo.numCurvePoints == 0){
                g->draw_line(intersections[streetSegInfo.to].xy_loc, intersections[streetSegInfo.from].xy_loc);           
            }
            
            //Code to draw street segments when there are curve points
            else {
                ezgl::point2d coordinates;
                LatLon firstCurvePoint = getStreetSegmentCurvePoint(0, street_segment_id);
                coordinates.x = x_from_lon(firstCurvePoint.longitude());
                coordinates.y = y_from_lat(firstCurvePoint.latitude());
                
                //Draw each line between consecutive curve points one by one
                
                g->draw_line(intersections[streetSegInfo.from].xy_loc, coordinates);
                for(int point = 0; point < streetSegInfo.numCurvePoints -1; point++){
                    ezgl::point2d coordinates2;
                    LatLon currentCurvePoint, nextCurvePoint;
                    currentCurvePoint = getStreetSegmentCurvePoint(point, street_segment_id);
                    nextCurvePoint = getStreetSegmentCurvePoint(point+1, street_segment_id);
                    coordinates.x = x_from_lon(currentCurvePoint.longitude());
                    coordinates.y = y_from_lat(currentCurvePoint.latitude());
                    coordinates2.x = x_from_lon(nextCurvePoint.longitude());
                    coordinates2.y = y_from_lat(nextCurvePoint.latitude());

                    g->draw_line(coordinates,coordinates2);

                }
                
                //DRaw line from last curve point to to point
                LatLon lastCurvePoint = getStreetSegmentCurvePoint(streetSegInfo.numCurvePoints-1, street_segment_id);
                coordinates.x = x_from_lon(lastCurvePoint.longitude());
                coordinates.y = y_from_lat(lastCurvePoint.latitude());  
                g->draw_line(intersections[streetSegInfo.to].xy_loc, coordinates); 
            }
        }
    }  
}

void act_on_mouse_click(ezgl::application* app, GdkEventButton* /*event*/, double x , double y){
    
    LatLon pos = LatLon (lat_from_y(y), lon_from_x(x));
    int id = findClosestIntersection(pos); 
    intersections[id].highlight = !intersections[id].highlight; 
    app->update_message("Closest Intersection: " + intersections[id].name);
    
    if (enable_navigate && clicked_intersection == 1){
         
         
         if (source_and_destination.first == id){
             app->update_message("You are already at your destination!");
             goto chooseAgain; 
         }
         source_and_destination.second = id; 
         
         draw_path = true; 
    
         displayInstructions(app);
         
         clicked_intersection = 2;
         
         chooseAgain:;

    }
    
    else if (enable_navigate && clicked_intersection == 0){
        source_and_destination.first = id;          
        app->update_message("Click on your destination");
        clicked_intersection = 1;
        instructions_just_once = 0;
    }
    app->refresh_drawing();
    std::cout << "Closest Intersection: "<< intersections[id].name << endl; 

}


//Subroutine for 'find' button
void find_button(GtkWidget */*widget*/, ezgl::application *application)
{
  // Get the GtkEntry objects
    GtkEntry* text_entry = (GtkEntry *) application->get_object("TextInput1");
    GtkEntry* text_entry_2 = (GtkEntry *) application->get_object("TextInput2");
  // Get the text written in the widgets
    const char* text = gtk_entry_get_text(text_entry);
    const char* text2 = gtk_entry_get_text(text_entry_2);
  // convert to std::string type
    std::string str = text;
    std::string str2 = text2;
   // check if there are two text inputs
    if(str2 == ""){
        //if not, check if the text entered by user matches a street name
        std::vector<StreetIdx> street_ids = findStreetIdsFromPartialStreetName(str);
        if(street_ids.size() == 0){
            //if not, check if the text input matches a feature name
            bool found = false;
            int i;
            for(i = 0; i < getNumFeatures(); i++){
                //compare each feature name to the given input
                if(getFeatureName(i) == str){
                    //if a match is found, highlight that feature
                    feature_highlight[i] = true;
                    found = true;
                    break;
                }
            }
            if(!found){
                //if the text input doesn't match a feature, check if it matches a POI
                for(i = 0; i < getNumPointsOfInterest(); i++){
                    //compare each POI name to the given input
                    if(getPOIName(i) == str){
                        //if a match is found, highlight that POI
                        poi_highlight[i] = true;
                        found = true;
                        break;
                    }
                }
                if(!found){
                    //if the text input doesn't match a POI, print an error message and return from subroutine
                    application->update_message("Error: Incorrect Input");
                    return;
                }
                //print the name of the POI to the application interface and return from the subroutine
                application->update_message(getPOIName(i));
                application->refresh_drawing();
                return;
            }
             //print the name of the feature to the application interface and return from the subroutine
            application->update_message(getFeatureName(i));
            application->refresh_drawing();
            return;
        }
        StreetIdx id = street_ids[0];
        for(int i = 0; i < getNumStreetSegments(); i++){
            //highlight each street segment of the matching street
            if(getStreetSegmentInfo(i).streetID == id) street_seg_highlight[i] = true;
        }
        //print the name of the street to the application interface and return from the subroutine
        application->update_message(getStreetName(id));
        application->refresh_drawing();
        return;
    }
    
    std::size_t loc1 = str.find("&");
    std::size_t loc2 = str2.find("&");
    //check if both text inputs specify an intersection by checking if they both contain ampersand
    if(loc1 != std::string::npos && loc2 != std::string::npos){
        //extract the street names that would specify the intersections
        std::string str_first_name = str.substr(0, loc1);
        std::string str_second_name = str.substr(loc1 + 1);
        std::string str2_first_name = str2.substr(0, loc2);
        std::string str2_second_name = str2.substr(loc2 + 1);
        //check if the street names are legal
        std::vector<StreetIdx> str_first_id = findStreetIdsFromPartialStreetName(str_first_name);
        std::vector<StreetIdx> str_second_id = findStreetIdsFromPartialStreetName(str_second_name);
        std::vector<StreetIdx> str2_first_id = findStreetIdsFromPartialStreetName(str2_first_name);
        std::vector<StreetIdx> str2_second_id = findStreetIdsFromPartialStreetName(str2_second_name);
        if((str_first_id.size() == 0) || (str_second_id.size() == 0) || (str2_first_id.size() == 0) || (str2_first_id.size() == 0)){
            //if any of the strings do not match a legal street name, print an error message and return
            application->update_message("Error: incorrect street name");
            return;
        }
        std::pair<StreetIdx, StreetIdx> str_id_pair;
        std::vector<IntersectionIdx> str_common_ids;
        bool found = false;
        //check if the first pair of streets intersect and if so, obtain the intersection id
        for(int i = 0; i < str_first_id.size() && !found; i++){
            for(int j = 0; j < str_second_id.size() && !found; j++){
                //compare every possible street that may be specified by the first pair of street name inputs to see if an intersection exists
                str_id_pair.first = str_first_id[i];
                str_id_pair.second = str_second_id[j];
                str_common_ids = findIntersectionsOfTwoStreets(str_id_pair);
                if(str_common_ids.size()) found = true;
            }
        }
        if(!found){
            //if no intersection exists, print an error message and return
            application->update_message("No intersections found");
            return;
        }
        
        found = false;
        std::pair<StreetIdx, StreetIdx> str2_id_pair;
        std::vector<IntersectionIdx> str2_common_ids;
        //check if the second pair of streets intersect and if so, obtain the intersection id
        for(int i = 0; i < str2_first_id.size() && !found; i++){
            for(int j = 0; j < str2_second_id.size() && !found; j++){
                //compare every possible street that may be specified by the second pair of street name inputs to see if an intersection exists
                str2_id_pair.first = str2_first_id[i];
                str2_id_pair.second = str2_second_id[j];
                str2_common_ids = findIntersectionsOfTwoStreets(str2_id_pair);
                if(str2_common_ids.size()) found = true;
            }
        }
        if(!found){
            //if no intersection exists, print an error message and return
            application->update_message("No intersections found");
            return;
        }
        
        source_and_destination.first = str_common_ids[0];
        source_and_destination.second = str2_common_ids[0];
        path = findPathBetweenIntersections(15, source_and_destination);
        draw_path = true;
        enable_navigate = true;
        displayInstructions(application);
        application->refresh_drawing();
        
        return;
        
        
    }
    
    
    
    
    //if there are two inputs, check if they are two valid streets
    std::vector<StreetIdx> first_id = findStreetIdsFromPartialStreetName(str);
    std::vector<StreetIdx> second_id = findStreetIdsFromPartialStreetName(str2);
    if((first_id.size() == 0) || (second_id.size() == 0)){
        //if any of the inputs do not match a street name, print an error message and return from the subroutine
        application->update_message("Error: incorrect street name");
        return;
    }
    
    std::pair<StreetIdx, StreetIdx> id_pair;
    std::vector<IntersectionIdx> common_ids;
    bool found = false;
    //check if the streets intersect and if so, obtain the intersection id
    for(int i = 0; i < first_id.size() && !found; i++){
            for(int j = 0; j < second_id.size() && !found; j++){
                //compare every possible street that may be specified by the street name inputs to see if an intersection exists
                id_pair.first = first_id[i];
                id_pair.second = second_id[j];
                common_ids = findIntersectionsOfTwoStreets(id_pair);
                if(common_ids.size()) found = true;
            }
        }
    if(!found){
        //if no intersections found, print an error message and return
        application->update_message("No intersections found");
        return;
    }
    for(int i = 0; i < common_ids.size(); i++){
        //highlight each intersection
        StreetIdx id = common_ids[i];
        intersections[id].highlight = true;
    }
    application->refresh_drawing();
    //print information about the intersections to user interface and terminal
    application->update_message(str + " & " + str2);
    std::cout << common_ids.size() << " intersections between " << getStreetName(id_pair.first) << " and " << getStreetName(id_pair.second) << ":" << std::endl;
    for(int i = 0; i < common_ids.size(); i++){
        LatLon pos = getIntersectionPosition(common_ids[i]);
        std::cout << "Latitude: " << pos.latitude() << " Longitude: " << pos.longitude() << std::endl;
    }
 
}

//This function decides when to display the POI names
void display_poi_names(ezgl::renderer *g){
    
    g->set_color(ezgl::BLACK);
    int current_zoom = get_zoom_level(g);

    for(POIIdx POInum = 0; POInum < pointsOfInterests.size(); POInum+=3){
        string poi_type = pointsOfInterests[POInum].POI_type;
        //cout << poi_type << endl; 
        if (current_zoom == -1){return;}
        if (current_zoom >= 6){
            //Display names based on the enables(toggles)
            if (enable_food == true){
                if (poi_type == "restaurant"||poi_type == "fast_food" ){
                    display_name(g, POInum, 10);
                }
            }
            if (enable_healthcare == true){
                if (poi_type == "doctors"||poi_type == "pharmacy"||poi_type == "clinic" ){
                    display_name(g, POInum, 10);
                } 
            }
            if (enable_entertainment == true){
                if (poi_type == "night_club"||poi_type == "events_venue"||poi_type == "community_centre" ||poi_type == "bar"){
                    display_name(g, POInum, 10);
                } 
            }
        }    
    }
}

//Function to configure while drawing text
void display_name(ezgl::renderer *g, POIIdx POInum, int font_size){
    g->set_font_size(font_size);
    g->draw_text (pointsOfInterests[POInum].xy_loc, getPOIName(POInum));
}

//Function to display feature names
void display_feature_names(ezgl::renderer *g){
    g->set_color(ezgl::BLACK);
    g->set_text_rotation(0);
    int num_features = getNumFeatures(); 
    int random_counter = 0;
    for (int id = 0; id < num_features; id+=4){
       ezgl :: point2d feature_coord;
       feature_coord.x = x_from_lon(getFeaturePoint(0, id).longitude());
       feature_coord.y = y_from_lat(getFeaturePoint(0, id).latitude());
       
        g->set_font_size(10);
        string type_name = asString(getFeatureType(id));
        string name = getFeatureName(id);
        
        if (get_zoom_level(g)<=3){
        if (name!="<noname>"){
            if (type_name == "lake"||type_name == "river"){
                g->set_color(ezgl::CERULEAN_BLUE);
                g->draw_text (feature_coord, (getFeatureName(id)));
            }
            if (type_name == "park"||type_name == "greenspace"){
                random_counter ++;
                //remove magic number
                if (random_counter == 59){
                    g->set_color(ezgl::JADE);
                    g->draw_text (feature_coord, (getFeatureName(id)));
                    random_counter = 0;}
                }
            } 
        }
    } 
}

//Function to display street names
void display_street_names(ezgl::renderer * g) {
  int previous_seg = 0;
  int current_seg;
  g -> set_font_size(10);
  g -> set_color(ezgl::BLACK);
  int num_of_segs = getNumStreetSegments();

  for (int seg = 0; seg < num_of_segs; ++seg) {

    current_seg = seg;
    if ((getStreetSegmentInfo(previous_seg).streetID) == (getStreetSegmentInfo(current_seg).streetID)) {
      std::pair < LatLon, LatLon > coords;
      coords.first = (getIntersectionPosition(getStreetSegmentInfo(previous_seg).from));
      coords.second = (getIntersectionPosition(getStreetSegmentInfo(current_seg).from));
      if (findDistanceBetweenTwoPoints(coords) >= 10000) {
        subroutine_display_street_name(seg, g);
        previous_seg = current_seg;

      }
    } else {
      subroutine_display_street_name(seg, g);
      previous_seg = current_seg;
    }
  }
}

//Calculates the middle point of the street from the LatLon ends of the Street Segments
//Calculates the rotation angle and sets it 
//if the street is one way, appends a direction to the street name
//Draws text on canvas 
void subroutine_display_street_name(int seg, ezgl::renderer * g) {
   float one_x = x_from_lon(getIntersectionPosition(getStreetSegmentInfo(seg).from).longitude());
   float one_y = y_from_lat(getIntersectionPosition(getStreetSegmentInfo(seg).from).latitude());
   float two_x = x_from_lon(getIntersectionPosition(getStreetSegmentInfo(seg).to).longitude());
   float two_y = y_from_lat(getIntersectionPosition(getStreetSegmentInfo(seg).to).latitude());

   float mid_x = (one_x + two_x) / 2.0;
   float mid_y = (one_y + two_y) / 2.0;

   ezgl::point2d centre(mid_x, mid_y);
   float x2_minus_x1 = two_x - one_x;
   float y2_minus_y1 = two_y - one_y;

   double rotation_angle = atan((y2_minus_y1) / x2_minus_x1) / kDegreeToRadian;
   
   if (x2_minus_x1 != 0) {
      g -> set_text_rotation(rotation_angle);
   }
   
   if (getStreetSegmentInfo(seg).oneWay){
       
       if ((one_x < two_x) && (one_y > two_y)){
           getStreetName(getStreetSegmentInfo(seg).streetID).append("-->");
           
       }
       else {
           getStreetName(getStreetSegmentInfo(seg).streetID).append("<--");
           
       }
   }

   if (getStreetName(getStreetSegmentInfo(seg).streetID) != "<unknown>") {
      g -> draw_text(centre, getStreetName(getStreetSegmentInfo(seg).streetID), findStreetSegmentLength(seg), findStreetSegmentLength(seg));
   }
}

//function to display subway station names 
void display_subway_station_names(ezgl::renderer *g){
   
    g->set_font_size(10);
    
    std::vector <string> colours;
    int current_zoom = get_zoom_level(g);
    for (unsigned i = 0; i < osm_subway_lines.size()/2; i++){
        for (unsigned j = 0; j < getTagCount(osm_subway_lines[i]); j++) {
            pair<string, string> tagPair = getTagPair(osm_subway_lines[i], j);
            
            if (tagPair.first == "colour"){
                
                colours.push_back(tagPair.second);
            }
        }
              
        std::vector <TypedOSMID> route_members = getRelationMembers(osm_subway_lines[i]);
        ezgl::point2d Loc;Loc.x = 0; Loc.y = 0;
        
        for (unsigned j = 0; j < route_members.size(); j++){
            if (route_members[j].type()==TypedOSMID::Node){
                const OSMNode *current_node = nullptr;
                
                for (unsigned k = 0; k < getNumberOfNodes();k++){
                    current_node = getNodeByIndex(k);
                    if (current_node->id()==route_members[j]){
                        break;
                    }
                }
                
                for (unsigned k = 0; k< getTagCount(current_node); k++){
                    pair <string, string> tagPair = getTagPair(current_node, k);
                    
                    
                    if (tagPair.first == "name"){
                        
                        Loc.x = x_from_lon(getNodeCoords(current_node).longitude());
                        Loc.y = y_from_lat(getNodeCoords(current_node).latitude());
                        
                        line.push_back(Loc);
                        
                        
                        if (current_zoom >= 4){
                            g->set_color(ezgl::BLACK);
                            g->draw_text(Loc, tagPair.second);  
                        }
                        break;
                    }   
                }

            }
            
            
        }
        int t = 1;
        // subway lines colours 
        switch(i){
            case 1: g->set_color(ezgl::ORANGE); t++; break;
            case 2: g->set_color(ezgl::PURPLE); t++; break;
            case 3: g->set_color(ezgl::RED); t++; break;
            case 4: g->set_color(ezgl::YELLOW); t++; break;
            default: g->set_color(ezgl::GREEN);
        }
        
        
                    if (line.size()>2){
                        for (int a = 0; a<=line.size()-2; a++){
                            //cout<< "lines are being drawn" << endl;
                            g->set_line_width(6);
                            g->draw_line(line[a],line[a+1]);
                        }
                    }else if (line.size()==2){
                       g->set_line_width(5);
                       g->draw_line(line[0],line[1]); 
                    }
                line.resize(0);
    }     
}

//Callback function for subway_button
void subway_button(GtkWidget */*widget*/, ezgl::application *application){
    enable_subway = !enable_subway;
    application->refresh_drawing();  
}

//Callback function for POI_button 
void POI_button(GtkWidget */*widget*/, ezgl::application *application){
    enable_food = !enable_food;
    application->refresh_drawing();
}

//Callback function for Healthcare_button 
void Healthcare_button(GtkWidget */*widget*/, ezgl::application *application){
    enable_healthcare = !enable_healthcare;
    application->refresh_drawing();
}

//Callback function for Entertainment_button
void Entertainment_button(GtkWidget */*widget*/, ezgl::application *application){
    enable_entertainment = !enable_entertainment;
    application->refresh_drawing();
}

//Callback function for Highways_button
void Highways_button(GtkWidget */*widget*/, ezgl::application *application){
    enable_highways = !enable_highways;
    application->refresh_drawing();
}

//Callback function for Navigate_button
void Navigate_button(GtkWidget */*widget*/, ezgl::application *application){
    enable_navigate = !enable_navigate;
    application->refresh_drawing();
}

//Function to get current zoom level
int get_zoom_level(ezgl::renderer *g){

    double ratio = initial_area/(g->get_visible_world().area());
    double factor = 1; 
    
    //MAGIC NUMER
    double factors[13];
    for (int i = 0; i<13; i++){
        factors[i]= factor; 
        factor*=area_zoom_factor;
    }
    // testing ratios to get how many times the user clicked zoom in 
    if (ratio <= factors[0]){return -1;}
    else if (ratio <= (initial_ratio*factors[1]+1)){return 1;}
    else if (ratio <= (initial_ratio*factors[2]+1)){return 2;}
    else if (ratio <= (initial_ratio*factors[3]+1)){return 3;}
    else if (ratio <= (initial_ratio*factors[4]+1)){return 4;}
    else if (ratio <= (initial_ratio*factors[5]+1)){return 5;}
    else if (ratio <= (initial_ratio*factors[6]+1)){return 6;}
    else if (ratio <= (initial_ratio*factors[7]+1)){return 7;}
    else if (ratio <= (initial_ratio*factors[8])+1){return 8;}
    else if (ratio <= (initial_ratio*factors[9])+1){return 9;}
    else if (ratio <= (initial_ratio*factors[10])+1){return 10;}
    else if (ratio <= (initial_ratio*factors[11])+1){return 11;}
    else if (ratio <= (initial_ratio*factors[12])+1){return 12;}
    else return 1000;
}

//Convert lon to x
float x_from_lon(float longitude){
    
    float x =kEarthRadiusInMeters*longitude*kDegreeToRadian*cos_latitude_average;
    return x;
}

//Convert lat to y
float y_from_lat(float latitude){
    
    float y = kEarthRadiusInMeters*latitude*kDegreeToRadian;
    return y;
    
}

//Convert y from lat
float lat_from_y(float y){
    
    float latitude = y/(kEarthRadiusInMeters*kDegreeToRadian);
    return latitude;
    
}

//convert x to lon
float lon_from_x(float x){
    
    float longitude = x/(kEarthRadiusInMeters*kDegreeToRadian*cos_latitude_average);
    return longitude;
}

//functions made for m3
void test_button(GtkWidget */*widget*/, ezgl::application *application)
{
    GObject *window; // the parent window over which to add the dialog
    GtkWidget *content_area; // the content area of the dialog
    GtkWidget *label; // the label we will create to display a message in the content area
    GtkWidget *dialog; // the dialog box we will create
    
    // Update the status bar message
    application->update_message("Help button pressed: See dialog box for instructions.");
  
    // Redraw the main canvas
    application->refresh_drawing();
  
    // get a pointer to the main application window
    window = application->get_object(application->get_main_window_id().c_str());
    
    dialog = gtk_dialog_new_with_buttons(
    "Help",
    (GtkWindow*) window,
    GTK_DIALOG_MODAL,
    ("OK"),
    GTK_RESPONSE_REJECT,
    NULL);
    
    // Create a label and attach it to the content area of the dialog
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    label = gtk_label_new("BUTTONS: \n"
            "Highways: Displays highways when clicked. \n"
            "Subways: Displays subway lines and stations when clicked. \n"
            "Food: Displays POIs related to food when zoomed in. \n"
            "Healthcare: Displays POIs related to healthcare when zoomed in. \n"
            "Entertainment: Displays POIs related to entertainment when zoomed in. \n"
            "Navigate: Allows you to click on two locations and get directions on how to go from the first location to the second. \n"
            "Clear: Resets the map by clearing any highlighting. \n"
            "Proceed: Closes the application. \n \n"
            "THINGS TO REMEMBER: \n"
            "- Make sure to click on the 'Navigate' button before clicking or entering into the search bars a current or final destination. \n"
            "- When using the search bars to find a path between locations, make sure to enter intersections with the street names separated by '&'.\n"
            "- Check the terminal to see detailed directions on how to go from one location to another. \n"
            
            );
    gtk_container_add(GTK_CONTAINER(content_area), label);
    
    // The main purpose of this is to show dialog?s child widget, label
    gtk_widget_show_all(dialog);
    
    // Connecting the "response" signal from the user to the associated callback function
    g_signal_connect(
    GTK_DIALOG(dialog),
    "response",
    G_CALLBACK(on_dialog_response),
    NULL
    );
    
    
}


void on_dialog_response(GtkDialog *dialog, gint response_id, gpointer /*user_data*/)
{

    // For demonstration purposes, this will show the enum name and int value of the button that was pressed
    switch(response_id) {
        case GTK_RESPONSE_ACCEPT:
        break;
        case GTK_RESPONSE_DELETE_EVENT:
        break;
        case GTK_RESPONSE_REJECT:
        break;
        default:
        break;
    }
// This will cause the dialog to be destroyed and close.
// without this line the dialog remains open unless the
// response_id is GTK_RESPONSE_DELETE_EVENT which
// automatically closes the dialog without the following line.
gtk_widget_destroy(GTK_WIDGET (dialog));
    // For demonstration purposes, this will show the enum name and int value of the button that was pressed
    std::cout << "response is ";
    switch(response_id) {
    case GTK_RESPONSE_ACCEPT:
    std::cout << "GTK_RESPONSE_ACCEPT ";
    break;
    case GTK_RESPONSE_DELETE_EVENT:
    std::cout << "GTK_RESPONSE_DELETE_EVENT (i.e. ?X? button) ";
    break;
    case GTK_RESPONSE_REJECT:
    std::cout << "GTK_RESPONSE_REJECT ";
    break;
    default:
    std::cout << "UNKNOWN ";
    break;
    }
    std::cout << "(" << response_id << ")\n";
    // This will cause the dialog to be destroyed and close.
    // without this line the dialog remains open unless the
    // response_id is GTK_RESPONSE_DELETE_EVENT which
    // automatically closes the dialog without the following line.
    gtk_widget_destroy(GTK_WIDGET (dialog));

}

//This function displays the instructions for the user based on the travel path
void displayInstructions(ezgl::application* app){

    if(enable_navigate){ 
            app->update_message("Found path!");
            
            //Store information about the source, destination, and path
            int intersection1 = source_and_destination.first;
            int intersection2 = source_and_destination.second;
            vector <int> currPath = findPathBetweenIntersections(15, make_pair(intersection1, intersection2));
            cout<<"Directions for " << intersections[intersection1].name<< " to "<< intersections[intersection2].name<< "."<<endl;
            
            //Get the hours, minutes, and seconds for the path travel time
            int totalSeconds = (int)computePathTravelTime(15,currPath);
            int hour = totalSeconds/3600;
            totalSeconds %= 3600;
            int minutes = totalSeconds/60;
            totalSeconds %= 60;
            int seconds = roundSecond(totalSeconds);
            
            //Display travel time to destination
            if(minutes == 0){
                cout<< "You will reach your destination in "<< seconds << " seconds"<<"."<<endl;
            }
            else if(hour == 0){
                cout<< "You will reach your destination in "<< minutes << " minutes " << seconds << " seconds"<<"."<<endl;
            }
            else{
                cout<< "You will reach your destination in "<< hour<< " hours " << minutes << " minutes " << seconds << " seconds"<<"."<<endl;
            }
            
            cout<<"Directions: "<<endl;
            
            //Declarations and definitions of variables used in rest of function
            int currentStreetId = -1;
            double currentDistance = 0;
            int firstStreetId = getStreetSegmentInfo(currPath[0]).streetID;
            string streetName = getStreetName(getStreetSegmentInfo(currPath[0]).streetID);
            StreetSegmentInfo currSegInfo;
            StreetSegmentInfo prevSegInfo;
            string direction;
            string arrow;
            
            //Loop through every segment in the path
            for(int seg = 0; seg < currPath.size()-1;seg++){
                
                currSegInfo = getStreetSegmentInfo(currPath[seg]);
                prevSegInfo = getStreetSegmentInfo(currPath[seg-1]);
                
                //When we are still on the same street
                if (getStreetName(currSegInfo.streetID)==streetName){
                    currentDistance += findStreetSegmentLength(currPath[seg]);
                    
                }
                else {
                    double angle = 0;
                
                    int dir;
                    
                    //Determine the clockwise angle between 2 street segment vectors based on the 4 possible alignments of the vectors
                    if(currSegInfo.streetID!=currentStreetId){
                        currentStreetId = currSegInfo.streetID;
                        if(currSegInfo.from == prevSegInfo.from){\
                            angle = findAngleBetweenTwoSegmentVectors(prevSegInfo.to, currSegInfo.from, currSegInfo.to);
                        }
                        else if(currSegInfo.from == prevSegInfo.to){
                            angle = findAngleBetweenTwoSegmentVectors(prevSegInfo.to, currSegInfo.to, currSegInfo.from);
                        }
                        else if(currSegInfo.to == prevSegInfo.from){
                            angle = findAngleBetweenTwoSegmentVectors(prevSegInfo.from, currSegInfo.from, currSegInfo.to);
                        }
                        else if(currSegInfo.to == prevSegInfo.to){
                            angle = findAngleBetweenTwoSegmentVectors(prevSegInfo.from, currSegInfo.to, currSegInfo.from);
                        }
                        dir =  determineDirectionFromAngle(angle);
                    }

                    if(dir == 1){
                        arrow = " \u2190 "; //left arrow
                    }
                    else if(dir == 2){
                        arrow = " \u2192 "; //right arrow
                    }
                    else if(dir == 3){
                        arrow = " \u2191 "; //striaght arrow
                    }
                    
                    //Display the initial direction for the first street segment
                    if(firstStreetId == prevSegInfo.streetID && instructions_just_once == 0){
                        cout << "Head" << getInitialDirection(intersection1, currPath[0]) << "then ";
                        instructions_just_once = 1;
                    }
                    
                    currentDistance = roundSecond(currentDistance);
                    
                    //Start displaying instructions
                    //Account for when street name is unknown
                    if(getStreetName(prevSegInfo.streetID) == "<unknown>"){
                        
                        //Left
                        if(dir == 1){
                        direction = "left";
                        cout << arrow << "Turn " << direction << " and move for " << currentDistance << " meters." << endl;
                        }
                        
                        //Right
                        else if(dir == 2){
                            direction = "right";
                            cout << arrow << "Turn " << direction << " and move for " << currentDistance << " meters." << endl;
                        }
                        
                        //Straight
                        else if(dir == 3){
                            direction = "straight";
                            cout << arrow << "Head " << direction << " and move for " << currentDistance << " meters." << endl;
                        }
                    }
                    
                    //Left
                    else if(dir == 1){
                        direction = "left";
                        cout << arrow << "Turn " << direction << " on " << getStreetName(prevSegInfo.streetID) << " and move for " << currentDistance << " meters." << endl;
                    }
                    
                    //Right
                    else if(dir == 2){
                        direction = "right";
                        cout << arrow << "Turn " << direction << " on " << getStreetName(prevSegInfo.streetID) << " and move for " << currentDistance << " meters." << endl;
                    }
                    
                    //Straight
                    else if(dir == 3){
                        direction = "straight";
                        cout << arrow << "Head " << direction << " on " << getStreetName(prevSegInfo.streetID) << " and move for " << currentDistance << " meters." << endl;
                    }              
                        streetName = getStreetName(currSegInfo.streetID);
                        currentDistance = findStreetSegmentLength(currPath[seg]);
                } 
            }      
            
            //Display instruction for the last street
            if(getStreetName(prevSegInfo.streetID) != "<unknown>"){
                cout << arrow << "Go " << direction << " on " << getStreetName(prevSegInfo.streetID) << " and move for " << currentDistance << " meters." << endl;
            }
            else {
                cout << arrow << "Go " << direction << " and move for " << currentDistance << " meters." << endl;
            }
            cout << "Arrived at destination!" << endl;   
    }  
}

//This function calculates the clockwise angle between two street segment vectors
double findAngleBetweenTwoSegmentVectors(IntersectionIdx point1, IntersectionIdx point2, IntersectionIdx point3){
    
    double determinant, denominator;
    double x1,x2,y1,y2;
    
    //Calculate the x,y values of the two vectors
    x1 = intersections[point2].xy_loc.x - intersections[point1].xy_loc.x;
    x2 = intersections[point3].xy_loc.x - intersections[point2].xy_loc.x;
    y1 = intersections[point2].xy_loc.y - intersections[point1].xy_loc.y;
    y2 = intersections[point3].xy_loc.y - intersections[point2].xy_loc.y;
    
    //Perform relavant calculations using geometric formula
    determinant = x1*y2-x2*y1;
    denominator = x1*x2+y1*y2;
    double theta = atan2(determinant, denominator);
    
    //Convert from radians to degrees and return
    return theta*57.2958; //MAKE A MAGIC NUMBER!
}

//This function determines the direction to turn in based on the angle between two street segent vectors
int determineDirectionFromAngle(double angle){
    
    if(angle>15 && angle<165){
        return 1;//left
    }
    else if(angle>-165 && angle<-15){
        return 2;//right
    }
    return 3; //straight
}

//This function determines which geographical direction to go to when first leaving the source intersection
string getInitialDirection(IntersectionIdx src, StreetSegmentIdx seg0){
    
    //Get the locations of the source and end point of first segment in the path
    ezgl::point2d first_point = intersections[src].xy_loc;
    ezgl::point2d second_point = intersections[getStreetSegmentInfo(seg0).to].xy_loc;
    if(first_point == second_point){
        second_point = intersections[getStreetSegmentInfo(seg0).from].xy_loc;
    }
    
    //Calculate differences in x and y values
    double delta_x = first_point.x - second_point.x;
    double delta_y = first_point.y - second_point.y;
    
    //When difference in y-coordinates is greater than difference in x coordinates,
    //determine whether to go north or south
    if(abs(delta_y)>abs(delta_x)){
        if(delta_y<0){
            return " north ";
        }
        return " south ";
    }
    
    //When difference in x-coordinates is greater than difference in y coordinates,
    //determine whether to go east or west
    else {
        if(delta_x<0){
            return " east ";
        }
        return " west ";
    }
}

//This function rounds a number to the nearest 10's place
int roundSecond(int sec){
    int small = (sec / 10) * 10; 
    int high = small + 10;
    return (sec - small <= high - sec)? small : high;
}
