#pragma once

#include <NCollection_DataMap.hxx>
#include <TopTools_ShapeMapHasher.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>

#include <set>

using WireIdSet = std::set<int>;

class TrackedShape
{
public:
	explicit TrackedShape(const TopoDS_Shape& shape);

	const TopoDS_Shape& shape() const noexcept;

	bool containsFace(const TopoDS_Face& face) const;
	const WireIdSet& faceOrigins(const TopoDS_Face& face) const;

	void addFaceOrigin(const TopoDS_Face& face, const int wireId);
	void addFaceOrigins(const TopoDS_Face& face, const WireIdSet& wireIds);

private:
	using FaceOriginMap = NCollection_DataMap<TopoDS_Shape, WireIdSet, TopTools_ShapeMapHasher>;

	TopoDS_Shape m_shape;
	FaceOriginMap m_faceOrigins;
};