#pragma once

#include <TopoDS_Wire.hxx>

/**
 * @brief Wire with its identifier
 */
struct WireInfo
{
    int m_id;
    TopoDS_Wire m_wire;
};