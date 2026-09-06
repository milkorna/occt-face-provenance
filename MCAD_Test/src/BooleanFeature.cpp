#include "BooleanFeature.h"

#include "TrackedShape.h"

#include <BRepAlgoAPI_BooleanOperation.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <NCollection_List.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>

#include <sstream>
#include <stdexcept>

namespace
{
void propagateFaceOrigins(const TrackedShape& source, BRepAlgoAPI_BooleanOperation& algorithm, TrackedShape& result)
{
    for (TopExp_Explorer explorer{source.shape(), TopAbs_FACE}; explorer.More(); explorer.Next())
    {
        const TopoDS_Face& sourceFace = TopoDS::Face(explorer.Current());
        const WireIdSet& origins = source.faceOrigins(sourceFace);

        if (result.containsFace(sourceFace))
        {
            result.addFaceOrigins(sourceFace, origins);
        }

        const auto& modifiedShapes = algorithm.Modified(sourceFace);
        for (const TopoDS_Shape& modifiedShape : modifiedShapes)
        {
            if (modifiedShape.ShapeType() == TopAbs_FACE)
            {
                const TopoDS_Face& modifiedFace = TopoDS::Face(modifiedShape);

                if (!result.containsFace(modifiedFace))
                {
                    throw std::runtime_error("Modified face is not present in the Boolean result");
                }

                result.addFaceOrigins(modifiedFace, origins);
            }
        }
    }
}

template <typename BooleanAlgorithm>
TrackedShape buildBoolean(const TrackedShape& argument, const TrackedShape& tool)
{
    NCollection_List<TopoDS_Shape> arguments{};
    arguments.Append(argument.shape());

    NCollection_List<TopoDS_Shape> tools{};
    tools.Append(tool.shape());

    BooleanAlgorithm algorithm{};
    algorithm.SetArguments(arguments);
    algorithm.SetTools(tools);
    algorithm.SetNonDestructive(true);
    algorithm.SetToFillHistory(true);
    algorithm.Build();

    if (!algorithm.IsDone() || algorithm.HasErrors())
    {
        std::ostringstream errors{};
        algorithm.DumpErrors(errors);

        throw std::runtime_error("Failed to perform Boolean operation:\n" + errors.str());
    }

    const TopoDS_Shape shape{algorithm.Shape()};
    if (shape.IsNull())
    {
        throw std::runtime_error("Boolean operation completed without producing a result");
    }

    BRepCheck_Analyzer analyzer{shape};
    if (!analyzer.IsValid())
    {
        throw std::runtime_error("Boolean result contains invalid topology");
    }

    TrackedShape result{shape};

    propagateFaceOrigins(argument, algorithm, result);
    propagateFaceOrigins(tool, algorithm, result);

    return result;
}

TrackedShape buildBoolean(const BooleanType type, const TrackedShape& argument, const TrackedShape& tool)
{
    switch (type)
    {
    case BooleanType::Union:
        return buildBoolean<BRepAlgoAPI_Cut>(argument, tool);

    case BooleanType::Subtract:
        return buildBoolean<BRepAlgoAPI_Fuse>(argument, tool);
    }

    throw std::invalid_argument("Unsupported Boolean type");
}
} // namespace

BooleanFeature::BooleanFeature(const int id, const BooleanType type, const TrackedShape& argument,
                               const TrackedShape& tool)
    : m_id{id},
      m_type{type},
      m_result{buildBoolean(type, argument, tool)}
{
}

int BooleanFeature::id() const noexcept
{
    return m_id;
}

BooleanType BooleanFeature::type() const noexcept
{
    return m_type;
}

const TrackedShape& BooleanFeature::result() const noexcept
{
    return m_result;
}