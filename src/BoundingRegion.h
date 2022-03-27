#ifndef CITY4CFD_BOUNDINGREGION_H
#define CITY4CFD_BOUNDINGREGION_H

#include "types.h"
#include "CGALTypes.h"

class BoundingRegion {
public:
    BoundingRegion();
    ~BoundingRegion();

    void operator()(double radius);
    void operator()(Polygon_2& poly);

    void calc_influ_region_bpg(const DT& dt, const Point_set_3& pointCloudBuildings, Buildings& buildings);
    void calc_bnd_bpg(const Polygon_2& influRegionPoly, const Buildings& buildings);

    Polygon_2& get_bounding_region();
    const Polygon_2& get_bounding_region() const;

protected:
    Polygon_2  _boundingRegion;

    Polygon_2  calc_bnd_poly(const std::vector<Point_2>& candidatePts, const double hMax,
                             const double angle, const double enlargeRatio = 1) const;
    double     calc_blockage_ratio(const Buildings& buildings, const double angle, Polygon_2& localPoly) const;

};

#endif //CITY4CFD_BOUNDINGREGION_H