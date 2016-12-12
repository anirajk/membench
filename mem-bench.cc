#include <cinttypes>
#include <cstring>
#include <cassert>
#include <atomic>
#include <chrono>
#include <deque>
#include <iostream>
#include <thread>

#include "docopt.h"

using namespace std::chrono_literals;
using namespace std::chrono;
static const char USAGE[]=
R"(mem-bench


   Usage:
     mem-bench --threads=T --size=S

   Options:
     -h --help            Show this screen
     --threads=T          Number of threads [default:2]
     --size=S             Size of array in GB[default:4]
)";
int
main(int argc, const char** argv)
{

    std::map<std::string, docopt::value> args
        = docopt::docopt(USAGE,
                         { argv + 1, argv + argc },
                         true,               // show help if requested
                         "mem-bench 0.1");
    const size_t nThreads = args["--threads"].asLong();
    const size_t s = args["--size"].asLong() * 1024 * 1024 * 1024;
    //const size_t s = 3llu * 1024 * 1024 * 1024;
    //size_t nThreads = 4;
    std::atomic<size_t> nReady{};
    std::atomic<bool> go{};
    std::deque<std::thread> ts{};
    for (size_t t = 0; t < nThreads; ++t)
        ts.emplace_back([&]{
            volatile char* a = new char[s];
            volatile char* b = new char[s];
            assert(a && b); 
            //memcpy(a, b, s);
            for (size_t o = 0; o < s; o += 64) 
                *(a + o) = 'a';
            nReady++;
            while (!go);
            for (size_t o = 0; o < s; o += 64) 
                *(a + o); 
        }); 
    while (nReady < nThreads)
        std::this_thread::sleep_for(10ms);
    go = true;
    auto start = high_resolution_clock::now();
    for (auto& t : ts) 
        t.join();
    auto stop = high_resolution_clock::now();
    double ms = duration_cast<std::chrono::milliseconds>(stop - start).count();
    double mbs = s * nThreads / 1024 / 1024;
    std::cout << nThreads << " Threads" << std::endl
	      << ms << " ms" << std::endl
              << mbs *2 << " MB Allocated" << std::endl
              << mbs << " MB Touched" << std::endl
              << mbs / (ms / 1000.) << " MB/s" << std::endl;
    return 0;
}
