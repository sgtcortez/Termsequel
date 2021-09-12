#ifdef _WIN32
   // Should we really do this?
   #undef UNICODE
#endif

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <sstream>
#include <streambuf>
#include <string>
#include <sys/types.h>
#include <vector>
#include <time.h>
#include <iostream>
#include <fcntl.h>  // for OPEN file constants ...

#ifdef __linux__

#include <dirent.h>
#include <unistd.h>
#include <pwd.h>     // getpwuid
#include <sys/syscall.h>
#include <sys/types.h>
#include <linux/stat.h>

#define IS_DIRECTORY(bitset) (bitset & S_IFDIR)
#define FILE_TYPE_MASK S_IFMT
#define DIRECTORY_FLAG S_IFDIR
#define REGULAR_FLAG S_IFREG
#define CHARACTER_FLAG S_IFCHR

#define READ_PERMISSION_OWNER    S_IRUSR
#define WRITE_PERMISSION_OWNER   S_IWUSR
#define EXECUTE_PERMISSION_OWNER S_IXUSR

#define READ_PERMISSION_GROUP    S_IRGRP
#define WRITE_PERMISSION_GROUP   S_IWGRP
#define EXECUTE_PERMISSION_GROUP S_IXGRP

#define READ_PERMISSION_OTHERS    S_IROTH
#define WRITE_PERMISSION_OTHERS   S_IWOTH
#define EXECUTE_PERMISSION_OTHERS S_IXOTH

typedef struct statx stat_buffer_t;

static constexpr std::uint32_t get_uid(stat_buffer_t &buffer) {
   return buffer.stx_uid;
}

static constexpr std::uint16_t get_mode(stat_buffer_t &buffer) {
   return buffer.stx_mode;
}

static constexpr std::uint64_t get_size(stat_buffer_t &buffer) {
   return buffer.stx_size;
}

static constexpr __time_t get_last_modification_date(stat_buffer_t &buffer) {
   return buffer.stx_mtime.tv_sec;
}

static constexpr __time_t get_creation_date(stat_buffer_t &buffer) {
   return buffer.stx_btime.tv_sec;
}

static int termsequel_stat(
   const char* filename,
   stat_buffer_t *stat_buffer
) {
   // We need to do this, because, libc wrapper for Statx came only in glibc 2.28 ...
   // but, it is available on Linux Kernel since 4.11
   return syscall(__NR_statx, AT_FDCWD, filename, AT_SYMLINK_NOFOLLOW, STATX_ALL, stat_buffer);
}


constexpr static const char FILE_SEPARATOR = '/';

#elif defined(_WIN32)

#include <io.h> // _findfirst _findnext
#include <windows.h>  // GetFullPathName
#include <sys/stat.h>

#define IS_DIRECTORY(bitset) (bitset & _S_IFDIR)
#define FILE_TYPE_MASK _S_IFMT
#define DIRECTORY_FLAG _S_IFDIR
#define REGULAR_FLAG _S_IFREG
#define CHARACTER_FLAG _S_IFCHR

#define READ_PERMISSION_OWNER    _S_IREAD
#define WRITE_PERMISSION_OWNER   _S_IWRITE
#define EXECUTE_PERMISSION_OWNER _S_IEXEC

typedef struct _stat stat_buffer_t;

static constexpr std::uint16_t get_mode(stat_buffer_t &buffer) {
   return buffer.st_mode;
}

static constexpr std::uint64_t get_size(stat_buffer_t &buffer) {
   return buffer.st_size;
}

static constexpr __time64_t get_last_modification_date(stat_buffer_t &buffer) {
   return buffer.st_mtime;
}

static constexpr __time64_t get_creation_date(stat_buffer_t &buffer) {
   return buffer.st_ctime;
}

/**
 * Obs: We do not use open here, because, we can not open a directory in Windows ...
 * This is why, we need to use this version of stat, and, use if-elif directives ...
*/

static int termsequel_stat(const char *filename, stat_buffer_t *stat_buffer) {
   return _stat(filename, stat_buffer) ;
}

