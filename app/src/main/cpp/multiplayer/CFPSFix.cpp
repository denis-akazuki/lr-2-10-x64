#include "CFPSFix.h"
#include "main.h"
#include <sys/syscall.h>
#include <dirent.h>
#include <filesystem>

[[noreturn]] void CFPSFix::Routine()
{
    using namespace std::chrono_literals;
    constexpr uint32_t mask = 255;

    while (true)
    {
        std::this_thread::sleep_for(1s);

        for (const auto& entry : std::filesystem::directory_iterator("/proc/self/task"))
        {
            pid_t tid = std::stoi(entry.path().filename().string());
            syscall(__NR_sched_setaffinity, tid, sizeof(mask), &mask);
        }
    }
}

CFPSFix::CFPSFix()
{
    std::thread(&CFPSFix::Routine, this).detach();
}
