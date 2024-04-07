#include <iostream>
#include <cmath>
#include <cstdlib>
#include <chrono>
#include <ctime>
#include <vector>
#include "tbb/tbb.h"

using namespace std;
using namespace tbb;

double** createMatrix(int length, bool isSecond) {
    double** matrix = new double* [length];
    for (int i = 0; i < length; i++) {
        if (!isSecond) {
            matrix[i] = new double[length];
            for (int j = 0; j < length; j++) {
                matrix[i][j] = rand() % 10;
            }
        } else {
            matrix[i] = new double[1];
            matrix[i][0] = rand() % 10;
        }
    }
        
    return matrix;
}

void printMatrix(double** matrix, int length, bool isSecond) {
    for (int i = 0; i < length; i++) {
        if (!isSecond) {
            for(int j = 0; j < length; j++) {
                cout << matrix[i][j] << ' ';
            }
        } else {
            cout << matrix[i][0];
        }
        
        cout << endl;
    }
}

double* solveSystem(double** first, double** second, int length) {
    double* x = new double[length];

    for (int i = 0; i < length; i++) {
        double div = first[i][i];
        parallel_for(blocked_range<int>(i, length), [&](const blocked_range<int>& r) {
            for (int j = r.begin(); j != r.end(); j++) {
                first[i][j] /= div;
            }
        });
        second[i][0] /= div;

        parallel_for(blocked_range<int>(i + 1, length), [&](const blocked_range<int>& r) {
            for (int k = r.begin(); k != r.end(); k++) {
                double multiplier = first[k][i];
                for (int j = i; j < length; j++) {
                    first[k][j] -= multiplier * first[i][j];
                }
                second[k][0] -= multiplier * second[i][0];
            }
        });
    }

    for (int i = length - 1; i >= 0; i--) {
        x[i] = second[i][0];
        parallel_for(blocked_range<int>(i + 1, length), [&](const blocked_range<int>& r) {
            for (int j = r.begin(); j != r.end(); j++) {
                x[i] -= first[i][j] * x[j];
            }
        });
    }

    return x;
}

int main() {
    int length = 3000;
    double** a = createMatrix(length, false);
    double** b = createMatrix(length, true);
//    printMatrix(a, length, false);
    
//    cout << endl << "=" << endl << endl;
//    printMatrix(b, length, true);
//    cout << endl << "Result: " << endl << endl;

    auto start = std::chrono::steady_clock::now();
    double* result = solveSystem(a, b, length);

    auto end = std::chrono::steady_clock::now();

//    for(int i = 0; i < length; i++) {
//        cout << result[i] << endl;
//    }

    std::chrono::duration<double> solving_time = end - start;
    cout << "Elapsed Time: " << solving_time.count() << " seconds" << endl;

    return 0;
}

