#pragma once
#include <memory>

namespace tools::dbus {

/**
 *
 * @struct Message
 * @brief Структура для хранения сообщений от DBus.
 */
struct Message {
    std::string path; // 32
    std::string res;  // 32
};

class IStarter {
   public:
    IStarter() = default;
    virtual ~IStarter() = default;

    IStarter(IStarter const &) = delete;
    IStarter(IStarter &&) = delete;
    IStarter &operator=(IStarter const &) = delete;
    IStarter &operator=(IStarter &&) = delete;

    virtual IStarter &start() = 0;
    virtual IStarter &stop() = 0;
    virtual void poll() = 0;

    static std::unique_ptr<IStarter> create(std::string_view const &unit);

   private:
    static IStarter *create(std::string_view const &unit, char *error_message) noexcept;
    /**
     * @brief Максимальный размер буфера для сообщений об ошибках.
     */
    static constexpr std::size_t M_MAX_BUFFER_SIZE = 1024;
};
inline std::unique_ptr<IStarter> IStarter::create(std::string_view const &unit) {
    auto const message_error = std::make_unique<char[]>(M_MAX_BUFFER_SIZE);

    IStarter *new_object = create(unit, message_error.get());
    if (new_object == nullptr) {
        throw std::runtime_error(message_error.get());
    }

    return std::unique_ptr<IStarter>{new_object};
}

} // namespace tools::dbus
