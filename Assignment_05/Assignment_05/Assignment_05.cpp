#include <iostream>
#include <limits>
#include <omp.h>
#include <chrono>
#include <numeric>
#include <execution>
#include <functional>
#include <iterator>
#include <random>
#include <thread>

long num_steps{ 10000000 };
const double M_PI{ 3.14159265358979323846 };

// given integral method
void PI_Integral()
{
	auto startTime{ std::chrono::steady_clock::now() };
	const double step{ 1.0 / (double)num_steps };
	double sum{ 0.0 };
	const int num_threads{ omp_get_max_threads() };
	//double* sums{ new double[2*num_threads] };
	int num_runs{ 10 };
	for (int k{ 0 }; k < num_runs; ++k) {  // repeating the computation num_runs times
#pragma omp parallel
		{
			int id{ omp_get_thread_num() };
			int iStart{ (id * num_steps) / num_threads };
			int iEnd{ ((id + 1) * num_steps) / num_threads };
			//std::cout << "iStart=" << iStart << ", iEnd=" << iEnd << std::endl;
			if (id == num_threads - 1)
				iEnd = num_steps;
			double localsum{ 0.0 };
			//sums[id] = 0.0;
			for (int i{ iStart }; i < iEnd; ++i)
			{
				double x = (i + 0.5) * step;
				localsum += 4.0 / (1.0 + x * x);
			}
#pragma omp critical
			sum += localsum;
		}
	}
	//	for (int i = 0; i < num_threads; ++i)  // removing false sharing by replace sums by localsum
	//		sum += sums[i];

	double pi{ sum * step };
	pi = pi / double(num_runs);
	auto endTime{ std::chrono::steady_clock::now() };
	std::cout.precision(15);
	std::cout << "PI for num_steps = " << num_steps << " num_threads=" << num_threads << " is " << pi << " vs exact PI=" << M_PI << std::endl;
	std::cout << "Elapsed time in milliseconds: " << std::chrono::duration_cast<std::chrono::milliseconds>
		(endTime - startTime).count() / num_runs << std::endl;
}

#include <vector>
#include <algorithm>

void PI_MonteCarlo()
{
    auto startTime{ std::chrono::steady_clock::now() }; // start clock

    int num_threads{ omp_get_max_threads() }; // get number of threads (16)
    int num_runs{ 10 }; // repeats whole computation 10 times
    double pi{ 0.0 }; // accumulates pi then divided by num_runs to get average
    const double step{ 1.0 / (double)num_steps }; // steps between rectangles from the function f(x) = 4/(1+x^2)

    std::vector<long> indices(num_steps); // vector with number of steps
    std::iota(indices.begin(), indices.end(), 0); // fills the vector

    for (int k{ 0 }; k < num_runs; ++k) // repeat for the amount of runs (10)
    {
        long hits = std::transform_reduce(
            std::execution::par, // parallel execution, which is the equivalent to omp parallel
            indices.begin(),
            indices.end(), // iterator range, all elements from 0 to num_steps
            0L, // start at 0
            std::plus<long>{}, // reduce operation. plus adds the values
            [](long i) -> long // takes index i, i is not used. num_steps is used
            {
                thread_local std::mt19937_64 rng( // random number. thread_local means one instance per thread. this way avoids mutex use
                    std::random_device{}() ^ (std::hash<std::thread::id>{}(
                        std::this_thread::get_id()) * 2654435761ULL)
                );
                thread_local std::uniform_real_distribution<double> dist(0.0, 1.0); // uniform distribution from 0 to 1

                double x{ dist(rng) }; // random (x, y) point
                double y{ dist(rng) };
                return (x * x + y * y <= 1.0) ? 1L : 0L; // if point is inside the circle, return 1, else 0
            }
        );

        pi += 4.0 * (double)hits / (double)num_steps; // approx area if quarter circle. *4 to get pi
    }

    pi = pi / (double)num_runs; // avg betwee runs

    auto endTime{ std::chrono::steady_clock::now() };
    std::cout.precision(15);
    std::cout << "PI for num_steps = " << num_steps << " num_threads=" << num_threads << " is " << pi << " vs exact PI=" << M_PI << std::endl;
    std::cout << "Elapsed time in milliseconds: " << std::chrono::duration_cast<std::chrono::milliseconds>
        (endTime - startTime).count() / num_runs << std::endl;
}

int main()
{
	PI_Integral();

    std::cout << "\n\n";

	PI_MonteCarlo();
}