#include "Sketch.h"
#include "WireInfo.h"

#include <BRepBuilderAPI_MakePolygon.hxx>
#include <ElSLib.hxx>
#include <TopoDS_Wire.hxx>
#include <gp_Ax3.hxx>
#include <gp_Pln.hxx>
#include <gp_Pnt.hxx>
#include <gp_Pnt2d.hxx>
#include <gp_Vec.hxx>

#include <stdexcept>
#include <vector>


Sketch::Sketch(const int id, const gp_Pln& plane) : m_id(id), m_plane(plane)
{
}

int Sketch::id() const
{
	return m_id;
}

const gp_Pln& Sketch::plane() const
{
	return m_plane;
}

const std::vector<WireInfo>& Sketch::wires() const
{
	return m_wires;
}

void Sketch::addClosedWire(const int wireId, const std::vector<gp_Pnt2d>& points)
{
	const TopoDS_Wire wire = makeClosedWire(points);
	m_wires.push_back({ wireId, wire });
}

TopoDS_Wire Sketch::makeClosedWire(const std::vector<gp_Pnt2d>& points) const
{
	if (points.size() < 3)
	{
		throw std::invalid_argument("Closed contour requires at least three points");
	}

	BRepBuilderAPI_MakePolygon wireMaker;

	for (size_t i = 0; i < points.size(); ++i)
	{
		const gp_Pnt point = ElSLib::Value(points[i].X(), points[i].Y(), m_plane);
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