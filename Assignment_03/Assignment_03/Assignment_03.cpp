#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <chrono>

using namespace std;

// define the number of philosophers and the sim time
const int NUM_PHILS = 5;
const int SIM_TIME = 60;

// define the forks mutex
mutex forks[NUM_PHILS];

// Phil class
class Philosopher {
private:
    // id, left fork, right fork
    int id;
    mutex& leftFork;
    mutex& rightFork;
public:
    // constructor
    Philosopher(int i, mutex& left, mutex& right) :
        id(i), leftFork(left), rightFork(right) {}

    // eat
    void Eat()
    {
        // start timer
        auto start = chrono::steady_clock::now();

        // this while loop will run for 60 seconds, the sim time
        while (chrono::duration_cast<chrono::seconds>(chrono::steady_clock::now() - start).count() < SIM_TIME)
        {
            // get the forks
            mutex* first = &leftFork;
			mutex* second = &rightFork;

            // avoid deadlock
            if (&leftFork > &rightFork)
			{
				first = &rightFork;
				second = &leftFork;
			}

            // lock the forks
            {
                lock_guard<mutex> lock1(*first);
                lock_guard<mutex> lock2(*second);

                cout << "Philosopher " << id << " is eating using forks " << id << " and " << (id + 1) % NUM_PHILS << endl;

                this_thread::sleep_for(chrono::seconds(1));

            }

            // unlock the forks
			cout << "Philosopher " << id << " is thinking" << endl;

			this_thread::sleep_for(chrono::seconds(4));
        }
    }
};

int main()
{
    // create the philosophers and threads
    vector<thread> threads;
    vector<Philosopher*> philosophers;

    // push back the philosophers
    for (int i = 0; i < NUM_PHILS; ++i)
    {
		philosophers.push_back(new Philosopher(i, forks[i], forks[(i + 1) % NUM_PHILS]));
    }

    // emplace back the threads
    for (int i = 0; i < NUM_PHILS; ++i)
    {
        threads.emplace_back(&Philosopher::Eat, philosophers[i]);
    }

    // join the threads
    for (auto& t : threads)
	{
		t.join();
	}

    // delete the philosophers after
    for (auto& p : philosophers)
	{
		delete p;
	}

    return 0;
}