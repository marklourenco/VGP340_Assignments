#include <memory>
#include <random>
#include <cstdlib>
#include <iostream>
#include <ctime>
#include <thread>
#include <assert.h>

// omp
#include <omp.h>

// time
#include <chrono>

using namespace std;
using namespace chrono;

struct Matrix
{
	int nRows, nCols;
	long long** data;

	Matrix() = default;
	Matrix(int r, int c) : nRows{ r }, nCols{ c }
	{
		data = new long long* [nRows];
		for (int i = 0; i < nRows; ++i)
		{
			data[i] = new long long[nCols];
		}
	}
	~Matrix()
	{
		if (nRows == 0)
			return;
		for (int i = 0; i < nRows; ++i)
			delete[] data[i];
		delete[] data;
	}
	// initialize the matrix by random numbers
	void init()
	{
		srand(time(0));
		for (int i = 0; i < nRows; ++i)
			for (int j = 0; j < nCols; ++j)
				data[i][j] = rand() % 20 - 10;
	}

	void print()
	{
		cout << "Matrix:\n";
		for (int i = 0; i < nRows; ++i)
		{
			for (int j = 0; j < nCols; ++j)
				cout << data[i][j] << ", ";
			cout << endl;
		}
		cout << endl;
	}

	static void Mult(Matrix const& A, Matrix const& B, Matrix* result)
	{
		//checking if A's num of Col is equal to B's num of Row
		assert(A.nCols == B.nRows && "Matrices num of Cols and Rows do not match!");
		if (A.nCols != B.nRows)
		{
			cout << "A and B dimensions do not match!" <<
				" A.nCols=" << A.nCols << ", B.nRows=" << B.nRows << endl;
			return;
		}

		// do the multiplication
		for (int i = 0; i < A.nRows; ++i)
		{
			for (int j = 0; j < B.nCols; ++j)
			{
				long long res = 0;
				for (int k = 0; k < A.nCols; ++k)
					res += A.data[i][k] * B.data[k][j];
				result->data[i][j] = res;
			}
		}
	}

	static void MultMP(Matrix const& A, Matrix const& B, Matrix* result)
	{
		//checking if A's num of Col is equal to B's num of Row
		assert(A.nCols == B.nRows && "Matrices num of Cols and Rows do not match!");
		if (A.nCols != B.nRows)
		{
			cout << "A and B dimensions do not match!" <<
				" A.nCols=" << A.nCols << ", B.nRows=" << B.nRows << endl;
			return;
		}

		// do the multiplication
#pragma omp parallel for
		for (int i = 0; i < A.nRows; ++i)
		{
			for (int j = 0; j < B.nCols; ++j)
			{
				long long res = 0;
				for (int k = 0; k < A.nCols; ++k)
					res += A.data[i][k] * B.data[k][j];
				result->data[i][j] = res;
			}
		}
	}
};

void PrintMP(Matrix const& M)
{
	int rows = min(5, M.nRows);
	int cols = min(5, M.nCols);

	for (int i = 0; i < rows; ++i)
	{
		for (int j = 0; j < cols; ++j)
		{
			cout << M.data[i][j] << " ";
		}
		cout << endl;
	}
	cout << endl;
}

int main()
{
	int size = 500;

	Matrix A(size, size);
	Matrix B(size, size);
	Matrix resultSerial(size, size);
	Matrix resultParallel(size, size);

	A.init();
	B.init();

	// SERIAL
	auto startTime = chrono::steady_clock::now();
	Matrix::Mult(A, B, &resultSerial);
	chrono::steady_clock::time_point endTime = chrono::steady_clock::now();
	int duration = chrono::duration_cast<chrono::nanoseconds>(endTime - startTime).count();

	cout << "Serial Time: " << duration << " ns" << endl;

	// PARALLEL
	omp_set_num_threads(4);

	startTime = chrono::steady_clock::now();
	Matrix::MultMP(A, B, &resultParallel);
	endTime = chrono::steady_clock::now();
	duration = chrono::duration_cast<chrono::nanoseconds>(endTime - startTime).count();

	cout << "Parallel Time: " << duration << " ns" << endl;

	// PRINT 5x5
	cout << "\nSerial (5x5):\n";
	PrintMP(resultSerial);

	cout << "\nParallel (5x5):\n";
	PrintMP(resultParallel);

	return 0;
}