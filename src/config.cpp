#include "config.h"

//-- Config values are hardcoded until I add config file reading
namespace config {
    //-- Input info
    std::string              points_xyz;      // Ground
    std::string              buildings_xyz;   // Buildings
    std::string              gisdata;         // Building Polygons
    std::vector<std::string> topoLayers = {}; // Other polygons

    //-- Domain dimensions
    Point_2     pointOfInterest;
    double      dimOfDomain;
    double      topHeight;
    boost::variant<bool, double, std::string, Polygon_2> influenceRegion;

    //-- Reconstruction related
    double lod;
    double buildingPercentile;

    //-- Polygons related
    double edgeMaxLen;

    //-- Output
    fs::path    outputDir = fs::current_path();
    std::string outputFileName;
    OutputFormat outputFormat;
    bool outputSeparately = false;
}

void config::set_config(nlohmann::json& j) {
    //-- Path to point cloud(s)
    points_xyz    = j["point_clouds"]["ground"];
    buildings_xyz = j["point_clouds"]["buildings"];

    //-- Path to polygons
    bool foundBuildingPoly = false;
    for (auto& poly : j["polygons"]) {
        if (poly["type"] == "Building") {
            gisdata = poly["path"];
            foundBuildingPoly = true;
        }
        if (poly["type"] == "SurfaceLayer") {
            topoLayers.push_back(poly["path"]);
        }
    }
    if (!foundBuildingPoly) throw std::invalid_argument("Didn't find a path to building polygons in configuration file!");

    //-- Domain dimensions
    pointOfInterest = Point_2(j["point_of_interest"][0], j["point_of_interest"][1]);

    //- Influence region which can be defined differently
    if (j["influence_region"].is_string()) { //- Search for GeoJSON polygon
        influenceRegion = (std::string)j["influence_region"];
    } else if (j["influence_region"].size() > 2) { // Explicitly defined influ region polygon
        Polygon_2 tempPoly;
        for (auto& pt : j["influence_region"]) tempPoly.push_back(Point_2(pt[0], pt[1]));
        influenceRegion = tempPoly;
    } else if (j["influence_region"].size() == 2) {
        throw std::invalid_argument("Unknown setup of the influence region!");
    } else if (j["influence_region"].is_number() || j["influence_region"].is_array() && j["influence_region"][0].is_number()) { // Influ region radius
        influenceRegion = (double)j["influence_region"].front();
    } else influenceRegion = true; // Leave it to BPG

    //todo different ways of modelling domain, including height
    dimOfDomain = j["dim_of_domain"].front();
    topHeight = j["top_height"].front();

    //-- Reconstruction related
    lod = j["lod"].front();
    buildingPercentile = (double)j["building_percentile"].front() / 100.;

    //-- Polygons related
    edgeMaxLen = j["edge_max_len"].front();

    //-- Output
    //- File name
    if (outputFileName.empty() && j["output_file_name"].is_string())
        outputFileName = j["output_file_name"];
    else
        throw std::invalid_argument("Invalid output file name!");

    //- Output format
    std::string outputFormatConfig = j["output_format"];
    if (boost::iequals(outputFormatConfig, "obj")) {
        outputFormat = OBJ;
    } else if (boost::iequals(outputFormatConfig, "stl")) {
        outputFormat = STL;
    } else if (boost::iequals(outputFormatConfig, "cityjson")) {
        outputFormat = CityJSON;
    } else throw std::invalid_argument("Unsupported file format!");

    outputSeparately = j["output_separately"];
}