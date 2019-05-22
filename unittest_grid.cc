#include "grid.h"
#include <assert.h>
#include <vector>

using namespace std;

// Simple int matrix type to use for testing.
class Matrix
{
public:
    Matrix(initializer_list<initializer_list<int>> rows) :
        height(rows.size()),
        width(rows.begin()->size()),
        data()
    {
        for (const auto& row : rows) {
            data.insert(data.end(), row);
        }
    }

    int operator()(int x, int y) const
    {
        return data[y * width + x];
    }

    int getWidth() const { return width; }
    int getHeight() const { return height; }

private:
    const int height;
    const int width;
    vector<int> data;
};

// Tests median calculation.
void test_median(void)
{
    Matrix m1{
        {8, 3, 5},
        {9, 9, 5},
        {2, 1, 6}
    };
    assert(cellMedian(m1, 0, 0, 3, 3) == 5);
    assert(cellMedian(m1, 1, 1, 2, 2) == 5); // Mean of middle two elements.
    assert(cellMedian(m1, 0, 0, 1, 1) == 8);
    assert(cellMedian(m1, 0, 1, 2, 1) == 9);
}

// Alternate cell method used for testing purposes.
int cellSum(const Matrix& matrix, int x0, int y0, int width, int height)
{
    int sum = 0;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            sum += matrix(x0 + x, y0 + y);
        }
    }
    return sum;
}

// Tests splitting of matrix into grid.
void test_cellSplitter(void)
{
    Matrix m1{
        {0, 1, 2, 3},
        {4, 5, 6, 7},
        {8, 9, 0, 1},
        {2, 3, 4, 5}
    };

    auto answer = cellSplitter(m1, 2, 2, cellSum);
    assert(answer.size() == 4);
    assert(answer[0] == 10);
    assert(answer[1] == 18);
    assert(answer[2] == 22);
    assert(answer[3] == 10);

    answer = cellSplitter(m1, 3, 2, cellSum);
    assert(answer.size() == 6);
    assert(answer[0] == 10);
    assert(answer[1] == 8);
    assert(answer[2] == 10);
    assert(answer[3] == 22);
    assert(answer[4] == 4);
    assert(answer[5] == 6);

    Matrix m2{
        {0, 1, 2},
        {3, 4, 5},
        {6, 7, 8}
    };
    
    answer = cellSplitter(m2, 2, 2, cellSum);
    assert(answer.size() == 4);
    assert(answer[0] == 8);
    assert(answer[1] == 7);
    assert(answer[2] == 13);
    assert(answer[3] == 8);

    Matrix m3{ {1} };
    answer = cellSplitter(m3, 1, 1, cellSum);
    assert(answer.size() == 1);
    assert(answer[0] == 1);

    Matrix m4{
        {1, 2},
        {3, 4}
    };
    answer = cellSplitter(m4, 2, 2, cellMedian<Matrix>);
    assert(answer.size() == 4);
    assert(answer[0] == 1);
    assert(answer[1] == 2);
    assert(answer[2] == 3);
    assert(answer[3] == 4);

    Matrix m5{
       {0, 1, 2, 3, 4, 5, 6, 7, 8, 9},
       {1, 3, 4, 2, 5, 3, 2, 1, 2, 5},
       {4, 6, 7, 8, 4, 3, 6, 7, 8, 9},
       {3, 5, 3, 2, 1, 5, 6, 8, 5, 4},
       {3, 2, 5, 7, 6, 5, 4, 3, 3, 3},
       {3, 2, 1, 5, 6, 8, 7, 6, 4, 3} 
    };
    answer = cellSplitter(m5, 3, 2, cellMedian<Matrix>);
    assert(answer.size() == 3 * 2);
    assert(answer[0] == 3);
    assert(answer[1] == 4);
    assert(answer[3 * 2 - 1] == 4);
}

void unittest_grid(void)
{
    assert(divCeil(7, 5) == 2);
    assert(divCeil(8, 3) == 3);

    test_median();
    test_cellSplitter();
}