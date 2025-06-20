#include "dbus.hpp"

namespace tools::dbus {

    IStarter *IStarter::create(std::string_view const &unit, char *error_message) noexcept {
        try {
            auto *new_object = new Starter(unit);
            return new_object;
        } catch (std::exception const &ex) {
            ::snprintf(error_message, M_MAX_BUFFER_SIZE, "Cannot create new object %s", ex.what());
            return nullptr;
        }
    }

    Starter::Starter(std::string_view const &unit) :
        unit_name{unit}, system_bus{::sdbus::createSystemBusConnection()},
        proxy(::sdbus::createProxy(*system_bus, service_name, object_path)) {
        proxy->registerSignalHandler(
                interface_name, signal_name,
                std::function<void(sdbus::Signal)>([this](::sdbus::Signal signal) { this->signal_handler(signal); }));
        system_bus->enterEventLoopAsync();
    }
    void Starter::signal_handler(sdbus::Signal &signal) {
        uint32_t val;
        std::string unit;
        std::string str2;
        ::sdbus::ObjectPath path;
        signal >> val >> path >> unit >> str2;

        if (unit != unit_name) {
            return;
        }

        {
            std::lock_guard<std::mutex> msg_lg(messages_mutex);
            messages.push_back(Message{path, str2});
        }

        m_condition_variable.notify_one();
    }
    Starter &Starter::start() {
        ::sdbus::ObjectPath path;
        ::sdbus::createProxy(service_name, object_path)
                ->callMethod("StartUnit")
                .onInterface("org.freedesktop.systemd1.Manager")
                .withArguments(unit_name, "replace")
                .storeResultsTo(path);
        job_path = path;
        return *this;
    }
    Starter &Starter::stop() {
        ::sdbus::ObjectPath path;
        ::sdbus::createProxy(service_name, object_path)
                ->callMethod("StopUnit")
                .onInterface("org.freedesktop.systemd1.Manager")
                .withArguments(unit_name, "replace")
                .storeResultsTo(path);
        job_path = path;
        return *this;
    }
    void Starter::poll() {
        std::unique_lock<std::mutex> msg_lock(messages_mutex);
        while (true) {
            m_condition_variable.wait(msg_lock, [this] { return !messages.empty(); });

            while (!messages.empty()) {
                auto &message = messages.front();
                if (message.path == job_path) {
                    if (message.res == "done") {
                        return;
                    } else {
                        throw std::runtime_error(std::string(unit_name).append(" ").append(message.res));
                    }
                }
                messages.pop_front();
            }
        }
    }
} // namespace tools::dbus
