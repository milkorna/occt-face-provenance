#pragma once

class IdGenerator
{
public:
    static int nextSketchId() noexcept;
    static int nextWireId() noexcept;
    static int nextFeatureId() noexcept;
};