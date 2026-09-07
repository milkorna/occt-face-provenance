#include "IdGenerator.h"

int IdGenerator::nextSketchId() noexcept
{
    static int sketchId{1};
    return sketchId++;
}

int IdGenerator::nextWireId() noexcept
{
    static int wireId{1};
    return wireId++;
}

int IdGenerator::nextFeatureId() noexcept
{
    static int featureId{1};
    return featureId++;
}