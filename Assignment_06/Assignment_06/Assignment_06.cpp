/**
 * Challenge: Sort an array of random integers with merge sort
 */
#include <thread>
#include <random>
#include <chrono>
#include <iostream>
#include <algorithm>
#include <vector>

const unsigned int L = 2048;

/* declaration of merge helper function */
void merge(int* array, unsigned int left, unsigned int mid, unsigned int right);

int Partition(int* array, int start, int end);

/* sequential implementation of merge sort */
void sequential_merge_sort(int* array, unsigned int left, unsigned int right)
{
	if (right - left + 1 <= (int)L)
	{
		std::sort(array + left, array + right + 1);
		return;
	}
	if (left < right)
	{
		unsigned int mid = (left + right) / 2; // find the middle point
		sequential_merge_sort(array, left, mid); // sort the left half
		sequential_merge_sort(array, mid + 1, right); // sort the right half
		merge(array, left, mid, right); // merge the two sorted halves
	}
}

void Sequential_Quick_Sort(int* array, int start, int end)
{
	if (end - start + 1 <= (int)L)
	{
		std::sort(array + start, array + end + 1);
		return;
	}
	if (start < end)
	{
		int pivot = Partition(array, start, end);
		Sequential_Quick_Sort(array, start, pivot - 1);
		Sequential_Quick_Sort(array, pivot + 1, end);
	}
}

/* parallel implementation of merge sort */
void parallel_merge_sort(int* array, unsigned int left, unsigned int right)
{
	/***********************
	 * YOUR CODE GOES HERE *
	 ***********************/

	if (right - left + 1 <= (int)L)
	{
		std::sort(array + left, array + right + 1);
		return;
	}
	if (left < right)
	{
		unsigned int mid = (left + right) / 2;

		std::thread leftThread([array, left, mid]() {
			parallel_merge_sort(array, left, mid);
			});
		parallel_merge_sort(array, mid + 1, right);
		leftThread.join();

		merge(array, left, mid, right);
	}
}

void Parallel_Quick_Sort(int* array, int start, int end)
{
	if (end - start + 1 <= (int)L)
	{
		std::sort(array + start, array + end + 1);
		return;
	}
	if (start < end)
	{
		int pivot = Partition(array, start, end);

		std::thread leftThread([array, start, pivot]() {
			Parallel_Quick_Sort(array, start, pivot - 1);
			});
		Parallel_Quick_Sort(array, pivot + 1, end);
		leftThread.join();
	}
}

void Swap(int& a, int& b)
{
	int t = a;
	a = b;
	b = t;
}

int Partition(int* array, int start, int end)
{
	int pivot = array[end];
	int i = start - 1;
	for (int j = start; j < end; ++j)
	{
		if (array[j] < pivot)
		{
			++i;
			Swap(array[i], array[j]);
		}
	}
	Swap(array[i + 1], array[end]);
	return i + 1;
}

/* helper function to merge two sorted subarrays
   array[l..m] and array[m+1..r] into array */
void merge(int* arr, unsigned int left, unsigned int mid, unsigned int right)
{
	unsigned int num_left = mid - left + 1; // number of elements in left subarray
	unsigned int num_right = right - mid; // number of elements in right subarray

	// copy data into temporary left and right subarrays to be merged
	auto  array_left{ std::make_shared<int[]>(num_left) };
	auto  array_right{ std::make_shared<int[]>(num_right) };

	std::copy(&arr[left], &arr[mid + 1], array_left.get());
	std::copy(&arr[mid + 1], &arr[right + 1], array_right.get());

	// initialize indices for array_left, array_right, and input subarrays
	unsigned int index_left = 0;    // index to get elements from array_left
	unsigned int index_right = 0;    // index to get elements from array_right
	unsigned int index_insert = left; // index to insert elements into input array

	// merge temporary subarrays into original input array
	while ((index_left < num_left) || (index_right < num_right))
	{
		if ((index_left < num_left) && (index_right < num_right))
		{
			if (array_left[index_left] <= array_right[index_right])
			{
				arr[index_insert] = array_left[index_left];
				index_left++;
			}
			else
			{
				arr[index_insert] = array_right[index_right];
				index_right++;
			}
		}
		// copy any remain elements of array_left into array
		else if (index_left < num_left)
		{
			arr[index_insert] = array_left[index_left];
			index_left += 1;
		}
		// copy any remain elements of array_right into array
		else if (index_right < num_right)
		{
			arr[index_insert] = array_right[index_right];
			index_right += 1;
		}
		++index_insert;
	}
}

