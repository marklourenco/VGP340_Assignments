#include <omp.h>
#include <chrono>
#include <iostream>

using namespace std;

int NUM_THREADS = 4;
int numSteps = 1000000;
double step = 1.0f / (double)numSteps;

// multithreading
float PI_MP(int n)
{
	numSteps = n;

	double x = 0.0f;
	double pi = 0.0f;
	double sum = 0.0f;
	step = 1.0f / (double)numSteps;
	omp_set_num_threads(NUM_THREADS); // sets the number of parallel threads

    #pragma omp parallel for private(x) reduction(+:sum) // copies the code for each thread, copies the x, combines the sum at the end of the loop
	for (int i = 0; i < numSteps; ++i)
	{
		x = (i + 0.5f) * step;
		sum += 4.0f / (1.0f + x * x);
	}

	pi = step * sum;

	return pi;
}

// no multithreading
float PI_NO_MP(int n)
{
	numSteps = n;

	double x = 0.0f;
	double pi = 0.0f;
	double sum = 0.0f;
	step = 1.0f / (double)numSteps;

	// current cpu time in utc
	auto startTime = std::chrono::steady_clock::now();

	for (int i = 0; i < numSteps; ++i)
	{
		x = (i + 0.5f) * step;
		sum += 4.0f / (1.0f + x * x);
	}

	pi = step * sum;
	std::chrono::steady_clock::time_point endTime = std::chrono::steady_clock::now();
	int duration = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime).count();

	return pi;
}

int main()
{
	cout << "Enter the number of steps: ";
	int n;
	cin >> n;

	cout << "Enter the number of threads = ";
	cin >> NUM_THREADS;

	cout << "\n________\n" << endl;

	auto startTime = std::chrono::steady_clock::now();
	cout << "No Multithreading\n" << endl;
	cout << PI_NO_MP(n) << endl;
	std::chrono::steady_clock::time_point endTime = std::chrono::steady_clock::now();
	int duration = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime).count();
	cout << "Duration = " << duration << " ns\n" << endl;

	cout << "________\n" << endl;

	startTime = std::chrono::steady_clock::now();
	cout << "Multithreading\n" << endl;
	cout << PI_MP(n) << endl;
	endTime = std::chrono::steady_clock::now();
	duration = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime).count();
	cout << "Duration = " << duration << " ns" << endl;

	return 0;
}