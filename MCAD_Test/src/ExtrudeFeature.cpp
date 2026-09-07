#include "ExtrudeFeature.h"

#include "IdGenerator.h"
#include "Sketch.h"
#include "TrackedShape.h"

#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <Precision.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Wire.hxx>
#include <gp_Ax1.hxx>
#include <gp_Dir.hxx>
#include <gp_Pln.hxx>
#include <gp_Vec.hxx>

#include <cmath>
#include <stdexcept>

namespace
{
TopoDS_Face makeProfileFace(const Sketch& sketch, const TopoDS_Wire& wire)
{
    BRepBuilderAPI_MakeFace faceMaker{sketch.plane(), wire, true};

    if (!faceMaker.IsDone())
    {
        throw std::runtime_error("Failed to build profile face from wire");
    }

    const TopoDS_Face& face{faceMaker.Face()};
    BRepCheck_Analyzer analyzer{face};

    if (!analyzer.IsValid())
    {
        throw std::runtime_error("Profile face contains invalid topology");
    }

    return face;
}

gp_Vec makeExtrusionVector(const Sketch& sketch, const double distance)
{
    if (std::fabs(distance) <= Precision::Confusion())
    {
        throw std::invalid_argument("Extrusion distance is too small");
    }

    const gp_Ax1& axis{sketch.plane().Axis()};
    const gp_Dir& direction{axis.Direction()};

    gp_Vec extrusionVector{direction};
    extrusionVector.Multiply(distance);

    return extrusionVector;
}

void addFaceOrigin(TrackedShape& trackedShape, const TopoDS_Shape& shape, const int wireId)
{
    if (shape.ShapeType() != TopAbs_FACE)
    {
        throw std::runtime_error("Expected a face while assigning extrusion origin");
    }

    trackedShape.addFaceOrigin(TopoDS::Face(shape), wireId);
}

TrackedShape buildExtrusion(const Sketch& sketch, const int wireId, const double distance)
{
    const TopoDS_Wire& wire{sketch.wire(wireId)};
    const TopoDS_Face profileFace{makeProfileFace(sketch, wire)};
    const gp_Vec extrusionVector{makeExtrusionVector(sketch, distance)};

    BRepPrimAPI_MakePrism extruder{profileFace, extrusionVector};

    if (!extruder.IsDone())
    {
        throw std::runtime_error("Failed to build extrusion");
    }

    const TopoDS_Shape shape{extruder.Shape()};

    if (shape.IsNull())
    {
        throw std::runtime_error("Extrusion completed without producing result");
    }

    if (shape.ShapeType() != TopAbs_SOLID)
    {
        throw std::runtime_error("Extrusion did not produce solid");
    }

    TrackedShape result{shape};

    addFaceOrigin(result, extruder.FirstShape(), wireId);
    addFaceOrigin(result, extruder.LastShape(), wireId);

    for (TopExp_Explorer explorer{wire, TopAbs_EDGE}; explorer.More(); explorer.Next())
    {
        const TopoDS_Edge& edge{TopoDS::Edge(explorer.Current())};
        const auto& generatedShapes = extruder.Generated(edge);

        for (const TopoDS_Shape& generatedShape : generatedShapes)
        {
            addFaceOrigin(result, generatedShape, wireId);
        }
    }

    for (TopExp_Explorer explorer{shape, TopAbs_FACE}; explorer.More(); explorer.Next())
    {
        const TopoDS_Face& face{TopoDS::Face(explorer.Current())};

        if (result.faceOrigins(face).empty())
        {
            throw std::runtime_error("Failed to determine origin wire for extruded face");
        }
    }

    return result;
}
} // namespace

ExtrudeFeature::ExtrudeFeature(const Sketch& sketch, const int wireId, const double distance)
    : m_result{buildExtrusion(sketch, wireId, distance)}
{
}

const TrackedShape& ExtrudeFeature::result() const noexcept
{
    return m_result;
}