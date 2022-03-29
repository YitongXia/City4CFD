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

#ifndef CITY4CFD_TOPOFEATURE_H
#define CITY4CFD_TOPOFEATURE_H

#include "types.h"
#include "CGALTypes.h"

class TopoFeature {
public:
    TopoFeature();
    TopoFeature(std::string pid);
    TopoFeature(int outputLayerID);
    virtual ~TopoFeature();

    static int           get_num_output_layers();

    virtual const int    get_internal_id() const;
    virtual void         get_cityjson_info(nlohmann::json& b) const;
    virtual void         get_cityjson_semantics(nlohmann::json& g) const;
    virtual std::string  get_cityjson_primitive() const;
    virtual TopoClass    get_class() const = 0;
    virtual std::string  get_class_name() const = 0;

    Mesh&       get_mesh();
    const Mesh& get_mesh() const;
    void        set_id(unsigned long id);
    std::string get_id() const;
    const int   get_output_layer_id() const;
    bool        is_active() const;
    bool        is_imported() const;
    void        deactivate();

protected:
    static int _numOfOutputLayers;

    Mesh           _mesh;
    std::string    _id;
    bool           _f_active;
    bool           _f_imported;
    int            _outputLayerID; // 0- Terrain
                                   // 1- Buildings
                                   //    Surface Layers
                                   //    Sides
                                   //    Top
};

#endif //CITY4CFD_TOPOFEATURE_H