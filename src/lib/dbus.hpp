#pragma once

#include <sdbus-c++/sdbus-c++.h>

#include <condition_variable>
#include <list>
#include <string>

#include "starter/starter.hpp"

namespace tools::dbus {
/**
 * @class Starter
 * @brief Класс для запуска и отслеживания состояния systemd-юнитов через DBus.
 *
 * Класс предназначен для взаимодействия с systemd через DBus.
 * Позволяет запускать systemd-юнит, отслеживать события и проверять его статус.
 */
class Starter final : public IStarter {
   public:
    /**
     * @brief Конструктор, инициализирующий имя systemd-юнита.
     *
     * @param unit Имя systemd-юнита, с которым будет работать объект.
     */
    explicit Starter(std::string_view const &unit);
    ~Starter() override = default;

    Starter(Starter const &) = delete;
    Starter &operator=(Starter const &) = delete;
    Starter(Starter &&) = delete;
    Starter &operator=(Starter &&) = delete;
    /**
     * @brief Обработчик сигналов DBus.
     *
     * @param signal Сигнал, полученный от DBus.
     */
    void signal_handler(::sdbus::Signal &signal);
    /**
     * @brief Запуск systemd-юнита.
     *
     * Метод вызывает systemd для запуска указанного юнита и сохраняет путь до задания.
     *
     * @return Ссылка на текущий объект Starter для вызовов по цепочке.
     */
    IStarter &start() override;
    IStarter &stop() override;

    /**
     * @brief Ожидание завершения работы текущего задачи.
     *
     * Метод отслеживает сообщения DBus на предмет завершения работы текущего задания systemd-юнита.
     * В случае ошибки выбрасывается исключение.
     */
    void poll() override;

   private:
    std::condition_variable m_condition_variable;                                    // 48
    std::mutex messages_mutex;                                                       // 40
    std::string const unit_name;                                                     // 32
    std::string job_path;                                                            // 32
    ::sdbus::InterfaceName const interface_name{"org.freedesktop.systemd1.Manager"}; // 32
    ::sdbus::SignalName const signal_name{"JobRemoved"};                             // 32
    ::sdbus::ServiceName const service_name{"org.freedesktop.systemd1"};             // 32
    ::sdbus::ObjectPath const object_path{"/org/freedesktop/systemd1"};              // 32
    std::list<Message> messages;                                                     // 24
    std::unique_ptr<::sdbus::IConnection> system_bus;                                // 8
    std::unique_ptr<::sdbus::IProxy> proxy;                                          // 8
};

}; // namespace tools::dbus
