#include <cstddef>
#include <cstdint>
#include <cstdio>
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

#ifdef DEBUG
    #define DEBUG_SYSTEM 1
#endif

#ifdef DEBUG_SYSTEM
    #include <iostream>
#endif


// Stores the members that are useful to store
// from stat result
struct StatResult {
   // filesize in bytes
   std::uint64_t size;

   // The file owner
   std::string owner;

   // does not return from stat
   std::string filename;

};

// Returns the information about a directory and all its subdirectories/files
static std::vector<struct StatResult *> * get_directory_information(std::string name);
static std::vector<struct StatResult *> * get_information(std::string name);

/**
 * Returns the owner(login) of the file
*/
static std::string get_owner_name(uid_t owner);


std::vector<std::string *> * Termsequel::System::execute(Termsequel::Command *command) {

   // convert the raw input from stat to a beautiful input, considering user column order
   const auto stat_array = get_information(command->target);
   const auto rows = new std::vector<std::string *>;

   // output rule. Values must be padded
   // Starts with the default column name
   std::size_t bigger_filename = strlen("Filename");
   std::size_t bigger_column = strlen("Size");
   std::size_t bigger_owner = strlen("Owner");

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
         if ( column == COLUMN_TYPE::OWNER ) {
            if ( stat_element->owner.size() > bigger_owner ) {
               bigger_owner = stat_element->owner.size();
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
      if ( column == COLUMN_TYPE::OWNER ) {
         header->append(" Owner");
         header->insert(header->end(), bigger_owner - strlen("Owner"), ' ');
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
         if ( column == COLUMN_TYPE::OWNER ) {
            string->push_back(' ');
            string->append(stat_element->owner);
            string->insert(string->end(), bigger_owner - stat_element->owner.size(), ' ');
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
      #ifdef DEBUG_SYSTEM
         std::cerr << "Could not stat the file: " << filename << std::endl;
      #endif
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
      stat_value->owner = get_owner_name(stat_buffer.st_uid);
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

static std::string get_owner_name(uid_t owner) {
   constexpr std::uint32_t buffer_size = 30;
   char buffer[buffer_size];
   std::string output;

   std::snprintf(buffer, buffer_size, "id --name --user %d", owner);

   FILE *file = popen(buffer, "r");
   if (file) {
      output.reserve(buffer_size);
      std::fread(buffer, sizeof(buffer), buffer_size - 1, file);
      output.append(buffer);

      // id returns a new line. We must remove it.
      output.erase(output.find_first_of("\n"));
      pclose(file);
      return output;
   }
   output.append("Could not get owner ...");
   return output;
}