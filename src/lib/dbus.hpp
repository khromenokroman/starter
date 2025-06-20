#pragma once

#include <condition_variable>
#include <list>
#include <string>

#include <sdbus-c++/sdbus-c++.h>
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
        Starter &start();
        Starter &stop();

        /**
         * @brief Ожидание завершения работы текущего задачи.
         *
         * Метод отслеживает сообщения DBus на предмет завершения работы текущего задания systemd-юнита.
         * В случае ошибки выбрасывается исключение.
         */
        void poll();

    private:
        std::condition_variable m_condition_variable; // 48
        std::mutex messages_mutex; // 40
        std::string const unit_name; // 32
        std::string job_path; // 32
        ::sdbus::InterfaceName const interface_name{"org.freedesktop.systemd1.Manager"}; // 32
        ::sdbus::SignalName const signal_name{"JobRemoved"}; // 32
        ::sdbus::ServiceName const service_name{"org.freedesktop.systemd1"}; // 32
        ::sdbus::ObjectPath const object_path{"/org/freedesktop/systemd1"}; // 32
        std::list<Message> messages; // 24
        std::unique_ptr<::sdbus::IConnection> system_bus; // 8
        std::unique_ptr<::sdbus::IProxy> proxy; // 8
    };

    /*
     * Эту обертку пришлось сделать потому, что systemd не умеет корректно
     * следить за тем когда реально остановился весь таргет, поэтому приходится
     * для верности останавливать все остальное
     */

    /**
     * Класс служит для централизованного управления группами systemd-сервисов, их стартом и остановкой.
     * Предоставляет возможности добавления сервисов в очередь и выполнения операций с ними.
     */
    template<typename TypeData = std::string>
    class MassProcess {
    public:
        /**
         * @enum TYPE_PROCESS
         * @brief Тип операции, применяемой к сервисам.
         */
        enum class TYPE_PROCESS { STOP, RUN };
        /**
         * @brief Конструктор по умолчанию, для вне контекстных сервисов.
         */
        MassProcess() = default;
        /**
         * @brief Конструктор с указанием контекста.
         * @param context_name Имя контекста, добавляемое к названиям сервисов.
         */
        explicit MassProcess(std::string_view context_name) : m_context_name(context_name) {}
        ~MassProcess() = default;

        MassProcess(MassProcess const &) = delete;
        MassProcess(MassProcess &&) = delete;
        MassProcess &operator=(MassProcess const &) = delete;
        MassProcess &operator=(MassProcess &&) = delete;

        /**
         * @brief Добавляет сервисы в очередь с использованием имени контекста, если он задан.
         *
         * Если контекст задан, то в конце имени сервиса добавляется суффикс `@<context_name>.service`.
         * В противном случае добавляется `.service`.
         *
         * @tparam Args Типы аргументов.
         * @param args Имена сервисов для добавления.
         */
        template<typename... Args>
            requires(std::is_constructible_v<TypeData, std::decay_t<Args>> && ...)
        void add_queue_default(Args &&...args) {
            if (m_context_name.has_value()) {
                (fill_vector(std::string(std::forward<Args>(args))
                                     .append("@")
                                     .append(m_context_name.value())
                                     .append(".service")),
                 ...);
            } else {
                (fill_vector(std::string(std::forward<Args>(args)).append(".service")), ...);
            }
        }
        /**
         * @brief Добавляет сервисы в очередь без изменений.
         *
         * В отличие от add_queue_default, передаваемые имена сервисов остаются в неизменном виде.
         *
         * @tparam Args Типы аргументов.
         * @param args Имена сервисов для добавления.
         */
        template<typename... Args>
            requires(std::is_constructible_v<TypeData, std::decay_t<Args>> && ...)
        void add_queue_custom(Args &&...args) {
            (fill_vector(std::forward<Args>(args)), ...);
        }

        /**
         * @brief Запуск сервисов из очереди.
         *
         * Использует ngfw::engine::configurators::Starter для выполнения операции.
         * Если не удалось запустить один или несколько сервисов, выбрасывается исключение std::runtime_error.
         */
        void start() { process(TYPE_PROCESS::RUN); }
        /**
         * @brief Остановка сервисов из очереди.
         *
         * Использует ngfw::engine::configurators::Starter для выполнения операции.
         * Если не удалось остановить один или несколько сервисов, выбрасывается исключение std::runtime_error.
         */
        void stop() { process(TYPE_PROCESS::STOP); }

    private:
        /**
         * @brief Внутренний метод для обработки запуска или остановки сервисов.
         * @param type Тип операции (запуск или остановка).
         */
        void process(TYPE_PROCESS type) {
            std::string error_unit;
            std::ranges::for_each(units, [&error_unit, type](auto const &unit) {
                try {
                    if (type == TYPE_PROCESS::STOP) {
                        Starter(unit).stop().poll();
                    } else {
                        Starter(unit).start().poll();
                    }
                } catch (std::exception const &ex) {
                    error_unit.append(unit).append(": ").append(ex.what()).append("\n");
                }
            });
            if (!error_unit.empty()) {
                /*
                 * удалим "\n" в конце
                 */
                error_unit.pop_back();
                if (type == TYPE_PROCESS::STOP) {
                    throw std::runtime_error("while stop unit:\n" + error_unit);
                }
                throw std::runtime_error("while start unit:\n" + error_unit);
            }
        }
        /**
         * @brief Добавляет имя сервиса в очередь.
         *
         * @param unit Имя сервиса.
         */
        void fill_vector(std::string_view &&unit) {
            if constexpr (std::is_same_v<TypeData, std::string>) {
                units.emplace_back(unit);
            } else {
                units.emplace_back(TypeData{unit});
            }
        }

        std::optional<TypeData> m_context_name;
        std::vector<TypeData> units; // 24
    };

}; // namespace tools::dbus
