#include <mutex>
#include <iostream>
#include <random>
#include <chrono>
#include <future>
#include <vector>
#include <cmath>
#include <limits>
#include <atomic>
#include <thread>
#include <condition_variable>


struct Point {
    float x, y;
    bool isStop;  // if true, this is a stop signal, not a real point
                  // if false, this is a real point

    //  default constructor (0, 0)
    Point() : x(0.0f), y(0.0f), isStop(false) {}

    // named constructor for real points
    Point(float x, float y) : x(x), y(y), isStop(false) {}

    // named constructor for the stop
    static Point MakeStop() {
        Point p;
        p.isStop = true;
        return p;
    }
};

struct CircularBuffer {
    Point* buf;          // point instead of int
    int capacity;
    int frontIdx;        // next read position
    int rearIdx;         // next write position
    int count;           // number of items currently in the buffer

    std::mutex m;
    std::condition_variable notEmpty;  // signal this when add
    std::condition_variable notFull;   // signal this when remove

    CircularBuffer(int cap)
        : capacity(cap), frontIdx(0), rearIdx(0), count(0),
        buf(new Point[cap]) {
    }
    ~CircularBuffer() { delete[] buf; }
    
    // blocks (sleeps) if the buffer is full, wakes when room made
    void push(const Point& pt) {
        std::unique_lock<std::mutex> lk(m);
        // wait until there is at least one free slot
        notFull.wait(lk, [this]() { return count != capacity; });

        buf[rearIdx] = pt;
        rearIdx = (rearIdx + 1) % capacity;
        ++count;

        lk.unlock();
        notEmpty.notify_one();  // wake one sleeping
    }

    // pop
    // blocks (sleeps) if the buffer is empty, wakes when data added
    Point pop() {
        std::unique_lock<std::mutex> lk(m);
        // wait until there is at least one item to read
        notEmpty.wait(lk, [this]() { return count > 0; });

        Point data = buf[frontIdx];
        frontIdx = (frontIdx + 1) % capacity;
        --count;

        lk.unlock();
        notFull.notify_one();  // wake the producer if it was blocked
        return data;
    }
};

// distance between two points
float Distance(const Point& a, const Point& b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return std::sqrt(dx * dx + dy * dy);
}

struct ClosestPair {
    int   i = -1, j = -1;
    float dist = std::numeric_limits<float>::max();
};

ClosestPair FindClosestPair(const std::vector<Point>& pts) {
    ClosestPair best;
    for (int i = 0; i < (int)pts.size(); ++i) {
        for (int j = i + 1; j < (int)pts.size(); ++j) {
            float d = Distance(pts[i], pts[j]);
            if (d < best.dist) {
                best.dist = d;
                best.i = i;
                best.j = j;
            }
        }
    }
    return best;
}

void Consumer(CircularBuffer& buf, int quartetId) {
    std::vector<Point> localPts;  // points that belong to this quartet

    while (true) {
        Point pt = buf.pop();  // blocks until data is available

        if (pt.isStop) break;  // producer is done so exit

        bool belongs = false;
        switch (quartetId) {
        case 1: belongs = (pt.x >= 0.f && pt.y >= 0.f); break;  // (+,+)
        case 2: belongs = (pt.x < 0.f && pt.y >= 0.f); break;  // (-,+)
        case 3: belongs = (pt.x < 0.f && pt.y < 0.f); break;  // (-,-)
        case 4: belongs = (pt.x >= 0.f && pt.y < 0.f); break;  // (+,-)
        }

        if (!belongs) continue;

        localPts.push_back(pt);

        if (localPts.size() < 2) continue;

        ClosestPair cp = FindClosestPair(localPts);

        // print result
        static std::mutex printMutex;
        std::lock_guard<std::mutex> lk(printMutex);
        std::cout
            << "quartet " << quartetId << ": closest points are ("
            << localPts[cp.i].x << ", " << localPts[cp.i].y << ") and ("
            << localPts[cp.j].x << ", " << localPts[cp.j].y
            << ") and their distance is " << cp.dist
            << ". Total number of points in this quartet is "
            << localPts.size() << ".\n";
    }

    static std::mutex printMutex2;
    std::lock_guard<std::mutex> lk(printMutex2);
    if (localPts.size() >= 2) {
        ClosestPair cp = FindClosestPair(localPts);
        std::cout
            << "\n[FINAL] quartet " << quartetId
            << ": closest points are ("
            << localPts[cp.i].x << ", " << localPts[cp.i].y << ") and ("
            << localPts[cp.j].x << ", " << localPts[cp.j].y
            << ") - distance " << cp.dist
            << ". Total points: " << localPts.size() << ".\n";
    }
    else {
        std::cout
            << "\n[FINAL] quartet " << quartetId
            << ": not enough points to compute a closest pair ("
            << localPts.size() << " points).\n";
    }
}

void Producer(CircularBuffer& buf, int numConsumers) {
    constexpr int TOTAL_POINTS = 10000;

    std::mt19937 gen(
        (unsigned int)std::chrono::system_clock::now().time_since_epoch().count()
    );

    // [-1000, 1000]
    std::uniform_real_distribution<float> dist(-1000.f, 1000.f);

    for (int i = 0; i < TOTAL_POINTS; ++i) {
        float x = dist(gen);
        float y = dist(gen);
        buf.push(Point(x, y));

        // 30 ms pause
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }

    for (int i = 0; i < numConsumers; ++i) {
        buf.push(Point::MakeStop());
    }
}

int main() {
    constexpr int NUM_CONSUMERS = 4;

    CircularBuffer dataBuf(400);

    std::future<void> f1 = std::async(std::launch::async, Consumer, std::ref(dataBuf), 1);
    std::future<void> f2 = std::async(std::launch::async, Consumer, std::ref(dataBuf), 2);
    std::future<void> f3 = std::async(std::launch::async, Consumer, std::ref(dataBuf), 3);
    std::future<void> f4 = std::async(std::launch::async, Consumer, std::ref(dataBuf), 4);

    Producer(dataBuf, NUM_CONSUMERS);
    f4.get();
    f3.get();
    f2.get();
    f1.get();

    return 0;
}