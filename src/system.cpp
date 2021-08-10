#include <cstddef>
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

   // output rule. Values must be padded
   // Starts with the default column name
   std::size_t bigger_filename = strlen("Filename");
   std::size_t bigger_column = strlen("Size");

   for (const auto stat_element : *stat_array ) {
      for (const auto column: command->columns) {
         if ( column == COLUMN_TYPE::FILENAME ) {
               if ( stat_element->filename.size() > bigger_filename ) {
                  bigger_filename = stat_element->filename.size();
               }
         }
         if ( column == COLUMN_TYPE::FILESIZE ) {
               std::string number_string = std::to_string(stat_element->size);
               if (number_string.size() > bigger_column) {
                  bigger_column = number_string.size();
               }
         }
      }
   }

   // Creates the ouput
   // Should be something like:
   // Filename | Size
   // name       10
   // name220  | 100

   auto header = new std::string;
   for (const auto column: command->columns) {
      if ( column == COLUMN_TYPE::FILENAME ) {
         header->append(" Filename");
         header->insert(header->end(), bigger_filename - strlen("Filename"), ' ');
      }
      if ( column == COLUMN_TYPE::FILESIZE ) {
         header->append(" Size");
         header->insert(header->end(), bigger_column - strlen("Size"), ' ');
      }
   }
   rows->push_back(header);

   // Should use MOVE
   header = nullptr;

   for (const auto stat_element : *stat_array ) {
      auto string = new std::string;
      for (const auto column: command->columns) {
         if ( column == COLUMN_TYPE::FILENAME ) {
               string->push_back(' ');
               string->append(stat_element->filename);
               string->insert(string->end(), bigger_filename - stat_element->filename.size(), ' ');
         }
         if ( column == COLUMN_TYPE::FILESIZE ) {
               std::string column_value = std::to_string(stat_element->size);
               string->push_back(' ');
               string->append(column_value);
               string->insert(string->end(), bigger_column - column_value.size() , ' ');
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