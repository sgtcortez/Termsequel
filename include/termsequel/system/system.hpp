#ifndef __TERMSEQUEL_SYSTEM__
#define __TERMSEQUEL_SYSTEM__

/**
 * This class responsibility is to fetch the information from the operating
 * system
 */

#include <functional>
#include <string>

namespace termsequel {
namespace system {
struct FileInformation
{
    std::uint64_t size;
    std::string   name;
    std::string   relative_path;
    std::string   creation;
    struct
    {
        std::uint64_t id;
        std::string   name;
    } owner;
};

enum class Action
{
    INFORMATION = 1,
    OPEN        = 2,
    CLOSE       = 3,

};

using on_file_callback = std::function<void ( FileInformation )>;
using io_system        = void *;

class System {
  private:
    std::int32_t     interested_fields_mask;
    io_system        poll;
    on_file_callback callback;

  public:
    System ( on_file_callback callback );
    ~System ();

    /**
     * List the content of this location
     * @param source the location of the file/directory.
     * This function, will push events to the internal IO system.
     */
    void query ( const std::string &source );

  private:
    /**
     * This function handles the IO(pushes new events, poll events) and so on
     */
    void handle_io ();
};

}; // namespace system
}; // namespace termsequel

#endif