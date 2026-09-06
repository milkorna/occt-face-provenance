#include "TrackedShape.h"

#include <TopAbs_ShapeEnum.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>

#include <stdexcept>

TrackedShape::TrackedShape(const TopoDS_Shape& shape)
    : m_shape{shape}
{
    if (m_shape.IsNull())
    {
        throw std::invalid_argument("Tracked shape cannot be null");
    }

    for (TopExp_Explorer explorer{m_shape, TopAbs_FACE}; explorer.More(); explorer.Next())
    {
        m_faceOrigins.Bind(explorer.Current(), WireIdSet{});
    }
}

const TopoDS_Shape& TrackedShape::shape() const noexcept
{
    return m_shape;
}

bool TrackedShape::containsFace(const TopoDS_Face& face) const
{
    if (face.IsNull())
    {
        return false;
    }

    return m_faceOrigins.IsBound(face);
}

const WireIdSet& TrackedShape::faceOrigins(const TopoDS_Face& face) const
{
    const WireIdSet* origins{m_faceOrigins.Seek(face)};

    if (origins == nullptr)
    {
        throw std::invalid_argument("Face does not belong to the tracked shape");
    }

    return *origins;
}

void TrackedShape::addFaceOrigin(const TopoDS_Face& face, const int wireId)
{
    WireIdSet* origins{m_faceOrigins.ChangeSeek(face)};

    if (origins == nullptr)
    {
        throw std::invalid_argument("Face does not belong to the tracked shape");
    }

    origins->insert(wireId);
}

void TrackedShape::addFaceOrigins(const TopoDS_Face& face, const WireIdSet& wireIds)
{
    WireIdSet* origins{m_faceOrigins.ChangeSeek(face)};

    if (origins == nullptr)
    {
        throw std::invalid_argument("Face does not belong to the tracked shape");
    }

    origins->insert(wireIds.begin(), wireIds.end());
}