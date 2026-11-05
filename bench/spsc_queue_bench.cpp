#include <hft/lockfree/spsc_queue.hpp>

#include <atomic>
#include <barrier>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <thread>

using clock_type = std::chrono::steady_clock;

template<std::size_t Capacity>
void benchmark(std::uint64_t iterations)
{
    hft::spsc_queue<std::uint64_t> queue{Capacity};

    std::barrier sync{3};

    std::uint64_t checksum = 0;

    std::thread producer([&] {
        sync.arrive_and_wait();

        for(std::uint64_t i = 0; i < iterations; ++i) {
            while(!queue.try_push(i));
        }
    });

    std::thread consumer([&] {
        sync.arrive_and_wait();

        std::uint64_t localChecksum = 0;

        for(std::uint64_t i = 0; i < iterations;) {
            if(auto* value = queue.front()) {
                localChecksum += *value;
                queue.try_pop();
                ++i;
            }
        }

        checksum = localChecksum;
    });

    auto const start = clock_type::now();

    sync.arrive_and_wait();

    producer.join();
    consumer.join();

    auto const end = clock_type::now();

    auto const elapsed =
        std::chrono::duration<double>(end - start).count();

    auto const throughput = iterations / elapsed;

    std::cout
        << "Capacity:   " << Capacity << '\n'
        << "Elements:   " << iterations << '\n'
        << "Time:       " << elapsed << " s\n"
        << "Throughput: " << throughput / 1'000'000.0
        << " million elements/s\n"
        << "Checksum:   " << checksum << "\n\n";
}

int main()
{
    constexpr std::uint64_t iterations = 100'000'000;

    // benchmark<1>(iterations);
    // benchmark<16>(iterations);
    // benchmark<256>(iterations);
    benchmark<1024>(iterations);
    // benchmark<65536>(iterations);
}
