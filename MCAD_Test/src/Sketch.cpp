#include "Sketch.h"

#include "IdGenerator.h"
#include "WireInfo.h"

#include <BRepBuilderAPI_MakePolygon.hxx>
#include <ElSLib.hxx>
#include <TopoDS_Wire.hxx>
#include <gp_Pln.hxx>
#include <gp_Pnt.hxx>
#include <gp_Pnt2d.hxx>

#include <stdexcept>
#include <vector>

Sketch::Sketch(const gp_Pln& plane)
    : m_id{IdGenerator::nextSketchId()},
      m_plane{plane}
{
}

int Sketch::id() const noexcept
{
    return m_id;
}

const gp_Pln& Sketch::plane() const noexcept
{
    return m_plane;
}

const std::vector<WireInfo>& Sketch::wires() const noexcept
{
    return m_wires;
}

int Sketch::addClosedWire(const std::vector<gp_Pnt2d>& points)
{
    const int wireId{IdGenerator::nextWireId()};
    const TopoDS_Wire wire{makeClosedWire(points)};

    m_wires.push_back(WireInfo{wireId, wire});

    return wireId;
}

TopoDS_Wire Sketch::makeClosedWire(const std::vector<gp_Pnt2d>& points) const
{
    if (points.size() < 3)
    {
        throw std::invalid_argument("Closed contour requires at least three points");
    }

    BRepBuilderAPI_MakePolygon wireMaker{};

    for (size_t i = 0; i < points.size(); ++i)
    {
        const gp_Pnt point{ElSLib::Value(points[i].X(), points[i].Y(), m_plane)};
        wireMaker.Add(point);

        if (i > 0 && !wireMaker.Added())
        {
            throw std::invalid_argument("Contour contains two consecutive coincident points");
        }
    }

    wireMaker.Close();

    if (!wireMaker.IsDone())
    {
        throw std::runtime_error("Failed to build closed polygonal wire");
    }

    return wireMaker.Wire();
}