constexpr static const char FILE_SEPARATOR = '\\';

#endif

#include "system.hpp"


#define BUFFER_SIZE 1024 * 4 // 4 KiB

// Used to indicate a variable ...
static constexpr char VARIABLE_SYMBOL = '$';

#ifdef DEBUG
#define DEBUG_SYSTEM 1
#endif

// Stores the members that are useful to store
// from stat result
struct StatResult {

  // the deepth level of the file
  std::uint16_t level;

  // The file permissions
  struct {
    // owner permissions
    char owner[4];
#ifdef __linux__
    // group permissions
    char group[4];

    // others permissions
    char others[4];
#endif
  } permissions;

  // filesize in bytes
  std::uint64_t size;

  // The file owner
  std::string owner;

  // does not return from stat
  std::string filename;

  // the type of the file
  std::string file_type;

   // Last modification of the file
  std::string last_modification;

  // The creation date of the file
  std::string creation_date;

   // the relative path of the file
  std::string relative_path;

   // The absolute path of the file
   std::string absolute_path;
};

union Comparasion {
  std::uint64_t integer_value;
  const char *string_value;
};

// Returns the information about a directory and all its subdirectories/files
static std::vector<struct StatResult *> * get_directory_information(
   std::string name,
   Termsequel::ConditionList *conditions,
   std::uint16_t current_level
);

static std::vector<struct StatResult *> * get_information(
   std::string name,
   Termsequel::ConditionList *conditions,
   std::uint16_t current_level
);

// Checks if the value should return to the user
static bool should_return(
   struct StatResult *row,
   struct Termsequel::ConditionList *condition_list
);

// Checks if the shoud enter the directory
// Checks only for the level condition
static bool should_go_recursive(
   std::uint16_t current_level,
   struct Termsequel::ConditionList *condition_list
);

/**
 * Returns the value from a variable from operating system
*/
static std::string get_variable_value(std::string name);

#ifdef __linux__

/**
 * Returns the owner(login) of the file
 * Windows stat result, does not return the owner and the group!
 */
static std::string get_owner_name(uid_t owner);

#endif

bool compare(
   bool left,
   bool right,
   Termsequel::LogicalOperator operator_type
);

