#include <cstdint>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>

#include <vector>

#include "system.hpp"

namespace Termsequel {

    static constexpr const std::uint16_t DIRECTORY_TYPE    = S_IFDIR;
    static constexpr const std::uint16_t REGULAR_FILE_TYPE = S_IFREG; 

};


static std::vector<Termsequel::ResultRow *> * get_directory_information(std::string name);


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
        // Could not stat
        return new std::vector<ResultRow *>;
    } 

    if ( stat_buffer.st_mode & DIRECTORY_TYPE ) {
        // directory 
        return get_directory_information(filename);
    } else if ( stat_buffer.st_mode & REGULAR_FILE_TYPE ) {
        // Regular file
        auto row = new ResultRow(stat_buffer, filename);
        auto vector = new std::vector<ResultRow *>;
        vector->push_back(row);
        return vector;
    }
    
    // unsupported type
    return new std::vector<ResultRow *>;
}

static std::vector<Termsequel::ResultRow *> * get_directory_information(std::string name) {

    DIR *directory = opendir(name.c_str());
    struct dirent *directory_entry = nullptr;
    if (directory == nullptr) return nullptr;
    auto vector = new std::vector<Termsequel::ResultRow *>;
    while(true) {
        directory_entry = readdir(directory);
        if ( !(directory_entry))  {
            // reached the end
           break; 
        }
        // Ignore the pseudo directories    
        if (directory_entry->d_name[0] == '.' && ( directory_entry->d_name[1] == '.' || directory_entry->d_name[1] == '\0' )) continue;

        // We need to pass the absolute path or relative path to stat.  
        // or, use the fstat function. Then, we can pass only the directory fd and the file name
        std::string relative_path = name + "/" + directory_entry->d_name;

        auto info_vector = Termsequel::System::get_information(relative_path);
        for ( auto element : *info_vector) {
            vector->push_back(element);
        }
        delete info_vector;
    }
    closedir(directory);
    return vector;
}