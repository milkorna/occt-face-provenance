#pragma once

#include "TrackedShape.h"

class Sketch;

/**
 * @brief Extrusion feature created from sketch wire
 */
class ExtrudeFeature
{
  public:
    /**
     * @brief Constructs extrusion feature from sketch wire
     * @param sketch Source sketch
     * @param wireId Source wire identifier
     * @param distance Extrusion distance
     */
    ExtrudeFeature(const Sketch& sketch, const int wireId, const double distance);

    /**
     * @brief Returns tracked extrusion result
     * @return Extrusion result
     */
    const TrackedShape& result() const noexcept;

  private:
    TrackedShape m_result;
};