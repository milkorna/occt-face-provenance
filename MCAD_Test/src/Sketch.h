#pragma once

#include "WireInfo.h"

#include <TopoDS_Wire.hxx>
#include <gp_Pln.hxx>
#include <gp_Pnt2d.hxx>

#include <vector>

/**
 * @brief Planar sketch containing closed wires
 */
class Sketch
{
  public:
    /**
     * @brief Constructs sketch on plane
     * @param plane Sketch plane
     */
    explicit Sketch(const gp_Pln& plane);

    /**
     * @brief Returns the sketch plane
     * @return Sketch plane
     */
    const gp_Pln& plane() const noexcept;

    /**
     * @brief Returns wire by its identifier
     * @param wireId Wire identifier
     * @return Requested wire
     */
    const TopoDS_Wire& wire(const int wireId) const;

    /**
     * @brief Adds closed polygonal wire defined in sketch coordinates
     * @param points Wire vertices in local sketch coordinates
     * @return Wire identifier
     */
    int addClosedWire(const std::vector<gp_Pnt2d>& points);

  private:
    /**
     * @brief Builds closed polygonal wire from local sketch coordinates
     * @param points Wire vertices in local sketch coordinates
     * @return Constructed wire
     */
    TopoDS_Wire makeClosedWire(const std::vector<gp_Pnt2d>& points) const;

    int m_id;
    gp_Pln m_plane;
    std::vector<WireInfo> m_wires;
};