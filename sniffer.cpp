
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <iostream>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>

namespace kb_sniffer {

const char* key_name(unsigned short code) {
    switch (code) {
        case KEY_ENTER:     return "ENTER";
        case KEY_SPACE:     return "SPACE";
        case KEY_BACKSPACE: return "BACKSPACE";
        case KEY_TAB:       return "TAB";
        case KEY_ESC:       return "ESC";
        case KEY_LEFTSHIFT:
        case KEY_RIGHTSHIFT: return "SHIFT";
        case KEY_LEFTCTRL:
        case KEY_RIGHTCTRL:  return "CTRL";
        case KEY_LEFTALT:
        case KEY_RIGHTALT:   return "ALT";
        case KEY_CAPSLOCK:   return "CAPSLOCK";
        case KEY_UP:         return "ARROW_UP";
        case KEY_DOWN:       return "ARROW_DOWN";
        case KEY_LEFT:       return "ARROW_LEFT";
        case KEY_RIGHT:      return "ARROW_RIGHT";
        default:             return nullptr;
    }
}

void print_usage(const char* prog) {
    std::fprintf(stderr, "Usage: %s /dev/input/eventX\n", prog);
    std::fprintf(stderr, "Поради для пошуку вашої клавіатури:\n");
    std::fprintf(stderr, "  cat /proc/bus/input/devices\n");
    std::fprintf(stderr, "  ls -l /dev/input/by-id/ | grep -i kbd\n");
}

} // namespace kb_sniffer

int main(int argc, char* argv[]) {
    if (argc != 2) {
        kb_sniffer::print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    const std::string dev_path = argv[1];

    int fd = open(dev_path.c_str(), O_RDONLY);
    if (fd < 0) {
        std::fprintf(stderr, "[error] Не вдалося відкрити %s: %s\n",
                     dev_path.c_str(), std::strerror(errno));
        std::fprintf(stderr, "[hint] Запустіть через sudo: sudo %s %s\n",
                     argv[0], dev_path.c_str());
        return EXIT_FAILURE;
    }

    std::printf("[ok]   Підключено до пристрою вводу: %s\n", dev_path.c_str());
    std::printf("[info] Перехоплення натискань клавіш активне.\n");
    std::printf("[info] Друкуйте в будь-якому вікні — події з'являться тут.\n");
    std::printf("--------------------------------------------------------\n");
    std::fflush(stdout);

    input_event packet;
    unsigned long counter = 0;

    while (true) {
        ssize_t n = read(fd, &packet, sizeof(packet));
        if (n != static_cast<ssize_t>(sizeof(packet))) {
            std::fprintf(stderr, "[warn] Некоректний обсяг даних від пристрою, зупиняємось.\n");
            break;
        }

        // EV_KEY з value==1 означає натискання (2 = утримання, 0 = відпускання).
        if (packet.type == EV_KEY && packet.value == 1) {
            ++counter;
            const char* name = kb_sniffer::key_name(packet.code);
            if (name != nullptr) {
                std::printf("#%lu  keycode=%-3u  name=%s\n",
                            counter, packet.code, name);
            } else {
                std::printf("#%lu  keycode=%-3u\n", counter, packet.code);
            }
            std::fflush(stdout);
        }
    }

    close(fd);
    return EXIT_SUCCESS;
}
