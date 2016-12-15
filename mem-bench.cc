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

   Basic Memory Bandwidth measurements over 2 arrays
   Usage:
     mem-bench [--threads=T] [--size=S] [--mode=m] [--verbose]

   Options:
     -h --help            Show this screen
     --verbose            Show more verbose output
     --threads=T          Number of threads [default: 2] 
     --size=S             Size of each array in MB [default: 512]
     --mode=m             Mode (read,write,assign,memcpy) [default: all]
)";
int
main(int argc, const char** argv)
{

    std::map<std::string, docopt::value> args
        = docopt::docopt(USAGE,
                         { argv + 1, argv + argc },
                         true,               // show help if requested
                         "mem-bench 0.2");
    const size_t nThreads = args["--threads"].asLong();
    const size_t s = 1024 * 1024 * args["--size"].asLong();
    const bool verbose = bool(args["--verbose"])&&
			 args["--verbose"].asBool();
    const char *strmodes[] =
    {
     "read",
     "write",
     "assign",
     "memcpy",
     "all"
    };
    //const size_t s = 3llu * 1024 * 1024 * 1024;
    //size_t nThreads = 4;
    std::atomic<size_t> nReady{};
    std::atomic<bool> go{};
    std::deque<std::thread> ts{};
    enum Mode { MODE_READ, MODE_WRITE, MODE_ASSIGN, MODE_MEMCPY, MODE_ALL };
    Mode mode = MODE_ALL;
    if(args["--mode"].asString() == "read"){
       mode = MODE_READ;
    }
    else if(args["--mode"].asString() == "write"){
       mode = MODE_WRITE;
    }
    else if(args["--mode"].asString() == "assign"){
       mode = MODE_ASSIGN;
    }
    else if(args["--mode"].asString() == "memcpy"){
       mode = MODE_MEMCPY;
    }
    else if(args["--mode"].asString() == "all"){
       mode = MODE_ALL;
    }
    for (size_t t = 0; t < nThreads; ++t)
        ts.emplace_back([&]{
            volatile char* a = new char[s];
            volatile char* b = new char[s];
            assert(a && b); 
            for (size_t o = 0; o < s; o += 64){ 
                *(a + o) = 'a';
                *(b + o) = 'b';
	    }
            nReady++;
            while (!go);
	    for(size_t o = 0; o < s; o += 64){
                    if (o>=s-64){
                        if (mode==MODE_MEMCPY || mode==MODE_ALL){
                           memcpy(const_cast<char*>(a),const_cast<char*>(b),s);
			   break;
                       }
                    }
		    switch(mode){
			case MODE_READ:
			    *(a + o);
			    *(b + o);
			    break;
			case MODE_WRITE:
			    *(a + o) = 'b';
			    *(b + o) = 'a';
			    break;
			case MODE_ASSIGN:
			    *(b + o) = *(a + o);
			    break;
                        case MODE_MEMCPY:
                            break;
			case MODE_ALL:
			    *(a + o);
			    *(b + o);
			    *(a + o) = 'b';
			    *(b + o) = 'a';
			    *(b + o) = *(a + o);
			    break;
                        default:
                            assert(0);
		    }
	}
        }); 
    while (nReady < nThreads)
        std::this_thread::sleep_for(10ms);
    go = true;
    auto start = high_resolution_clock::now();
    for (auto& t : ts) 
        t.join();
    auto stop = high_resolution_clock::now();
    double ms = duration_cast<std::chrono::milliseconds>(stop - start).count();
    double mbs;
    if (mode==MODE_ALL)
      mbs = s * 8 * nThreads / 1024 / 1024;
    else
      mbs = s * 2 * nThreads / 1024 / 1024;
    if(verbose)
        std::cout << nThreads << " Threads" << std::endl
	          << ms << " ms" << std::endl
                  << mbs << " MB Touched" << std::endl;
    std::cout <<"mode " << strmodes[mode] << std::endl
              << mbs / (ms / 1000.) << " MB/s" << std::endl;
    return 0;
}
