#pragma once

#include <NCollection_DataMap.hxx>
#include <TopTools_ShapeMapHasher.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>

#include <set>

using WireIdSet = std::set<int>;

/**
 * @brief Shape with face-to-wire origin tracking
 */
class TrackedShape
{
  public:
    /**
     * @brief Constructs tracked shape
     * @param shape Shape
     */
    explicit TrackedShape(const TopoDS_Shape& shape);

    /**
     * @brief Returns tracked shape
     * @return Tracked shape
     */
    const TopoDS_Shape& shape() const noexcept;

    /**
     * @brief Checks whether face belongs to tracked shape
     * @param face Face to check
     * @return True if face belongs to tracked shape
     */
    bool containsFace(const TopoDS_Face& face) const;

    /**
     * @brief Returns origin wire identifiers for face
     * @param face Tracked face
     * @return Origin wire identifiers
     */
    const WireIdSet& faceOrigins(const TopoDS_Face& face) const;

    /**
     * @brief Adds origin wire identifier for face
     * @param face Tracked face
     * @param wireId Origin wire identifier
     */
    void addFaceOrigin(const TopoDS_Face& face, const int wireId);

    /**
     * @brief Adds origin wire identifiers for face
     * @param face Tracked face
     * @param wireIds Origin wire identifiers
     */
    void addFaceOrigins(const TopoDS_Face& face, const WireIdSet& wireIds);

  private:
    using FaceOriginMap = NCollection_DataMap<TopoDS_Shape, WireIdSet, TopTools_ShapeMapHasher>;

    TopoDS_Shape m_shape;
    FaceOriginMap m_faceOrigins;
};