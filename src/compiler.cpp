#include "lexeme.hpp"
#include "lexical.hpp"
#include "syntax.hpp"
#include "compiler.hpp"
#include "system.hpp"
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <memory>

using column_type  = Termsequel::COLUMN_TYPE;
using lexeme       = Termsequel::LexemeType;
using lexeme_key   = Termsequel::LexemeKey;
using operator_key = Termsequel::Operator;

Termsequel::Compiler::Compiler(std::string raw_input) {
    this->raw_input = raw_input;
}

void append_action(
   const Termsequel::Lexeme *lexeme,
   Termsequel::SystemCommand *system_command
) {

   switch (lexeme->get_type().get_key()) {
      case lexeme_key::OPERATION:
         system_command->command = Termsequel::COMMAND_TYPE::LIST;
         break;
      case lexeme_key::COLUMN:
            if ( lexeme::STAR == lexeme->get_type()) {
               system_command->columns.push_back(column_type::FILENAME);
               system_command->columns.push_back(column_type::FILESIZE);
               system_command->columns.push_back(column_type::OWNER);
#ifdef __linux__
               system_command->columns.push_back(column_type::GROUP);
#endif
               system_command->columns.push_back(column_type::LEVEL);
               system_command->columns.push_back(column_type::FILE_TYPE);
               system_command->columns.push_back(column_type::OWNER_PERMISSIONS);
#ifdef __linux__
               system_command->columns.push_back(column_type::GROUP_PERMISSIONS);
               system_command->columns.push_back(column_type::OTHERS_PERMISSIONS);
#endif
               system_command->columns.push_back(column_type::LAST_MODIFICATION);
               system_command->columns.push_back(column_type::CREATION_DATE);
               system_command->columns.push_back(column_type::RELATIVE_PATH);
               system_command->columns.push_back(column_type::ABSOLUTE_PATH);
               break;
            }

            {
               // Can be column on column list or where filters
               auto column = column_type::FILENAME;
               if      ( lexeme::NAME               == lexeme->get_type() ) column = column_type::FILENAME;
               else if ( lexeme::SIZE               == lexeme->get_type() ) column = column_type::FILESIZE;
               else if ( lexeme::OWNER              == lexeme->get_type() ) column = column_type::OWNER;
#ifdef __linux__
               else if ( lexeme::GROUP              == lexeme->get_type() ) column = column_type::GROUP;
#endif
               else if ( lexeme::LEVEL              == lexeme->get_type() ) column = column_type::LEVEL;
               else if ( lexeme::FILE_TYPE          == lexeme->get_type() ) column = column_type::FILE_TYPE;
               else if ( lexeme::OWNER_PERMISSIONS  == lexeme->get_type() ) column = column_type::OWNER_PERMISSIONS;
#ifdef __linux__
               else if ( lexeme::GROUP_PERMISSIONS  == lexeme->get_type() ) column = column_type::GROUP_PERMISSIONS;
               else if ( lexeme::OTHERS_PERMISSIONS == lexeme->get_type() ) column = column_type::OTHERS_PERMISSIONS;
#endif
               else if ( lexeme::LAST_MODIFICATION  == lexeme->get_type() ) column = column_type::LAST_MODIFICATION;
               else if ( lexeme::CREATION_DATE      == lexeme->get_type() ) column = column_type::CREATION_DATE;
               else if ( lexeme::RELATIVE_PATH      == lexeme->get_type() ) column = column_type::RELATIVE_PATH;
               else if ( lexeme::ABSOLUTE_PATH      == lexeme->get_type() ) column = column_type::ABSOLUTE_PATH;

               if (system_command->conditions) {
                  // will be on the where filters
                  auto condition = new struct Termsequel::Condition();
                  condition->column = column;
                  system_command->conditions->conditions.push_back(condition);
               } else {
                  // will be at the column list
                  system_command->columns.push_back(column);
               }

            }
         break;
      case lexeme_key::WHERE:
         system_command->conditions = new Termsequel::ConditionList;
         break;
      case lexeme_key::COMPARASION:
         {
            operator_key operator_value;
            if      ( lexeme::EQUAL           == lexeme->get_type()) operator_value = operator_key::EQUAL;
            else if ( lexeme::NOT_EQUAL       == lexeme->get_type()) operator_value = operator_key::NOT_EQUAL;
            else if ( lexeme::STARTS_WITH     == lexeme->get_type()) operator_value = operator_key::STARTS_WITH;
            else if ( lexeme::NOT_STARTS_WITH == lexeme->get_type()) operator_value = operator_key::NOT_STARTS_WITH;
            else if ( lexeme::ENDS_WITH       == lexeme->get_type()) operator_value = operator_key::ENDS_WITH;
            else if ( lexeme::NOT_ENDS_WITH   == lexeme->get_type()) operator_value = operator_key::NOT_ENDS_WITH;
            else if ( lexeme::CONTAINS        == lexeme->get_type()) operator_value = operator_key::CONTAINS;
            else if ( lexeme::NOT_CONTAINS    == lexeme->get_type()) operator_value = operator_key::NOT_CONTAINS;
            else if ( lexeme::BIGGER          == lexeme->get_type()) operator_value = operator_key::BIGGER;
            else if ( lexeme::LESS            == lexeme->get_type()) operator_value = operator_key::LESS;
            else if ( lexeme::BIGGER_OR_EQUAL == lexeme->get_type()) operator_value = operator_key::BIGGER_OR_EQUAL;
            else if ( lexeme::LESS_OR_EQUAL   == lexeme->get_type()) operator_value = operator_key::LESS_OR_EQUAL;
            else {
               std::cerr << "Unmapped operator value!" << std::endl;
               break;
            }
            system_command->conditions->conditions[system_command->conditions->conditions.size() - 1]->operator_value = operator_value;
         }
         break;
      case lexeme_key::IDENTIFIER:
         if (system_command->conditions) {
            system_command->conditions->conditions[system_command->conditions->conditions.size() - 1]->value = *lexeme->get_value();
         } else {
            system_command->target = *lexeme->get_value();
         }
         break;
      case lexeme_key::LOGICAL:
         if (lexeme::AND == lexeme->get_type()) {
            system_command->conditions->operators.push_back(Termsequel::LogicalOperator::AND);
         } else {
            system_command->conditions->operators.push_back(Termsequel::LogicalOperator::OR);
         }
         break;
      default:
         break;
   }
}

Termsequel::SystemCommand * Termsequel::Compiler::compile() const {

   Lexical lexical(this->raw_input);
   Syntax syntax;

   if (!(syntax.analyse(lexical))) {
      // Would be better to throw an exception ...
      return nullptr;
   }

   const auto command = new Termsequel::SystemCommand();
   std::uint32_t index = 0;
   while (true) {

      const auto lexeme = lexical[index++].get();
      append_action(lexeme, command);
      if (!(lexical.has_next(index))) {
         // stop! Reached the end
         break;
      }
   }
   return command;
}