#include "io.h"

#include "config.h"
#include "TopoFeature.h"
#include "Boundary.h"

//-- Input functions
void IO::read_config(std::string& config_path) {
    std::ifstream json_file(config_path);
    if (!json_file)
        throw std::invalid_argument(std::string("Configuration file " + config_path + " not found."));

    //-- Filepaths in the json file are relative to the location of the json file
    config::workDir = fs::path(config_path).parent_path();
    fs::current_path(config::workDir);
    std::cout << "Work directory path: " << fs::canonical(config::workDir) << std::endl;

    nlohmann::json j;
    try {
        j = nlohmann::json::parse(json_file, nullptr, true, true);
    } catch (std::exception& e) {
        std::cerr << "ERROR: Configuration file is not a valid JSON file. "
                  << "Check if you're missing a comma or a brace." << std::endl;
        throw;
    }

    std::cout << "\nValidating JSON configuration file...";
    config::validate(j);
    std::cout <<"Configuration file is valid! \n" << std::endl;

    config::set_config(j);
}

bool IO::read_point_cloud(std::string& file, Point_set_3& pc) {
    //todo test CGAL 5.3 if it can read .las without external reader
    std::ifstream ifile(file, std::ios_base::binary);
    ifile >> pc;
    return true;
}

void IO::read_geojson_polygons(std::string& file, JsonVector& jsonPolygons) { // For now specifically GeoJSON, but can potentially change
    try {
        std::ifstream ifs(file);
        nlohmann::json j = nlohmann::json::parse(ifs);

        int count;
        for (auto& feature : j["features"]) {
            if (feature["geometry"]["type"] == "Polygon") {
                auto poly = feature["geometry"]["coordinates"];
                jsonPolygons.emplace_back(std::make_unique<nlohmann::json>(poly));
            } else if (feature["geometry"]["type"] == "MultiPolygon") {
                for (auto& poly : feature["geometry"]["coordinates"]) {
                    jsonPolygons.emplace_back(std::make_unique<nlohmann::json>(poly));
                }
            } else {
                // Exception handling - maybe write to log file
//                std::cout << "In file '" << file << "' cannot parse geometry type "
//                          << feature["geometry"]["type"] << ". Object ID: " << count << std::endl;
            }
            ++count;
        }
    } catch (std::exception& e) {
        throw std::runtime_error(std::string("Error parsing JSON file '" + file + "'. Details: " + e.what()));
    }
}

void IO::read_explicit_geometries(std::string& file, JsonVector& importedBuildings,
                                  std::vector<Point_3>& importedBuildingPts) {
    try {
        std::ifstream ifs(file);
        nlohmann::json j = nlohmann::json::parse(ifs);

        //-- Add vertices
        for (auto& pt : j["vertices"]) {
            double ptx = ((double)pt[0] * (double)j["transform"]["scale"][0]) + (double)j["transform"]["translate"][0];
            double pty = ((double)pt[1] * (double)j["transform"]["scale"][1]) + (double)j["transform"]["translate"][1];
            double ptz = ((double)pt[2] * (double)j["transform"]["scale"][2]) + (double)j["transform"]["translate"][2];
            importedBuildingPts.emplace_back(ptx, pty, ptz);
        }

        //-- Separate individual buildings
        for (auto& cityObj : j["CityObjects"]) {
            if (cityObj["type"] == "BuildingPart") {
                importedBuildings.push_back(std::make_unique<nlohmann::json>(cityObj));
            }
        }
    } catch (std::exception& e) {
        throw std::runtime_error(std::string("Error parsing JSON file '" + file + "'. Details: " + e.what()));
    }
}

//-- Output functions
void IO::print_progress_bar(int percent) {
    std::string bar;
    for (int i = 0; i < 50; i++) {
        if (i < (percent / 2)) {
            bar.replace(i, 1, "=");
        }
        else if (i == (percent / 2)) {
            bar.replace(i, 1, ">");
        }
        else {
            bar.replace(i, 1, " ");
        }
    }
    std::clog << "\r" "    [" << bar << "] ";
    std::clog.width(3);
    std::clog << percent << "%     " << std::flush;
}

