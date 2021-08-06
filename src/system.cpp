#include <cstdint>
#include <cstring>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <streambuf>
#include <sstream>


#include <vector>

#include "system.hpp"

// Stores the members that are useful to store
// from stat result
struct StatResult {        
    std::uint64_t size;
    // does not return from stat
    std::string filename;
};

// Returns the information about a directory and all its subdirectories/files
static std::vector<struct StatResult *> * get_directory_information(std::string name);
static std::vector<struct StatResult *> * get_information(std::string name);


std::vector<std::string *> * Termsequel::System::execute(Termsequel::Command *command) {

    // convert the raw input from stat to a beautiful input, considering user column order
    const auto stat_array = get_information(command->target);
    const auto rows = new std::vector<std::string *>;

    for (const auto stat_element : *stat_array ) {
        auto string = new std::string;
        for (const auto column: command->columns) {
            if ( column == FILENAME ) {
                string->append("Filename: ");
                string->append(stat_element->filename); 
                string->append(" ");
            } 
            if ( column == FILESIZE ) {
                string->append("Filesize: ");
                string->append(std::to_string(stat_element->size)); 
                string->append(" ");            
            }
        }
        rows->push_back(string);
        delete stat_element;
    }
    delete stat_array;
    return rows;
};


static std::vector<struct StatResult *> * get_information(std::string filename) {

    // check if file is directory, if so, iterate over directory(might go recursively)
    // otherwise, just return information about the specific file

    struct stat stat_buffer;

    if ( stat(filename.c_str(), &stat_buffer ) != 0 ) {
        // error
        // Could not stat
        // returns empty vector
        return new std::vector<struct StatResult *>;
    } 


    if ( stat_buffer.st_mode & S_IFDIR ) {
        // directory 
        return get_directory_information(filename);
    } else if ( stat_buffer.st_mode & S_IFREG ) {
        // Regular file

        // Store the stat result in the heap
        auto stat_value = new struct StatResult;
        stat_value->filename = filename;
        stat_value->size = stat_buffer.st_size;
        auto vector = new std::vector<struct StatResult *>;
        vector->push_back(stat_value);
        return vector;
    }
    
    // unsupported type, for now, returns empty vector
    return new std::vector<struct StatResult *>;
}

static std::vector<struct StatResult *> * get_directory_information(std::string name) {

    DIR *directory = opendir(name.c_str());
    struct dirent *directory_entry = nullptr;
    if (directory == nullptr) return nullptr;
    auto vector = new std::vector<struct StatResult *>;
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

        auto info_vector = get_information(relative_path);
        for ( auto element : *info_vector) {
            vector->push_back(element);
        }
        // deletes the old vector
        delete info_vector;
    }
    closedir(directory);
    return vector;
}