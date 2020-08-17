#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/cURLpp.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include<mutex>

using namespace std::literals::string_literals;

namespace {

    unsigned ln = 1;
    auto root = "http://iki.fi/bisqwit/ctu85/"s;

    auto Color(int n, const std::string& s) {
        return "\33[38;5;" + std::to_string(n) + 'm' + s + "\33[m";
    }

    auto Line(int l) {
        int m = l - ln; ln = l;
        return "\r" + (m<0 ? "\33[" + std::to_string(-m) + 'A' : std::string(m, '\n'));
    }

    std::mutex print_lock;

    std::size_t Download(const std::string& url, const std::string& filename, unsigned line) {
        std::ofstream of(filename);
        std::size_t written = 0;
        cURLpp::Easy req;
        req.setOpt(cURLpp::Options::Url(url));
        req.setOpt(cURLpp::Options::NoProgress(false));
        req.setOpt(cURLpp::Options::FollowLocation(true));
        req.setOpt(cURLpp::Options::ProgressFunction([&](std::size_t total, std::size_t done, auto...)
            {
                /*
                std::cout << "\r" << done << " of " << total
                    << "bytes received (" << int(total ? done * 100. / total : 0) << "%)" << std::flush;
                */
                std::lock_guard<std::mutex> l(print_lock);
                std::cout << Line(line) << Color(143, filename + ": ") << done << " of " << total
                    << "bytes received (" << int(total ? done * 100. / total : 0) << "%)" << std::flush;
                return 0;
            }));
        req.setOpt(cURLpp::Options::WriteFunction([&](const char* p, std::size_t size, std::size_t nmemb)
            {
                of.write(p, size * nmemb);
                written += size * nmemb;
                return size * nmemb;
            }));
        req.perform();
        return written;
    }
}

#include <vector>
#include <thread>
#include <atomic>

int main()
{
    cURLpp::initialize();

    unsigned line = 1;

    std::vector<std::thread> downloaders;
    std::atomic<std::size_t> total {0};

    for (const auto& p : {
            "8859-1.TXT"s ,"8859-2.TXT"s ,"8859-3.TXT"s ,"8859-4.TXT"s ,"8859-5.TXT"s,
            "8859-6.TXT"s ,"8859-7.TXT"s ,"8859-8.TXT"s ,"8859-9.TXT"s ,"8859-10.TXT"s,
            "8859-11.TXT"s ,"8859-13.TXT"s ,"8859-14.TXT"s ,"8859-15.TXT"s ,"8859-16.TXT"s})
    {
        downloaders.emplace_back([p, l=line++, &total]
        {
           total += Download(root+p, p, l);
        });
    }

    for(auto& p: downloaders) p.join();
    std::cout << Line(line) << Color(174, std::to_string(total) + " bytes downloaded total") << std::endl;
}

/*
$ sudo apt install cmake
$ git clone https://github.com/jpbarrette/curlpp.git
$ cd curlpp/
$ mkdir build
$ cd build
$ sudo apt install libcurl4-openssl-dev
$ cmake ..
$ sudo cmake --build . --target install
$ sudo cp libcurlpp.so.1 /usr/lib/

n3k@kique-asus:~/code$ g++ -std=c++14 download.cc -O -Wall -Wextra -lcurl -lcurlpp -lpthread -pedantic
n3k@kique-asus:~/code$ ./a.out
8859-1.TXT: 9995 of 9995bytes received (100%)
8859-2.TXT: 10219 of 10219bytes received (100%)
8859-3.TXT: 9901 of 9901bytes received (100%)
8859-4.TXT: 10195 of 10195bytes received (100%)
8859-5.TXT: 9830 of 9830bytes received (100%)
8859-6.TXT: 7723 of 7723bytes received (100%)
8859-7.TXT: 10053 of 10053bytes received (100%)
8859-8.TXT: 7943 of 7943bytes received (100%)
8859-9.TXT: 10030 of 10030bytes received (100%)
8859-10.TXT: 10385 of 10385bytes received (100%)
8859-11.TXT: 9192 of 9192bytes received (100%)
8859-13.TXT: 10026 of 10026bytes received (100%)
8859-14.TXT: 10459 of 10459bytes received (100%)
8859-15.TXT: 10039 of 10039bytes received (100%)
8859-16.TXT: 10390 of 10390bytes received (100%)
146380 bytes downloaded total
*/
