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
#include <string>
#include "drawMap.h"
#include "m1.h"
#include "m2.h"

//Program exit codes
constexpr int SUCCESS_EXIT_CODE = 0;        //Everyting went OK
constexpr int ERROR_EXIT_CODE = 1;          //An error occured
constexpr int BAD_ARGUMENTS_EXIT_CODE = 2;  //Invalid command-line usage

//The default map to load if none is specified
std::string default_map_path = "/cad2/ece297s/public/maps/toronto_canada.streets.bin";
std::string maps[] = {"/cad2/ece297s/public/maps/beijing_china.streets.bin",
"/cad2/ece297s/public/maps/cairo_egypt.streets.bin",
"/cad2/ece297s/public/maps/cape-town_south-africa.streets.bin",
"/cad2/ece297s/public/maps/golden-horseshoe_canada.streets.bin",
"/cad2/ece297s/public/maps/hamilton_canada.streets.bin",
"/cad2/ece297s/public/maps/iceland.streets.bin",
"/cad2/ece297s/public/maps/interlaken_switzerland.streets.bin",
"/cad2/ece297s/public/maps/london_england.streets.bin",
"/cad2/ece297s/public/maps/moscow_russia.streets.bin",
"/cad2/ece297s/public/maps/new-delhi_india.streets.bin",
"/cad2/ece297s/public/maps/new-york_usa.streets.bin",
"/cad2/ece297s/public/maps/rio-de-janeiro_brazil.streets.bin",
"/cad2/ece297s/public/maps/saint-helena.streets.bin",
"/cad2/ece297s/public/maps/singapore.streets.bin",
"/cad2/ece297s/public/maps/sydney_australia.streets.bin",
"/cad2/ece297s/public/maps/tehran_iran.streets.bin",
"/cad2/ece297s/public/maps/tokyo_japan.streets.bin",
"/cad2/ece297s/public/maps/toronto_canada.streets.bin"};//an array that contains every possible map path


// The start routine of your program (main) when you are running your standalone
// mapper program. This main routine is *never called* when you are running 
// ece297exercise (the unit tests) -- those tests have their own main routine
// and directly call your functions in /libstreetmap/src/ to test them.
// Don't write any code in this file that you want run by ece297exerise -- it 
// will not be called!
int main(int argc, char** argv) {

    std::string map_path;
    bool load_success = false;
    if(argc == 1) {
        //Use a default map
        map_path = default_map_path;
        load_success = loadMap(map_path);
    } else if (argc == 2) {
        //Get the map from the command line
        map_path = argv[1];
        for(int i = 0; i < 18; i++){
            //check if the provided argument matches a valid map path
            if(map_path == maps[i]){
                load_success = loadMap(map_path);
                break;
            }
            
        }
    } else {
        //Invalid arguments
        std::cout << "Usage: " << argv[0] << " [map_file_path]\n";
        std::cout << "  If no map_file_path is provided a default map is loaded.\n";
    }

    //Load the map and related data structures
    
    if(!load_success) {
        //if no map or invalid file path provided, keep asking the user until a valid map path is provided
        do{
            std::cout<< "Map not found\n";
            std::cout << "Enter map: ";
            std::cin >> map_path;
            for(int i = 0; i < 18; i++){
                //check if the new input matches a valid map path
                if(map_path == maps[i]){
                    load_success = loadMap(map_path);
                    break;
                }
            }
        }while(!load_success);
    }

    std::cout << "Successfully loaded map '" << map_path << "'\n";

    //You can now do something with the map data
    drawMap();
 
    //Clean-up the map data and related data structures
    std::cout << "Closing map\n";
    closeMap(); 

    return SUCCESS_EXIT_CODE;
}