int main()
{
	std::mt19937 generator((unsigned int)std::chrono::system_clock::now().time_since_epoch().count());

	const int NUM_EVAL_RUNS = 2;
	const int N = 10000000; // number of elements to sort

	std::vector<int> original_array, seq_merge_result, par_merge_result;
	std::vector<int> seq_quick_result, par_quick_result;
	original_array.reserve(N);
	seq_merge_result.reserve(N);
	par_merge_result.reserve(N);

	seq_quick_result.reserve(N);
	par_quick_result.reserve(N);

	for (int i = 0; i < N; i++)
	{
		original_array.push_back(generator() % 2000000 - 1000000);
	}

	// seq merge sort

	std::cout << "Evaluating Merge Sequential Implementation...\n";
	std::chrono::duration<double> sequential_merge_time(0);
	seq_merge_result = original_array;
	//std::copy(&original_array[0], &original_array[N-1], seq_merge_result);
	sequential_merge_sort(&seq_merge_result[0], 0, N - 1); // "warm up"	
	for (int i = 0; i < NUM_EVAL_RUNS; i++)
	{
		seq_merge_result = original_array;
		//	std::copy(&original_array[0], &original_array[N-1], seq_merge_result); // reset result array
		auto start_time = std::chrono::high_resolution_clock::now();
		sequential_merge_sort(&seq_merge_result[0], 0, N - 1);
		sequential_merge_time += std::chrono::high_resolution_clock::now() - start_time;
	}
	sequential_merge_time /= NUM_EVAL_RUNS;

	// par merge sort

	printf("Evaluating Merge Parallel Implementation...\n");
	std::chrono::duration<double> parallel_merge_time(0);

	par_merge_result = original_array;
	//	std::copy(&original_array[0], &original_array[N-1], par_merge_result);
	parallel_merge_sort(&par_merge_result[0], 0, N - 1); // "warm up"
	for (int i = 0; i < NUM_EVAL_RUNS; i++)
	{
		par_merge_result = original_array;
		//	std::copy(&original_array[0], &original_array[N - 1], par_merge_result); // reset result array
		auto start_time = std::chrono::high_resolution_clock::now();
		parallel_merge_sort(&par_merge_result[0], 0, N - 1);
		parallel_merge_time += std::chrono::high_resolution_clock::now() - start_time;
	}
	parallel_merge_time /= NUM_EVAL_RUNS;

	// seq quick sort

	std::cout << "Evaluating QuickSort Sequential Implementation...\n";
	std::chrono::duration<double> sequential_quick_time(0);
	seq_quick_result = original_array;
	Sequential_Quick_Sort(&seq_quick_result[0], 0, N - 1);
	for (int i = 0; i < NUM_EVAL_RUNS; ++i)
	{
		seq_quick_result = original_array;
		auto start = std::chrono::high_resolution_clock::now();
		Sequential_Quick_Sort(&seq_quick_result[0], 0, N - 1);
		sequential_quick_time += std::chrono::high_resolution_clock::now() - start;
	}
	sequential_quick_time /= NUM_EVAL_RUNS;

	// par quick sort

	std::cout << "Evaluating QuickSort Parallel Implementation...\n";
	std::chrono::duration<double> parallel_quick_time(0);
	par_quick_result = original_array;
	Parallel_Quick_Sort(&par_quick_result[0], 0, N - 1);
	for (int i = 0; i < NUM_EVAL_RUNS; ++i)
	{
		par_quick_result = original_array;
		auto start = std::chrono::high_resolution_clock::now();
		Parallel_Quick_Sort(&par_quick_result[0], 0, N - 1);
		parallel_quick_time += std::chrono::high_resolution_clock::now() - start;
	}
	parallel_quick_time /= NUM_EVAL_RUNS;

	// verify sequential and parallel results are same
	for (int i = 0; i < 10; i++)
	{
		if (seq_merge_result[i] != par_merge_result[i])
		{
			std::cout << "ERROR: Result mismatch at index " << i << std::endl;
		}
	}

	std::cout << "\n\nRESULTS:\nL = " << L << "\nN = " << N << "\n\n";

	std::cout << "Merge Average Sequential Time: " << sequential_merge_time.count() * 1000 << " ms \n";
	std::cout << "  Merge Average Parallel Time: " << parallel_merge_time.count() * 1000 << " ms \n";
	std::cout << "Merge Speedup: " << sequential_merge_time / parallel_merge_time << " ms \n";
	std::cout << "Merge Efficiency " << 100 * (sequential_merge_time / parallel_merge_time) / std::thread::hardware_concurrency() << " ms \n";

	std::cout << "\n\n";

	// verify sequential and parallel results are same
	for (int i = 0; i < 10; i++)
	{
		if (seq_quick_result[i] != par_quick_result[i])
		{
			std::cout << "ERROR: Result mismatch at index " << i << std::endl;
		}
	}
	std::cout << "QuickSort Average Sequential Time: " << sequential_quick_time.count() * 1000 << " ms \n";
	std::cout << "  QuickSort Average Parallel Time: " << parallel_quick_time.count() * 1000 << " ms \n";
	std::cout << "QuickSort Speedup: " << sequential_quick_time / parallel_quick_time << " ms \n";
	std::cout << "QuickSort Efficiency " << 100 * (sequential_quick_time / parallel_quick_time) / std::thread::hardware_concurrency() << " ms \n";

	return 0;
}