#pragma once

#include "TrackedShape.h"

class Sketch;

class ExtrudeFeature
{
  public:
    ExtrudeFeature(const Sketch& sketch, const int wireId, const double distance);

    int id() const noexcept;
    int sketchId() const noexcept;
    int wireId() const noexcept;
    double distance() const noexcept;
    const TrackedShape& result() const noexcept;

  private:
    int m_id;
    int m_sketchId;
    int m_wireId;
    double m_distance;
    TrackedShape m_result;
};