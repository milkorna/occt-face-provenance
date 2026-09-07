#pragma once

#include "TrackedShape.h"

/**
 * @brief Supported Boolean operation types
 */
enum class BooleanType
{
    Union = 0,
    Subtract
};

/**
 * @brief Boolean operation feature for tracked shapes
 */
class BooleanFeature
{
  public:
    /**
     * @brief Constructs Boolean feature from argument and tool shapes
     * @param type Boolean operation type
     * @param argument Argument shape
     * @param tool Tool shape
     */
    BooleanFeature(const BooleanType type, const TrackedShape& argument, const TrackedShape& tool);

    /**
     * @brief Returns tracked Boolean result
     * @return Boolean result
     */
    const TrackedShape& result() const noexcept;

  private:
    TrackedShape m_result;
};
