
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <iostream>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>

namespace mem_watch {

constexpr useconds_t POLL_INTERVAL_US = 500000; // 500 ms

int open_proc_mem(pid_t target_pid) {
    std::string path = "/proc/" + std::to_string(target_pid) + "/mem";
    return open(path.c_str(), O_RDONLY);
}

bool read_int_at(int fd, off_t offset, int& out) {
    ssize_t n = pread(fd, &out, sizeof(out), offset);
    return n == static_cast<ssize_t>(sizeof(out));
}

void print_usage(const char* prog) {
    std::fprintf(stderr, "Use: %s <target_pid> <hex_address>\n", prog);
    std::fprintf(stderr, "Приклад: %s 12345 0x7ffe1a2b3c4d\n", prog);
}

} 

int main(int argc, char* argv[]) {
    if (argc != 3) {
        mem_watch::print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    pid_t target_pid = static_cast<pid_t>(std::stoi(argv[1]));
    off_t target_addr = static_cast<off_t>(std::stoull(argv[2], nullptr, 16));

    int fd = mem_watch::open_proc_mem(target_pid);
    if (fd < 0) {
        std::fprintf(stderr, "[error] Не вдалося відкрити /proc/%d/mem: %s\n",
                     target_pid, std::strerror(errno));
        std::fprintf(stderr, "[hint] Потрібні root-привілеї. Запустіть через sudo.\n");
        return EXIT_FAILURE;
    }

    std::printf("[ok]   Підключено до PID=%d, адреса=0x%lx\n",
                target_pid, static_cast<unsigned long>(target_addr));
    std::printf("[info] Опитування кожні %u мс. Ctrl+C для зупинки.\n\n",
                mem_watch::POLL_INTERVAL_US / 1000);

    int snapshot      = 0;
    int last_snapshot = 0;
    bool first_pass   = true;

    while (true) {
        if (!mem_watch::read_int_at(fd, target_addr, snapshot)) {
            std::printf("[stop] Втрачено доступ до пам'яті (процес ймовірно завершився).\n");
            break;
        }

        if (first_pass) {
            std::printf("[init]   Початкове значення за адресою: %d\n", snapshot);
            first_pass = false;
        } else if (snapshot != last_snapshot) {
            std::printf("[change] Виявлено оновлення: %d -> %d\n",
                        last_snapshot, snapshot);
        }

        last_snapshot = snapshot;
        std::fflush(stdout);
        usleep(mem_watch::POLL_INTERVAL_US);
    }

    close(fd);
    return EXIT_SUCCESS;
}
