#pragma once

#include <vector>
#include <functional>
#include <assert.h>

// Integer division x / y. Takes ceiling.
constexpr int divCeil(int x, int y)
{
    assert(x > 0);
    assert(y > 0);
    return 1 + ((x - 1) / y);
}

// Calculates median of all ints in matrix starting at x0, y0 of size width, height.
// See README for median algorithm.
template<typename MatrixT>
int cellMedian(const MatrixT& matrix, int x0, int y0, int width, int height)
{
    std::vector<int> values;

    assert(x0 + width <= matrix.getWidth());
    assert(y0 + height <= matrix.getHeight());

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            values.push_back(matrix(x0 + x, y0 + y));
        }
    }

    std::sort(values.begin(), values.end());

    assert(values.size() > 0);
    if (values.size() % 2 == 1) {
        // Odd number of elements. Return middle.
        return values[values.size() / 2];
    }
    // Even number of elements. Return mean of middle two.
    return (values[values.size() / 2 - 1] + values[values.size() / 2]) / 2;
}

// Splits a matrix into a grid of width x height cells. Executes function on each cell.
// Returns result as vector (row-major order). See README for algorithm description.
template<typename MatrixT, typename CallableT>
std::vector<int> cellSplitter(const MatrixT& matrix, int gridWidth, int gridHeight, CallableT func)
{
    assert(matrix.getWidth() >= gridWidth);
    assert(matrix.getHeight() >= gridHeight);

    std::vector<int> values;

    // Calculate ceil/floor, and number of cells of each, in Y dimension.
    const int ceilY = divCeil(matrix.getHeight(), gridHeight);
    const int floorY = matrix.getHeight() / gridHeight;
    const int numCeilY = matrix.getHeight() - gridHeight * floorY;

    // Calculate ceil/floor, and number of cells of each, in X dimension.
    const int ceilX = divCeil(matrix.getWidth(), gridWidth);
    const int floorX = matrix.getWidth() / gridWidth;
    const int numCeilX = matrix.getWidth() - gridWidth * floorX;

    // Iterate through each grid cell keeping track of which indices in the matrix
    // have already been used.
    int matrixY = 0; // Y index into original matrix.
    for (int gridY = 0; gridY < gridHeight; gridY++) {

        // Layout ceil-sized cells first.
        const int currentCellHeight = gridY < numCeilY ? ceilY : floorY;
        assert(currentCellHeight > 0);

        int matrixX = 0; // X index into original matrix.
        for (int gridX = 0; gridX < gridWidth; gridX++) {

            const int currentCellWidth = gridX < numCeilX ? ceilX : floorX;
            assert(currentCellWidth > 0);

            values.push_back(func(matrix, matrixX, matrixY, currentCellWidth, currentCellHeight));
            matrixX += currentCellWidth;

        }
        matrixY += currentCellHeight;
    }

    return values;
}