void IO::output_obj(const OutputFeatures& allFeatures) {
    int numOutputSurfaces = TopoFeature::get_num_output_layers();
    std::vector<std::ofstream> of;
    std::vector<std::string>   fs(numOutputSurfaces), bs(numOutputSurfaces);

    std::vector<std::unordered_map<std::string, int>> dPts(numOutputSurfaces);
    //-- Output points
    for (auto& f : allFeatures) {
        if (f->get_class() == BUILDING)
            bs[f->get_output_layer_id()] += "\no " + std::to_string(f->get_internal_id());
        if (config::outputSeparately)
            IO::get_obj_pts(f->get_mesh(),
                            fs[f->get_output_layer_id()],
                            bs[f->get_output_layer_id()],
                            dPts[f->get_output_layer_id()]);
        else
            IO::get_obj_pts(f->get_mesh(),
                            fs[f->get_output_layer_id()],
                            bs[f->get_output_layer_id()],
                            dPts.front());
    }

    //-- Add class name and output to file
    if (!config::outputSeparately) {
        of.emplace_back();
        of.back().open(config::outputFileName + ".obj");
    }
    for (int i = 0; i < fs.size(); ++i) {
        if (bs[i].empty()) continue;
        if (config::outputSeparately) {
            of.emplace_back();
            of.back().open(config::outputFileName + "_" + config::outputSurfaces[i] + ".obj");
        }

        of.back() << fs[i] << "\ng " << config::outputSurfaces[i] << bs[i];
    }
    for (auto& f : of) f.close();
}

void IO::output_stl(const OutputFeatures& allFeatures) {
    int numOutputLayers = TopoFeature::get_num_output_layers();
    std::vector<std::ofstream> of;
    std::vector<std::string>   fs(numOutputLayers);

    //-- Get all triangles
    for (auto& f : allFeatures) {
        if (!f->is_active()) continue;
        IO::get_stl_pts(f->get_mesh(), fs[f->get_output_layer_id()]);
    }

    //-- Add class name and output to file
    if (!config::outputSeparately) {
        of.emplace_back();
        of.back().open(config::outputFileName + ".stl");
    }
    for (int i = 0; i < fs.size(); ++i) {
        if (fs[i].empty()) continue;
        if (config::outputSeparately) {
            of.emplace_back();
            of.back().open(config::outputFileName + "_" + config::outputSurfaces[i] + ".stl");
        }

        of.back() << "\nsolid " << config::outputSurfaces[i];
        of.back() << fs[i];
        of.back() << "\nendsolid " << config::outputSurfaces[i];
    }
    for (auto& f : of) f.close();
}

void IO::output_cityjson(const OutputFeatures& allFeatures) {
    using namespace config;
    std::ofstream of;
    nlohmann::json j;

    j["type"] = "CityJSON";
    j["version"] = "1.0";
    j["metadata"] = {};
    std::vector<double> bbox = Boundary::get_domain_bbox();
    j["metadata"]["geographicalExtent"] = Boundary::get_domain_bbox();
    j["metadata"]["referenceSystem"] = "urn:ogc:def:crs:EPSG::7415";
    std::unordered_map<std::string, int> dPts;
    for (auto& f : allFeatures) {
        // Only Buildings and Terrain for now
        if (f->get_class() != BUILDING && f->get_class() != TERRAIN) continue;
        //-- Get feature info
        nlohmann::json b;
        f->get_cityjson_info(b);

        //-- Get feature geometry
        nlohmann::json g;
        IO::get_cityjson_geom(f->get_mesh(), g, dPts, f->get_cityjson_primitive());

        //-- Get feature semantics
        f->get_cityjson_semantics(g);

        //-- Append to main json struct
        b["geometry"].push_back(g);
        j["CityObjects"][f->get_id()] = b;
    }

    //-- Vertices - store them in a vector to quickly sort
    std::vector<std::string> thepts;
    thepts.resize(dPts.size());
    for (auto& p : dPts)
        thepts[p.second] = p.first;
    dPts.clear();
    for (auto& p : thepts) {
        std::vector<std::string> c;
        boost::split(c, p, boost::is_any_of(" "));
        j["vertices"].push_back({std::stod(c[0], NULL), std::stod(c[1], NULL), std::stod(c[2], NULL) });
    }

    of.open(outputFileName + ".json");
    of << j.dump() << std::endl;
}

void IO::get_obj_pts(const Mesh& mesh,
                     std::string& fs,
                     std::string& bs,
                     std::unordered_map<std::string, int>& dPts)
{
    for (auto& face : mesh.faces()) {
        std::vector<int> faceIdx; faceIdx.reserve(3);
        std::string fsTemp;
        std::string bsTemp;
        for (auto index : CGAL::vertices_around_face(mesh.halfedge(face), mesh)) {
            std::string pt = gen_key_bucket(mesh.point(index));
            auto it = dPts.find(pt);
            if (it == dPts.end()) {
                fs += "\nv " + pt;
                bsTemp += " " + std::to_string(dPts.size() + 1);

                faceIdx.push_back(dPts.size() + 1);

                dPts[pt] = dPts.size() + 1;
            } else {
                bsTemp += " " + std::to_string(it->second);

                faceIdx.push_back(it->second);
            }
        }

        if (IO::not_small(faceIdx)) {
            bs += "\nf";
            bs += bsTemp;
//        } else {
//            std::cerr << "Found duplicates!" << std::endl;
        }
    }
}

