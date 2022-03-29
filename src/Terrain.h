/*
  Copyright (c) 2020-2021,
  Ivan Pađen <i.paden@tudelft.nl>
  3D Geoinformation,
  Delft University of Technology

  This file is part of City4CFD.

  City4CFD is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  City4CFD is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  You should have received a copy of the GNU General Public License
  along with this program. If not, see <http://www.gnu.org/licenses/>
*/

#ifndef CITY4CFD_TERRAIN_H
#define CITY4CFD_TERRAIN_H

#include "TopoFeature.h"

class Terrain : public TopoFeature {
public:
    using TopoFeature::TopoFeature;
    Terrain();
    Terrain(int pid);
    ~Terrain();

    void set_cdt(const Point_set_3 &pointCloud);
    void prep_constraints(const PolyFeatures& features, Point_set_3& pointCloud, const bool costrainBuildings);
    void constrain_features();
    void create_mesh(const PolyFeatures& features);

    CDT&         get_cdt();
    const CDT&   get_cdt() const;

    void         get_cityjson_info(nlohmann::json& b) const override;
    std::string  get_cityjson_primitive() const override;
    TopoClass    get_class() const override;
    std::string  get_class_name() const override;

    const SurfaceLayers& get_surface_layers() const;

protected:
    CDT                    _cdt;
    SurfaceLayers          _surfaceLayersTerrain;
    std::list<Polygon_3>   _constrainedPolys;
};

#endif //CITY4CFD_TERRAIN_H