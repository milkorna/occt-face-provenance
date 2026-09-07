#include "IdGenerator.h"

int IdGenerator::nextWireId() noexcept
{
    static int wireId{1};
    return wireId++;
}
