#include <cstdint>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

#include <vector>

#include "system.hpp"

namespace Termsequel {

    static constexpr const std::uint16_t DIRECTORY_TYPE    = S_IFDIR;
    static constexpr const std::uint16_t REGULAR_FILE_TYPE = S_IFREG; 

};

Termsequel::ResultRow::ResultRow(
    struct stat &stat_buffer,
    std::string name
) {
    this->size = stat_buffer.st_size;
    this->name = name;
}

std::uint64_t Termsequel::ResultRow::get_size() {
    return this->size;
}

std::string Termsequel::ResultRow::get_name() {
    return this->name;
}

std::vector<Termsequel::ResultRow *> * Termsequel::System::get_information(std::string filename) {

    // check if file is directory, if so, iterate over directory(might go recursively)
    // otherwise, just return information about the specific file

    struct stat stat_buffer;

    if ( stat(filename.c_str(), &stat_buffer ) != 0 ) {
        // error
        return nullptr;
    } 

    if ( stat_buffer.st_mode & DIRECTORY_TYPE ) {
        // directory 
        // TODO
        return nullptr;
    } else if ( stat_buffer.st_mode & REGULAR_FILE_TYPE ) {
        // Regular file
        auto row = new ResultRow(stat_buffer, filename);
        auto vector = new std::vector<ResultRow *>;
        vector->push_back(row);
        return vector;
    }
    
    // unsupported type
    return nullptr;
}