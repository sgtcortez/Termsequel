/**
 * Should make system calls to different operating systems
*/
#ifndef __TERMSEQUEL_SYSTEM_H__
#define __TERMSEQUEL_SYSTEM_H__

#include <cstdint>
#include <string>
#include <vector>

namespace Termsequel {

   enum class COMMAND_TYPE {
      LIST // For now, just the list command is available
   };

   enum class COLUMN_TYPE {
      FILENAME,  // name of the file
      FILESIZE,  // size of the file,
      OWNER,     // owner of the file
      LEVEL,     // level of file in the deepth directory search
      FILE_TYPE, // type of the file. Regular file, directory etc ...
      OWNER_PERMISSIONS, // permissions of the owner
#ifdef __linux__
      GROUP_PERMISSIONS, // permissions of the group
      OTHERS_PERMISSIONS, // permissions of others
#endif
      LAST_MODIFICATION, // last modification of the file
      RELATIVE_PATH, // The relative path of the file
      ABSOLUTE_PATH, // The absolute path of the file
   };

   enum class LogicalOperator {
      AND, // LEFT AND RIGHT MUST BE TRUE
      OR   // LEFT OR RIGHT OR BOTH MUST BE TRUE
   };

   enum class Operator {
      EQUAL,
      STARTS_WITH,
      NOT_STARTS_WITH,
      ENDS_WITH,
      NOT_ENDS_WITH,
      CONTAINS,
      NOT_CONTAINS,
      BIGGER,
      LESS,
      BIGGER_OR_EQUAL,
      LESS_OR_EQUAL,
   };

   struct Condition {
      COLUMN_TYPE column;            // The column to apply the condition
      enum Operator operator_value;  // The operator to compare
      std::string value;             // The value to be compared
   };

   struct ConditionList {
      std::vector<Condition*> conditions;
      std::vector<LogicalOperator> operators;
   };


   // Represents an execution command
   struct SystemCommand {
      COMMAND_TYPE command;             // the command to be executed
      std::vector<COLUMN_TYPE> columns; // the columns to be retrieved
      std::string target;               // the target file/directory
      struct ConditionList *conditions; // The conditions that must be satisfied
   };

   class System {
      public:

         // Executes the command, and return the list of elements
         static void execute(const SystemCommand *command);
   };

}


#endif