bool Termsequel::System::execute(const Termsequel::SystemCommand *command) {

   std::string target;
   if (command->target[0] == VARIABLE_SYMBOL) {

      const auto first_slash = command->target.find_first_of(FILE_SEPARATOR);
      std::string temp_target = "";
      if (first_slash != std::string::npos) {
         temp_target = command->target.substr(0, first_slash);
      } else {
         temp_target = command->target;
      }
      target = get_variable_value(temp_target);

      if (target.size() == 0) {
         // the variable was not found, does nothing then ...
         return false;
      }
      if (first_slash != std::string::npos) {
         // must append the rest of the string
         // an input as: $HOME/Documents
         // Will become: home/user/Documents
         target = target + command->target.substr(first_slash);
      }
   } else {
      target = command->target;
   }

  const auto stat_array = get_information(target, command->conditions, 0);

  // output rule. Values must be padded
  // Starts with the default column name
  std::size_t bigger_filename = strlen("Filename");
  std::size_t bigger_column = strlen("Size");
  std::size_t bigger_owner = strlen("Owner");
  std::size_t bigger_level = strlen("Level");
  std::size_t bigger_file_type = strlen("File Type");
  std::size_t bigger_last_modification = strlen("Last Modification");
  std::size_t bigger_creation_date = strlen("Creation Date");
  std::size_t bigger_relative_path = strlen("Relative Path");
  std::size_t bigger_absolute_path = strlen("Absolute Path");

  for (const auto stat_element : *stat_array) {
    for (const auto column : command->columns) {
      if (column == COLUMN_TYPE::FILENAME) {
        if (stat_element->filename.size() > bigger_filename) {
          bigger_filename = stat_element->filename.size();
        }
      }
      if (column == COLUMN_TYPE::FILESIZE) {
        std::string number_string = std::to_string(stat_element->size);
        if (number_string.size() > bigger_column) {
          bigger_column = number_string.size();
        }
      }
      if (column == COLUMN_TYPE::OWNER) {
        if (stat_element->owner.size() > bigger_owner) {
          bigger_owner = stat_element->owner.size();
        }
      }
      if (column == COLUMN_TYPE::LEVEL) {
        std::string number_string = std::to_string(stat_element->level);
        if (number_string.size() > bigger_level) {
          bigger_level = number_string.size();
        }
      }
      if (column == COLUMN_TYPE::FILE_TYPE) {
        if (stat_element->file_type.size() > bigger_file_type) {
          bigger_file_type = stat_element->file_type.size();
        }
      }
      if (column == COLUMN_TYPE::LAST_MODIFICATION) {
         if ( stat_element->last_modification.size() > bigger_last_modification ) {
            bigger_last_modification = stat_element->last_modification.size();
         }
      }
      if ( column == COLUMN_TYPE::CREATION_DATE) {
         if ( stat_element->creation_date.size() > bigger_creation_date ) {
            bigger_creation_date = stat_element->creation_date.size();
         }
      }
      if (column == COLUMN_TYPE::RELATIVE_PATH ) {
         if ( stat_element->relative_path.size() > bigger_relative_path ) {
            bigger_relative_path = stat_element->relative_path.size();
         }
      }
      if (column == COLUMN_TYPE::ABSOLUTE_PATH) {
         if (stat_element->absolute_path.size() > bigger_absolute_path) {
            bigger_absolute_path = stat_element->absolute_path.size();
         }
      }
    }
  }

  // Creates the ouput
  // Should be something like:
  // Filename | Size
  // name       10
  // name220  | 100

   std::string header;
   for (const auto column : command->columns) {
      if (column == COLUMN_TYPE::FILENAME) {
         header.append(" Filename");
         header.insert(header.end(), bigger_filename - strlen("Filename"), ' ');
      }
      if (column == COLUMN_TYPE::FILESIZE) {
         header.append(" Size");
         header.insert(header.end(), bigger_column - strlen("Size"), ' ');
      }
      if (column == COLUMN_TYPE::OWNER) {
         header.append(" Owner");
         header.insert(header.end(), bigger_owner - strlen("Owner"), ' ');
      }
      if (column == COLUMN_TYPE::LEVEL) {
         header.append(" Level");
         header.insert(header.end(), bigger_level - strlen("Level"), ' ');
      }
      if (column == COLUMN_TYPE::FILE_TYPE) {
         header.append(" File Type");
         header.insert(header.end(), bigger_file_type - strlen("File Type"), ' ');
      }
      if (column == COLUMN_TYPE::OWNER_PERMISSIONS) {
         header.append(" Owner Permissions");
      }
#ifdef __linux__
      if (column == COLUMN_TYPE::GROUP_PERMISSIONS) {
         header.append(" Group Permissions");
      }
      if (column == COLUMN_TYPE::OTHERS_PERMISSIONS) {
         header.append(" Others Permissions");
      }
#endif
      if (column == COLUMN_TYPE::LAST_MODIFICATION) {
         header.append(" Last Modification");
         header.insert(header.end(), bigger_last_modification - strlen("Last Modification"), ' ');
      }
      if (column == COLUMN_TYPE::CREATION_DATE) {
         header.append(" Creation Date");
         header.insert(header.end(), bigger_creation_date - strlen("Creation Date"), ' ');
      }
      if ( column == COLUMN_TYPE::RELATIVE_PATH ) {
         header.append(" Relative Path");
         header.insert(header.end(), bigger_relative_path - strlen("Relative Path"), ' ');
      }
      if (column == COLUMN_TYPE::ABSOLUTE_PATH) {
         header.append(" Absolute Path");
         header.insert(header.end(), bigger_absolute_path - strlen("Absolute Path"), ' ');
      }
   }
   std::cout << header << std::endl;

   for (const auto stat_element : *stat_array) {
      std::string string;
      for (const auto column : command->columns) {
         if (column == COLUMN_TYPE::FILENAME) {
            string.push_back(' ');
            string.append(stat_element->filename);
            string.insert(string.end(), bigger_filename - stat_element->filename.size(), ' ');
         }
         if (column == COLUMN_TYPE::FILESIZE) {
            std::string column_value = std::to_string(stat_element->size);
            string.push_back(' ');
            string.append(column_value);
            string.insert(string.end(), bigger_column - column_value.size(), ' ');
         }
         if (column == COLUMN_TYPE::OWNER) {
            string.push_back(' ');
            string.append(stat_element->owner);
            string.insert(string.end(), bigger_owner - stat_element->owner.size(), ' ');
         }
         if (column == COLUMN_TYPE::LEVEL) {
            std::string column_value = std::to_string(stat_element->level);
            string.push_back(' ');
            string.append(column_value);
            string.insert(string.end(), bigger_column - column_value.size(), ' ');
         }
         if (column == COLUMN_TYPE::FILE_TYPE) {
            string.push_back(' ');
            string.append(stat_element->file_type);
            string.insert(string.end(), bigger_file_type - stat_element->file_type.size(), ' ');
         }
         if (column == COLUMN_TYPE::OWNER_PERMISSIONS) {
            string.push_back(' ');
            string.append(stat_element->permissions.owner);
            string.insert(string.end(), strlen("Owner Permissions") - 3, ' ');
         }
#ifdef __linux__
         if (column == COLUMN_TYPE::GROUP_PERMISSIONS) {
            string.push_back(' ');
            string.append(stat_element->permissions.group);
            string.insert(string.end(), strlen("Group Permissions") - 3, ' ');
         }
         if (column == COLUMN_TYPE::OTHERS_PERMISSIONS) {
            string.push_back(' ');
            string.append(stat_element->permissions.others);
            string.insert(string.end(), strlen("Others Permissions") - 3, ' ');
         }
#endif
         if (column == COLUMN_TYPE::LAST_MODIFICATION) {
            string.push_back(' ');
            string.append(stat_element->last_modification);
            string.insert(string.end(), bigger_last_modification - stat_element->last_modification.size(), ' ');
         }
         if (column == COLUMN_TYPE::CREATION_DATE) {
            string.push_back(' ');
            string.append(stat_element->creation_date);
            string.insert(string.end(), bigger_creation_date - stat_element->creation_date.size(), ' ');
         }
         if (column == COLUMN_TYPE::RELATIVE_PATH) {
            string.push_back(' ');
            string.append(stat_element->relative_path);
            string.insert(string.end(), bigger_relative_path - stat_element->relative_path.size(), ' ');
         }
         if (column == COLUMN_TYPE::ABSOLUTE_PATH) {
            string.push_back(' ');
            string.append(stat_element->absolute_path);
            string.insert(string.end(), bigger_absolute_path - stat_element->absolute_path.size(), ' ');
         }
      }
      std::cout << string << std::endl;
      delete stat_element;
   }
   delete stat_array;
   return true;
}

