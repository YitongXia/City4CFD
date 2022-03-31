/*
  Copyright (c) 2021-2022,
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

#ifndef CITY4CFD_EXPLICITBUILDING_H
#define CITY4CFD_EXPLICITBUILDING_H

#include "Building.h"

class ImportedBuilding : public Building {
public:
    ImportedBuilding() = delete;
    ImportedBuilding(nlohmann::json  poly, std::vector<Point_3>& importedBuildingPts, const int internalID);
    ~ImportedBuilding();

    virtual void  reconstruct() override;

    void append_nonground_part(const std::shared_ptr<ImportedBuilding>& other);

    const nlohmann::json& get_building_json() const;
    const std::string&    get_parent_building_id() const;
    const int             get_lod_idx() const;
    const bool            is_appending() const;

//    virtual void  get_cityjson_info(nlohmann::json& b) const override;
//    virtual void  get_cityjson_semantics(nlohmann::json& g) const override;

protected:
    nlohmann::json                   _buildingJson;
    double                           _avgFootprintHeight;
    std::vector<int>                 _footprintIdxList;
    std::vector<std::vector<int>>    _footprintPtsIdxList;
    std::string                      _parentBuildingID;
    bool                             _appendToBuilding;
    int                              _lodIdx;
    std::vector<Point_3>&            _dPts;

    void check_simplicity(Polygon_2& ring);
};

#endif //CITY4CFD_EXPLICITBUILDING_H