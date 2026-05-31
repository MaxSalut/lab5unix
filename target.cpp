
#include <cstdio>
#include <iostream>
#include <unistd.h>

namespace target_app {

void print_banner(int* data_ptr) {
    std::printf("=== Target Application Started ===\n");
    std::printf("Process identifier (PID) : %d\n", getpid());
    std::printf("Variable located at addr : %p\n", static_cast<void*>(data_ptr));
    std::printf("===================================\n");
    std::printf("[info] Передайте PID та адресу спостерігачу та змінюйте значення.\n\n");
}

} 

int main() {
    int tracked_number = 100;

    target_app::print_banner(&tracked_number);

    int user_input = 0;
    while (true) {
        std::cout << "[input] Введіть ціле число (-1 для виходу): ";

        if (!(std::cin >> user_input)) {
            std::cout << "[warn] Некоректний ввід, завершуємо роботу." << std::endl;
            break;
        }

        if (user_input == -1) {
            std::cout << "[info] Отримано сигнал виходу. Завершення." << std::endl;
            break;
        }

        tracked_number = user_input;
        std::cout << "[ok] Значення збережено у пам'яті процесу." << std::endl;
    }

    return 0;
}