static std::vector<struct StatResult *> * get_information(
   std::string name,
   Termsequel::ConditionList *conditions,
   std::uint16_t current_level
) {

   stat_buffer_t stat_buffer;

   if (termsequel_stat(name.c_str(), &stat_buffer) != 0) {
      return new std::vector<struct StatResult *>;
   }


  // check if file is directory, if so, iterate over directory(might go
  // recursively) otherwise, just return information about the specific file
  if (IS_DIRECTORY(get_mode(stat_buffer))) {
    // directory
    // goes recursively
    if (should_go_recursive(current_level, conditions)) {
      return get_directory_information(name, conditions, current_level);
    } else {
      // didnt match the level criteria, will not go recursively
      return new std::vector<struct StatResult *>;
    }
  }

  // Store the stat result in the heap
  auto stat_value = new struct StatResult;
  if (name.find_first_of(FILE_SEPARATOR) != std::string::npos) {
    stat_value->filename = name.substr(name.find_last_of(FILE_SEPARATOR) + 1);
  } else {
    stat_value->filename = name;
  }
  stat_value->size = get_size(stat_buffer);
#ifdef __linux__
  stat_value->owner = get_owner_name(get_uid(stat_buffer));
#elif defined(_WIN32)
  stat_value->owner = "Not available";
#endif
  stat_value->level = current_level;

  // S_IFMT -> bit mask for the file type bit field
  switch (get_mode(stat_buffer) & FILE_TYPE_MASK) {
  case REGULAR_FLAG:
    stat_value->file_type = "REGULAR";
    break;
  case CHARACTER_FLAG:
    stat_value->file_type = "CHARACTER";
    break;
  default:
    stat_value->file_type = "Unknown";
    break;
  }

   stat_value->permissions.owner[0] = get_mode(stat_buffer) & READ_PERMISSION_OWNER    ? 'R' : '-';
   stat_value->permissions.owner[1] = get_mode(stat_buffer) & WRITE_PERMISSION_OWNER   ? 'W' : '-';
   stat_value->permissions.owner[2] = get_mode(stat_buffer) & EXECUTE_PERMISSION_OWNER ? 'X' : '-';
   stat_value->permissions.owner[3] = '\0';
#ifdef __linux__
   stat_value->permissions.group[0] = get_mode(stat_buffer) & READ_PERMISSION_GROUP    ? 'R' : '-';
   stat_value->permissions.group[1] = get_mode(stat_buffer) & WRITE_PERMISSION_GROUP   ? 'W' : '-';
   stat_value->permissions.group[2] = get_mode(stat_buffer) & EXECUTE_PERMISSION_GROUP ? 'X' : '-';
   stat_value->permissions.group[3] = '\0';

   stat_value->permissions.others[0] = get_mode(stat_buffer) & READ_PERMISSION_OTHERS    ? 'R' : '-';
   stat_value->permissions.others[1] = get_mode(stat_buffer) & WRITE_PERMISSION_OTHERS   ? 'W' : '-';
   stat_value->permissions.others[2] = get_mode(stat_buffer) & EXECUTE_PERMISSION_OTHERS ? 'X' : '-';
   stat_value->permissions.others[3] = '\0';
#endif

   // YYYY-MM-DD HH:MM:SS
   char buffer[20] = {0};
   const auto modification_date = get_last_modification_date(stat_buffer);
   const auto creation_date = get_creation_date(stat_buffer);

#ifdef __linux

   auto date = localtime(&(modification_date));

   std::strftime(buffer, sizeof(buffer) / sizeof(buffer[0]), "%Y-%m-%d %H:%M:%S", date);
   stat_value->last_modification = buffer;

   date = localtime(&(creation_date));

   std::strftime(buffer, sizeof(buffer) / sizeof(buffer[0]), "%Y-%m-%d %H:%M:%S", date);
   stat_value->creation_date = buffer;


#elif defined(_WIN32)
   struct tm local_time;
   errno_t error_number = localtime_s(&local_time, &(modification_date));
   if (error_number != 0) {
      // error here
      std::cerr << "Could not get the locatime. Error Number: " << error_number << std::endl;
   }
   std::strftime(buffer, sizeof(buffer) / sizeof(buffer[0]), "%Y-%m-%d %H:%M:%S", &local_time);

   stat_value->last_modification = buffer;

   error_number = localtime_s(&local_time, &(creation_date));
   if (error_number != 0) {
      // error here
      std::cerr << "Could not get the locatime. Error Number: " << error_number << std::endl;
   }
   std::strftime(buffer, sizeof(buffer) / sizeof(buffer[0]), "%Y-%m-%d %H:%M:%S", &local_time);
   stat_value->creation_date = buffer;



#endif

   stat_value->relative_path = name;

#ifdef __linux__

   char absolute_path_buffer[BUFFER_SIZE] = {0};
   if ( (realpath(name.c_str(), absolute_path_buffer)) == nullptr) {
      std::cerr << "Could not get the real path from file: " << name << std::endl;
   } else {
      stat_value->absolute_path = absolute_path_buffer;
   }

#elif defined (_WIN32)

   char absolute_path_buffer[BUFFER_SIZE] = {0};

   DWORD return_code = GetFullPathName(
      name.c_str(),
      BUFFER_SIZE,
      absolute_path_buffer,
      NULL
   );
   if (return_code == 0) {
      std::cerr << "Could not the the real path from file: " << name << std::endl;
   } else {
      stat_value->absolute_path = absolute_path_buffer;
   }

#endif


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
   std::uint16_t current_level
) {

#ifdef __linux__

  DIR *directory = opendir(name.c_str());
  auto vector = new std::vector<struct StatResult *>;
  struct dirent *directory_entry = nullptr;
  if (directory == nullptr) {
    // could not open the directory, might be because of permissions
    // so, just returns an empty vector
    return vector;
  }
  while (true) {
    directory_entry = readdir(directory);
    if (!(directory_entry)) {
      // reached the end
      break;
    }
    // Ignore the pseudo directories
    if (directory_entry->d_name[0] == '.' && (directory_entry->d_name[1] == '.' || directory_entry->d_name[1] == '\0')) continue;

    // We need to pass the absolute path or relative path to stat.
    // or, use the fstat function. Then, we can pass only the directory fd and
    // the file name
    std::string relative_path = name + "/" + directory_entry->d_name;

    // since, it will go recursively, we increment the actual level
    auto info_vector = get_information(relative_path, conditions, current_level + 1);
    for (auto element : *info_vector) vector->push_back(element);

    // deletes the old vector
    delete info_vector;
  }
  closedir(directory);
  return vector;

#elif defined(_WIN32)

  struct _finddata_t entry_file;
  intptr_t file_pointer;
  auto vector = new std::vector<struct StatResult *>;

  // get all the files
  std::string files_to_search = name + "\\*.*";
  if ((file_pointer = _findfirst(files_to_search.c_str(), &entry_file)) == -1L) {
    // no file found
    return vector;
  }
  do {
   // iterate over the files
   if (entry_file.name[0] == '.' && (entry_file.name[1] == '.' || entry_file.name[1] == '\0')) continue;

   std::string relative_path = name + FILE_SEPARATOR + entry_file.name;
   // since, it will go recursively, we increment the actual level

   auto info_vector = get_information(relative_path, conditions, current_level + 1);
   for (auto element : *info_vector) vector->push_back(element);

   delete info_vector;

  } while (_findnext(file_pointer, &entry_file) == 0);
  _findclose(file_pointer);

  return vector;

#endif
}

