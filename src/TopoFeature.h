#ifndef CITYCFD_TOPOFEATURE_H
#define CITYCFD_TOPOFEATURE_H

#include "definitions.h"
#include "config.h"
#include "geomtools.h"

class TopoFeature {
public:
    TopoFeature();
    TopoFeature(const int pid);
    ~TopoFeature();

    virtual TopoClass    get_class() const = 0;
    virtual std::string  get_class_name() const = 0;
    virtual void         get_cityjson_info(nlohmann::json& b) const;
    virtual void         get_cityjson_semantics(nlohmann::json& g) const;
    virtual std::string  get_cityjson_primitive() const;

    Mesh&       get_mesh();
    const Mesh& get_mesh() const;
    void        set_id(unsigned long id);
    std::string get_id() const;
    bool        is_active() const;
    void        deactivate();

protected:
    Mesh           _mesh;
    std::string    _id;
    bool           _f_active;
};

//-- TopoFeature derived from polygons
class PolyFeature : public TopoFeature {
public:
    using TopoFeature::TopoFeature;
    PolyFeature();
    PolyFeature(const nlohmann::json& poly);
    ~PolyFeature();

    virtual void        check_feature_scope();
    virtual void        calc_footprint_elevation(const SearchTree& searchTree);
    virtual void        threeDfy(const SearchTree& searchTree);
    virtual void        get_cityjson_info(nlohmann::json& b) const = 0;
    virtual void        get_cityjson_semantics(nlohmann::json& g) const = 0;
    virtual std::string get_cityjson_primitive() const = 0;

    virtual TopoClass   get_class() const = 0;
    virtual std::string get_class_name() const = 0;

    const Polygon_with_holes_2& get_poly() const;
    const std::vector<double>&  get_base_heights() const;

protected:
    Polygon_with_holes_2               _poly;
    std::vector<double>                _base_heights;

};

#endif //CITYCFD_TOPOFEATURE_H