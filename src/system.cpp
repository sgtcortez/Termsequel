#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
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

   // the deepth level of the file
   std::uint16_t level;

   // filesize in bytes
   std::uint64_t size;

   // The file owner
   std::string owner;

   // does not return from stat
   std::string filename;

   // the type of the file
   std::string file_type;

};

union Comparasion {
   std::uint64_t integer_value;
   const char *string_value;
};

// Returns the information about a directory and all its subdirectories/files
static std::vector<struct StatResult *> * get_directory_information(
   std::string name,
   Termsequel::ConditionList *conditions,
   std::uint64_t current_level
);
static std::vector<struct StatResult *> * get_information(
   std::string name,
   Termsequel::ConditionList *conditions,
   std::uint64_t current_level
);

// Checks if the value should return to the user
static bool should_return(
   struct StatResult *row,
   struct Termsequel::ConditionList *condition_list
);

// Checks if the shoud enter the directory
// Checks only for the level condition
static bool should_go_recursive(
   std::int32_t current_level,
   struct Termsequel::ConditionList *condition_list
);

/**
 * Returns the owner(login) of the file
*/
static std::string get_owner_name(uid_t owner);


std::vector<std::string *> * Termsequel::System::execute(Termsequel::Command *command) {

   const auto stat_array = get_information(command->target, command->conditions, 0);

   // convert the raw input from stat to a beautiful input, considering user column order
   const auto rows = new std::vector<std::string *>;

   // output rule. Values must be padded
   // Starts with the default column name
   std::size_t bigger_filename = strlen("Filename");
   std::size_t bigger_column = strlen("Size");
   std::size_t bigger_owner = strlen("Owner");
   std::size_t bigger_level   = strlen("Level");
   std::size_t bigger_file_type = strlen("File Type");

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
         if ( column == COLUMN_TYPE::LEVEL ) {
               std::string number_string = std::to_string(stat_element->level);
            if ( number_string.size() > bigger_level ) {
               bigger_level = number_string.size();
            }
         }
         if ( column == COLUMN_TYPE::FILE_TYPE ) {
            if (stat_element->file_type.size() > bigger_file_type) {
               bigger_file_type = stat_element->file_type.size();
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
      if ( column == COLUMN_TYPE::LEVEL ) {
         header->append(" Level");
         header->insert(header->end(), bigger_level - strlen("Level"), ' ');
      }
      if ( column == COLUMN_TYPE::FILE_TYPE ) {
         header->append(" File Type");
         header->insert(header->end(), bigger_file_type - strlen("File Type"), ' ');
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
         if ( column == COLUMN_TYPE::LEVEL ) {
               std::string column_value = std::to_string(stat_element->level);
               string->push_back(' ');
               string->append(column_value);
               string->insert(string->end(), bigger_column - column_value.size() , ' ');
         }
         if ( column == COLUMN_TYPE::FILE_TYPE ) {
            string->push_back(' ');
            string->append(stat_element->file_type);
            string->insert(string->end(), bigger_file_type - stat_element->file_type.size(), ' ');
         }
      }
      rows->push_back(string);
      delete stat_element;
   }
   delete stat_array;
   return rows;
};


static std::vector<struct StatResult *> * get_information(
   std::string name,
   Termsequel::ConditionList *conditions,
   std::uint64_t current_level
){
    // check if file is directory, if so, iterate over directory(might go recursively)
    // otherwise, just return information about the specific file

   struct stat stat_buffer;

   if ( stat(name.c_str(), &stat_buffer ) != 0 ) {
      // error
      // Could not stat
      // returns empty vector
      #ifdef DEBUG_SYSTEM
         std::cerr << "Could not stat the file: " << name << std::endl;
      #endif
      return new std::vector<struct StatResult *>;
   }


   if ( stat_buffer.st_mode & S_IFDIR ) {
      // directory
      // goes recursively
      if ( should_go_recursive(current_level, conditions) ) {
         return get_directory_information(name, conditions, current_level);
      } else {
         // didnt match the level criteria, will not go recursively
         return new std::vector<struct StatResult *>;
      }

   }

   // Store the stat result in the heap
   auto stat_value = new struct StatResult;
   if (name.find_first_of("/") != std::string::npos){
      stat_value->filename = name.substr(name.find_last_of("/") + 1);
   } else {
      stat_value->filename = name;
   }
   stat_value->size = stat_buffer.st_size;
   stat_value->owner = get_owner_name(stat_buffer.st_uid);
   stat_value->level = current_level;

   // S_IFMT -> bit mask for the file type bit field
   switch (stat_buffer.st_mode & S_IFMT) {
      case S_IFSOCK:
         stat_value->file_type = "SOCKET";
         break;
      case S_IFLNK:
         stat_value->file_type = "LINK";
         break;
      case S_IFREG:
         stat_value->file_type = "REGULAR";
         break;
      case S_IFBLK:
         stat_value->file_type = "BLOCK";
         break;
      case S_IFCHR:
         stat_value->file_type = "CHARACTER";
         break;
      case S_IFIFO:
         stat_value->file_type = "FIFO";
         break;
      default:
         break;
   }
   auto vector = new std::vector<struct StatResult *>;
   if (should_return(stat_value, conditions)) {
      vector->push_back(stat_value);
   } else {
      delete stat_value;
   }
   return vector;

}

static std::vector<struct StatResult *> * get_directory_information(
   std::string name,
   Termsequel::ConditionList *conditions,
   std::uint64_t current_level
) {

   DIR *directory = opendir(name.c_str());
   auto vector = new std::vector<struct StatResult *>;
   struct dirent *directory_entry = nullptr;
   if (directory == nullptr) {
      // could not open the directory, might be because of permissions
      // so, just returns an empty vector
      return vector;
   }
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

      // since, it will go recursively, we increment the actual level
      auto info_vector = get_information(relative_path, conditions, current_level + 1);
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

static bool should_return(
   struct StatResult *row,
   struct Termsequel::ConditionList *condition_list
) {

   bool ok = true;
   if (condition_list) {

      // stores the result of the comparasions
      std::vector<bool> comparasion;


      // There is a need to use c style loops, because we need to get the logical
      // operator based on the current index
      for (auto index = 0UL; index < condition_list->conditions.size(); index++) {
         bool current;
         const auto condition = condition_list->conditions[index];
         union Comparasion compare_value;
         bool compare_string = true;
         switch (condition->column) {
            case Termsequel::COLUMN_TYPE::FILENAME:
               compare_value.string_value = row->filename.c_str();
               break;
            case Termsequel::COLUMN_TYPE::FILESIZE:
               compare_value.integer_value = row->size;
               compare_string = false;
               break;
            case Termsequel::COLUMN_TYPE::OWNER:
               compare_value.string_value = row->owner.c_str();
               break;
            case Termsequel::COLUMN_TYPE::LEVEL:
               compare_value.integer_value = row->level;
               compare_string = false;
               break;
            case Termsequel::COLUMN_TYPE::FILE_TYPE:
               compare_value.string_value = row->file_type.c_str();
               break;
         }

         switch (condition->operator_value) {
            case Termsequel::Operator::EQUAL:
               if (!compare_string) {
                  // compare integer
                  std::uint64_t condition_value = std::atol(condition->value.c_str());
                  current = compare_value.integer_value == condition_value;
               } else {
                  // compare string
                  current = std::strncmp(compare_value.string_value, condition->value.c_str(), condition->value.size()) == 0;
               }
               break;
            case Termsequel::Operator::STARTS_WITH:
               if(compare_string) {
                  if ( condition->value.size() > strlen(compare_value.string_value) ) {
                     current = false;
                  } else {
                     current = std::strncmp(compare_value.string_value, condition->value.c_str(), condition->value.size()) == 0;
                  }
               } else {
                  // tries to compare integer with starts with
                  // should throw invalid syntax, but, this will not be so easy
                  current = false;
               }
               break;
            case Termsequel::Operator::ENDS_WITH:
               if(compare_string) {
                  if ( condition->value.size() > strlen(compare_value.string_value) ) {
                     current = false;
                  } else {
                     std::string tmp = compare_value.string_value;
                     current = tmp.compare(tmp.size() - condition->value.size(), condition->value.size(), condition->value) == 0;
                  }
               } else {
                  // tries to compare integer with starts with
                  // should throw invalid syntax, but, this will not be so easy to do
                  current = false;
               }
               break;
            case Termsequel::Operator::CONTAINS:
               if(compare_string) {
                  if ( condition->value.size() > strlen(compare_value.string_value) ) {
                     current = false;
                  } else {
                     current = std::strstr(compare_value.string_value, condition->value.c_str()) != nullptr;
                  }
               } else {
                  // tries to compare integer with starts with
                  // should throw invalid syntax, but, this will not be so easy
                  current = false;
               }
               break;
            case Termsequel::Operator::BIGGER:
               if (compare_string) {
                  current = std::strcmp(compare_value.string_value, condition->value.c_str()) > 0;
               } else {
                  std::uint64_t condition_value = std::atol(condition->value.c_str());
                  current = compare_value.integer_value > condition_value;
               }
               break;
            case Termsequel::Operator::LESS:
               if (compare_string) {
                  current = std::strcmp(compare_value.string_value, condition->value.c_str()) < 0;
               } else {
                  std::uint64_t condition_value = std::atol(condition->value.c_str());
                  current = compare_value.integer_value < condition_value;
               }
               break;
            case Termsequel::Operator::BIGGER_OR_EQUAL:
               if (compare_string) {
                  current = std::strcmp(compare_value.string_value, condition->value.c_str()) >= 0;
               } else {
                  std::uint64_t condition_value = std::atol(condition->value.c_str());
                  current = compare_value.integer_value >= condition_value;
               }
               break;
            case Termsequel::Operator::LESS_OR_EQUAL:
               if (compare_string) {
                  current = std::strcmp(compare_value.string_value, condition->value.c_str()) <= 0;
               } else {
                  std::uint64_t condition_value = std::atol(condition->value.c_str());
                  current = compare_value.integer_value <= condition_value;
               }
               break;
         }
         comparasion.push_back(current);
      }

      // just one comparasion, this means that there are no logical operators
      if (comparasion.size() == 1) return comparasion[0];

      // compares the results and yelds the boolean result
      for (auto index = 1UL; index < condition_list->conditions.size(); index++) {

         bool left = comparasion[index - 1];
         bool right = comparasion[index];
         ok = (left && right) || ( condition_list->operators[index - 1] == Termsequel::LogicalOperator::OR && (left || right));
         if (!ok) break;
      }
   }


   // returns the conditions value
   return ok;
}

static bool should_go_recursive(
   std::int32_t current_level,
   struct Termsequel::ConditionList *condition_list
) {
   if (!condition_list) return true;

   for (auto index = 0UL; index < condition_list->conditions.size(); index++) {
      const auto condition = condition_list->conditions[index];
      switch (condition->column) {
         case Termsequel::COLUMN_TYPE::LEVEL:
            if (condition->operator_value == Termsequel::Operator::LESS) {
               std::int64_t value = std::atoll(condition->value.c_str());
               return current_level < value;
            } else if ( condition->operator_value == Termsequel::Operator::LESS_OR_EQUAL ) {
               std::int64_t value = std::atoll(condition->value.c_str());
               return current_level <= value;
            } else if ( condition->operator_value == Termsequel::Operator::EQUAL ) {
               std::int64_t value = std::atoll(condition->value.c_str());
               return current_level <= value;
            } else {
               // should continue to go recursively
               return true;
            }
            break;
         default:
            break;
      }
   }
   return true;
}