void IO::get_stl_pts(Mesh& mesh, std::string& fs) {
    auto vnormals = mesh.add_property_map<vertex_descriptor, Vector_3>("v:normals", CGAL::NULL_VECTOR).first;
    auto fnormals = mesh.add_property_map<face_descriptor, Vector_3>("f:normals", CGAL::NULL_VECTOR).first;
    PMP::compute_normals(mesh, vnormals, fnormals);
    for (auto& face : mesh.faces()) {
        std::vector<std::string> outputPts;
        for (auto index: CGAL::vertices_around_face(mesh.halfedge(face), mesh)) {
            outputPts.push_back(gen_key_bucket(mesh.point(index)));
        }
        //-- Check for round off of small triangles
        if (outputPts[0] == outputPts[1]
         || outputPts[0] == outputPts[2]
         || outputPts[1] == outputPts[2]) continue;

        fs += "\nfacet normal " + gen_key_bucket(fnormals[face]);
        fs += "\n    outer loop";
        for (auto pt: outputPts) {
            fs += "\n        vertex " + pt;
        }
        fs += "\n    endloop";
        fs += "\nendfacet";
    }
}

void IO::get_cityjson_geom(const Mesh& mesh, nlohmann::json& g, std::unordered_map<std::string, int>& dPts,
                           std::string primitive) {
    g["type"] = primitive;
    g["lod"] = config::lod;
    g["boundaries"];
    for (auto& face: mesh.faces()) {
        std::vector<int> faceIdx;
        faceIdx.reserve(3);
        std::vector<int> tempPoly;
        tempPoly.reserve(3);
        for (auto index: CGAL::vertices_around_face(mesh.halfedge(face), mesh)) {
            std::string pt = gen_key_bucket(mesh.point(index));
            auto it = dPts.find(pt);
            if (it == dPts.end()) {
                faceIdx.push_back(dPts.size());

                tempPoly.push_back(dPts.size());

                dPts[pt] = dPts.size();
            } else {
                faceIdx.push_back(it->second);
                tempPoly.push_back(it->second);
            }
        }

        if (IO::not_small(faceIdx)) {
            g["boundaries"].push_back({tempPoly});
//        } else {
//            std::cerr << "Found duplicates!" << std::endl;
        }
    }
}

//-- Check for round off of small triangles
bool IO::not_small(std::vector<int> idxLst) {
    std::sort(idxLst.begin(), idxLst.end());
    auto it = std::unique(idxLst.begin(), idxLst.end());

    return (it == idxLst.end());
}

void IO::output_log() {
    if (!config::outputLog) return;

    //-- Output log file
    config::logSummary <<"\n// ----------------------------------------------------------------------------------------------- //" << std::endl;
    std::cout << "\nCreating log file '" << config::logName << "'" << std::endl;
    std::ofstream of;
    of.open(config::logName);
    of << config::log.str() << config::logSummary.str();
    of.close();

    //-- Output failed reconstructions
    if (!config::failedBuildings.empty()) {
        std::cout << "Outputting failed building reconstructions to 'failedReconstructions.geojson'"
                  << std::endl;
        //- Parse again the buildings polygon
        std::ifstream ifs(config::workDir.append(config::gisdata).string());
        nlohmann::json j = nlohmann::json::parse(ifs);
        //- Extract failed reconstructions
        nlohmann::json b;
        for (int i: config::failedBuildings) {
            b["features"].push_back(j["features"][i]);
        }
        b["crs"] = j["crs"];
        b["name"] = "failedBuildings";
        b["type"] = j["type"];
        //- Write to file
        of.open("failedReconstructions.geojson");
        of << b.dump();
        of.close();
    }
}

std::string IO::gen_key_bucket(const Point_2 p) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(3) << p.x() << " " << p.y();
    return ss.str();
}

//-- Templated functions
template<typename T>
std::string IO::gen_key_bucket(const T& p) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(3) << p.x() << " " << p.y() << " " << p.z();
    return ss.str();
}
//- Explicit template instantiation
template std::string IO::gen_key_bucket<Point_3>(const Point_3& p);
template std::string IO::gen_key_bucket<Vector_3>(const Vector_3& p);