#pragma once

/**
 * @brief Runs examples
 */
class ExampleRunner
{
  public:
    /**
     * @brief Runs basic extrusion and subtraction example
     */
    static void runBasicSubtract();

    /**
     * @brief Runs union with three sequential through-hole cuts
     */
    static void runUnionWithThreeHoles();

    /**
     * @brief Runs cut example with untracked base shape
     */
    static void runUntrackedBaseWithCut();

    /**
     * @brief Runs triple union with overlapping face origins and through hole
     */
    static void runTripleUnionWithThroughHole();
};
