#pragma once

/**
 * @brief Generates unique identifiers for entities
 */
class IdGenerator
{
  public:
    /**
     * @brief Returns next wire identifier
     * @return Wire identifier
     */
    static int nextWireId() noexcept;
};