#ifdef __linux__

static std::string get_owner_name(uid_t owner) {
   struct passwd *user = getpwuid(owner);
   std::string result = user->pw_name;
   return result;
}
#endif

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
      bool current = false;
      const auto condition = condition_list->conditions[index];
      union Comparasion compare_value = {0};
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
      case Termsequel::COLUMN_TYPE::OWNER_PERMISSIONS:
         compare_value.string_value = (row->permissions.owner);
         break;
#ifdef __linux
      case Termsequel::COLUMN_TYPE::GROUP_PERMISSIONS:
         compare_value.string_value = row->permissions.group;
         break;
      case Termsequel::COLUMN_TYPE::OTHERS_PERMISSIONS:
         compare_value.string_value = row->permissions.others;
         break;
#endif
      case Termsequel::COLUMN_TYPE::LAST_MODIFICATION:
         compare_value.string_value = row->last_modification.c_str();
         break;
      case Termsequel::COLUMN_TYPE::CREATION_DATE:
         compare_value.string_value = row->creation_date.c_str();
         break;
      case Termsequel::COLUMN_TYPE::RELATIVE_PATH:
         compare_value.string_value = row->relative_path.c_str();
         break;
      case Termsequel::COLUMN_TYPE::ABSOLUTE_PATH:
         compare_value.string_value = row->absolute_path.c_str();
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
               current = std::strncmp(compare_value.string_value, condition->value.c_str(), strlen(compare_value.string_value)) == 0;
            }
            break;
         case Termsequel::Operator::NOT_EQUAL:
            if (!compare_string) {
               // compare integer
               std::uint64_t condition_value = std::atol(condition->value.c_str());
               current = compare_value.integer_value != condition_value;
            } else {
               // compare string
               if (strlen(compare_value.string_value) != condition->value.size()) {
                  current = true;
               } else {
                  current = std::strncmp(compare_value.string_value, condition->value.c_str(), strlen(compare_value.string_value)) != 0;
               }
            }
            break;
         case Termsequel::Operator::STARTS_WITH:
            if (compare_string) {
               if (condition->value.size() > strlen(compare_value.string_value)) {
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
         case Termsequel::Operator::NOT_STARTS_WITH:
            if (compare_string) {
               if (condition->value.size() > strlen(compare_value.string_value)) {
                  current = false;
               } else {
                  current = std::strncmp( compare_value.string_value, condition->value.c_str(), condition->value.size()) != 0;
               }
            } else {
               // tries to compare integer with starts with
               // should throw invalid syntax, but, this will not be so easy
               current = false;
            }
            break;
         case Termsequel::Operator::ENDS_WITH:
            if (compare_string) {
               if (condition->value.size() > strlen(compare_value.string_value)) {
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
         case Termsequel::Operator::NOT_ENDS_WITH:
            if (compare_string) {
               if (condition->value.size() > strlen(compare_value.string_value)) {
               current = false;
               } else {
                  std::string tmp = compare_value.string_value;
                  current = tmp.compare(tmp.size() - condition->value.size(), condition->value.size(), condition->value) != 0;
               }
            } else {
               // tries to compare integer with starts with
               // should throw invalid syntax, but, this will not be so easy to do
               current = false;
            }
            break;
         case Termsequel::Operator::CONTAINS:
            if (compare_string) {
               if (condition->value.size() > strlen(compare_value.string_value)) {
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
         case Termsequel::Operator::NOT_CONTAINS:
            if (compare_string) {
               if (condition->value.size() > strlen(compare_value.string_value)) {
                  current = false;
               } else {
                  current = std::strstr(compare_value.string_value, condition->value.c_str()) == nullptr;
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
      ok = compare(left, right, condition_list->operators[index - 1]);
      comparasion[index] = ok;
   }
  }

  // returns the conditions value
  return ok;
}

static bool should_go_recursive(
   std::uint16_t current_level,
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
         } else if (condition->operator_value == Termsequel::Operator::LESS_OR_EQUAL) {
            std::int64_t value = std::atoll(condition->value.c_str());
            return current_level <= value;
         } else if (condition->operator_value == Termsequel::Operator::EQUAL) {
            std::int64_t value = std::atoll(condition->value.c_str());
            return current_level <= value;
         } else {
            // should continue to go recursively
            return true;
         }
      default:
         break;
    }
  }
  return true;
}

bool compare (
   bool left,
   bool right,
   Termsequel::LogicalOperator operator_type
) {
   switch (operator_type) {
      case Termsequel::LogicalOperator::AND:
         return left && right;
      case Termsequel::LogicalOperator::OR:
         return left || right;
   }
   return false;
}

static std::string get_variable_value(std::string name) {

   // Ignores the $ symbol ...
   const char *variable_name = name.c_str() + 1;
#ifdef __linux__
   // we should not free result, otherwise, we might get an undefined behavior
   const char *result = std::getenv(variable_name);
   if (result == nullptr) {
      // invalid variable ...
      std::cerr << "Value not found for: " << name << std::endl;
      return "";
   }

   return result;
#elif defined(_WIN32)
   size_t required_size = 0;
   getenv_s(&required_size, NULL, 0, variable_name);
   if (required_size == 0) {
      // invalid variable ...
      std::cerr << "Value not found for: " << name << std::endl;
      return "";
   }
   char *result = (char *) malloc(sizeof(char) * required_size + 1);
   getenv_s(&required_size, result, required_size, variable_name);

   // copy the value to the string
   std::string value = result;

   // frees the manually allocated string
   free(result);
   return value;
#endif
}