#pragma once

#include "WireInfo.h"

#include <TopoDS_Wire.hxx>
#include <gp_Pln.hxx>
#include <gp_Pnt2d.hxx>

#include <vector>

class Sketch
{
  public:
    Sketch(const int id, const gp_Pln& plane);

    int id() const noexcept;
    const gp_Pln& plane() const noexcept;
    const std::vector<WireInfo>& wires() const noexcept;

    void addClosedWire(const int wireId, const std::vector<gp_Pnt2d>& points);

  private:
    TopoDS_Wire makeClosedWire(const std::vector<gp_Pnt2d>& points) const;

    int m_id;
    gp_Pln m_plane;
    std::vector<WireInfo> m_wires;
};