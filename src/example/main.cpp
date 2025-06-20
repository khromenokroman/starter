#include <iostream>
#include <starter/starter.hpp>

int main() {
    try {
        auto proxy = ::tools::dbus::IStarter::create("chrony.service");

        proxy->stop();
        proxy->poll();
        std::cout << "Stop" << std::endl;

        proxy->start();
        proxy->poll();
        std::cout << "Start" << std::endl;

    } catch (std::exception const &ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
    }
    return 0;
}
