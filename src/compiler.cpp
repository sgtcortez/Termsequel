#include "lexeme.hpp"
#include "lexical.hpp"
#include "syntax.hpp"
#include "compiler.hpp"
#include "system.hpp"
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>


Termsequel::Compiler::Compiler(std::string raw_input) {
    this->raw_input = raw_input;
}

void append_action(
   const Termsequel::Lexeme *lexeme,
   Termsequel::SystemCommand *system_command
) {

   switch (lexeme->get_type().get_key()) {
      case Termsequel::LexemeKey::OPERATION:
         system_command->command = Termsequel::COMMAND_TYPE::LIST;
         break;
      case Termsequel::LexemeKey::COLUMN:
            if ( Termsequel::LexemeType::STAR == lexeme->get_type()) {
               system_command->columns.push_back(Termsequel::COLUMN_TYPE::FILENAME);
               system_command->columns.push_back(Termsequel::COLUMN_TYPE::FILESIZE);
               system_command->columns.push_back(Termsequel::COLUMN_TYPE::OWNER);
               system_command->columns.push_back(Termsequel::COLUMN_TYPE::LEVEL);
               system_command->columns.push_back(Termsequel::COLUMN_TYPE::FILE_TYPE);
               system_command->columns.push_back(Termsequel::COLUMN_TYPE::OWNER_PERMISSIONS);
#ifdef __linux__
               system_command->columns.push_back(Termsequel::COLUMN_TYPE::GROUP_PERMISSIONS);
               system_command->columns.push_back(Termsequel::COLUMN_TYPE::OTHERS_PERMISSIONS);
#endif
               system_command->columns.push_back(Termsequel::COLUMN_TYPE::LAST_MODIFICATION);
               system_command->columns.push_back(Termsequel::COLUMN_TYPE::RELATIVE_PATH);
               system_command->columns.push_back(Termsequel::COLUMN_TYPE::ABSOLUTE_PATH);
               break;
            }

            {
               // Can be column on column list or where filters
               Termsequel::COLUMN_TYPE column = Termsequel::COLUMN_TYPE::FILENAME;
               if      ( Termsequel::LexemeType::NAME               == lexeme->get_type() ) column = Termsequel::COLUMN_TYPE::FILENAME;
               else if ( Termsequel::LexemeType::SIZE               == lexeme->get_type() ) column = Termsequel::COLUMN_TYPE::FILESIZE;
               else if ( Termsequel::LexemeType::OWNER              == lexeme->get_type() ) column = Termsequel::COLUMN_TYPE::OWNER;
               else if ( Termsequel::LexemeType::LEVEL              == lexeme->get_type() ) column = Termsequel::COLUMN_TYPE::LEVEL;
               else if ( Termsequel::LexemeType::FILE_TYPE          == lexeme->get_type() ) column = Termsequel::COLUMN_TYPE::FILE_TYPE;
               else if ( Termsequel::LexemeType::OWNER_PERMISSIONS  == lexeme->get_type() ) column = Termsequel::COLUMN_TYPE::OWNER_PERMISSIONS;
#ifdef __linux__
               else if ( Termsequel::LexemeType::GROUP_PERMISSIONS  == lexeme->get_type() ) column = Termsequel::COLUMN_TYPE::GROUP_PERMISSIONS;
               else if ( Termsequel::LexemeType::OTHERS_PERMISSIONS == lexeme->get_type() ) column = Termsequel::COLUMN_TYPE::OTHERS_PERMISSIONS;
#endif
               else if ( Termsequel::LexemeType::LAST_MODIFICATION  == lexeme->get_type() ) column = Termsequel::COLUMN_TYPE::LAST_MODIFICATION;
               else if ( Termsequel::LexemeType::RELATIVE_PATH      == lexeme->get_type() ) column = Termsequel::COLUMN_TYPE::RELATIVE_PATH;
               else if ( Termsequel::LexemeType::ABSOLUTE_PATH      == lexeme->get_type() ) column = Termsequel::COLUMN_TYPE::ABSOLUTE_PATH;

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
      case Termsequel::LexemeKey::WHERE:
         system_command->conditions = new Termsequel::ConditionList;
         break;
      case Termsequel::LexemeKey::COMPARASION:
         {
            Termsequel::Operator operator_value;
            if      ( Termsequel::LexemeType::EQUAL           == lexeme->get_type()) operator_value = Termsequel::Operator::EQUAL;
            else if ( Termsequel::LexemeType::STARTS_WITH     == lexeme->get_type()) operator_value = Termsequel::Operator::STARTS_WITH;
            else if ( Termsequel::LexemeType::NOT_STARTS_WITH == lexeme->get_type()) operator_value = Termsequel::Operator::NOT_STARTS_WITH;
            else if ( Termsequel::LexemeType::ENDS_WITH       == lexeme->get_type()) operator_value = Termsequel::Operator::ENDS_WITH;
            else if ( Termsequel::LexemeType::NOT_ENDS_WITH   == lexeme->get_type()) operator_value = Termsequel::Operator::NOT_ENDS_WITH;
            else if ( Termsequel::LexemeType::CONTAINS        == lexeme->get_type()) operator_value = Termsequel::Operator::CONTAINS;
            else if ( Termsequel::LexemeType::NOT_CONTAINS    == lexeme->get_type()) operator_value = Termsequel::Operator::NOT_CONTAINS;
            else if ( Termsequel::LexemeType::BIGGER          == lexeme->get_type()) operator_value = Termsequel::Operator::BIGGER;
            else if ( Termsequel::LexemeType::LESS            == lexeme->get_type()) operator_value = Termsequel::Operator::LESS;
            else if ( Termsequel::LexemeType::BIGGER_OR_EQUAL == lexeme->get_type()) operator_value = Termsequel::Operator::BIGGER_OR_EQUAL;
            else if ( Termsequel::LexemeType::LESS_OR_EQUAL   == lexeme->get_type()) operator_value = Termsequel::Operator::LESS_OR_EQUAL;
            else {
               std::cerr << "Unmapped operator value!" << std::endl;
               break;
            }
            system_command->conditions->conditions[system_command->conditions->conditions.size() - 1]->operator_value = operator_value;
         }
         break;
      case Termsequel::LexemeKey::IDENTIFIER:
         if (system_command->conditions) {
            system_command->conditions->conditions[system_command->conditions->conditions.size() - 1]->value = *lexeme->get_value();
         } else {
            system_command->target = *lexeme->get_value();
         }
         break;
      case Termsequel::LexemeKey::LOGICAL:
         if (Termsequel::LexemeType::AND == lexeme->get_type()) {
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