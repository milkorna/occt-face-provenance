#pragma once

#include "TrackedShape.h"

enum class BooleanType
{
    Union = 0,
    Subtract
};

class BooleanFeature
{
  public:
    BooleanFeature(const BooleanType type, const TrackedShape& argument, const TrackedShape& tool);

    int id() const noexcept;
    BooleanType type() const noexcept;
    const TrackedShape& result() const noexcept;

  private:
    int m_id;
    BooleanType m_type;
    TrackedShape m_result;
};