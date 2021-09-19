#include "lexeme.hpp"
#include <cstdint>
#include <memory>

// https://www.stroustrup.com/bs_faq2.html#in-class
Termsequel::LexemeType::LexemeType(
   const Termsequel::LexemeKey lexeme_key,
   std::string name
) : lexeme_key(lexeme_key), name(name) {
};

bool Termsequel::LexemeType::operator==(const LexemeType & right) const {
   const std::intptr_t left_address  = (std::intptr_t)this;
   const std::intptr_t right_address = (std::intptr_t)&right;
   return left_address == right_address;
};

// https://stackoverflow.com/questions/751681/meaning-of-const-last-in-a-function-declaration-of-a-class
Termsequel::LexemeKey Termsequel::LexemeType::get_key() const {
   return this->lexeme_key;
};

const std::string Termsequel::LexemeType::get_name() const {
   return this->name;
}

Termsequel::Lexeme::Lexeme(const LexemeType &lexeme_type) : lexeme_type(lexeme_type){
};

Termsequel::Lexeme::Lexeme(
   const LexemeType &lexeme_type,
   const std::string value ) : lexeme_type(lexeme_type), value(std::make_shared<std::string>(value)) {
};

const Termsequel::LexemeType & Termsequel::Lexeme::get_type() const {
   return this->lexeme_type;
}

const std::shared_ptr<std::string> Termsequel::Lexeme::get_value() const {
   return this->value;
}


namespace Termsequel {
   LexemeType const LexemeType::SELECT             ( LexemeKey::OPERATION,   "SELECT");
   LexemeType const LexemeType::STAR               ( LexemeKey::COLUMN,      "STAR");
   LexemeType const LexemeType::NAME               ( LexemeKey::COLUMN,      "NAME");
   LexemeType const LexemeType::SIZE               ( LexemeKey::COLUMN,      "SIZE");
   LexemeType const LexemeType::OWNER              ( LexemeKey::COLUMN,      "OWNER");
#ifdef __linux__
   LexemeType const LexemeType::GROUP              ( LexemeKey::COLUMN,      "GROUP");
#endif
   LexemeType const LexemeType::LEVEL              ( LexemeKey::COLUMN,      "LEVEL");
   LexemeType const LexemeType::FILE_TYPE          ( LexemeKey::COLUMN,      "FILE_TYPE");
   LexemeType const LexemeType::OWNER_PERMISSIONS  ( LexemeKey::COLUMN,      "OWNER_PERMISSIONS");
#ifdef __linux__
   LexemeType const LexemeType::GROUP_PERMISSIONS  ( LexemeKey::COLUMN,      "GROUP_PERMISSIONS");
   LexemeType const LexemeType::OTHERS_PERMISSIONS ( LexemeKey::COLUMN,      "OTHERS_PERMISSIONS");
#endif
   LexemeType const LexemeType::LAST_MODIFICATION  ( LexemeKey::COLUMN,      "LAST_MODIFICATION");
   LexemeType const LexemeType::CREATION_DATE      ( LexemeKey::COLUMN,      "CREATION_DATE");
   LexemeType const LexemeType::RELATIVE_PATH      ( LexemeKey::COLUMN,      "RELATIVE_PATH");
   LexemeType const LexemeType::ABSOLUTE_PATH      ( LexemeKey::COLUMN,      "ABSOLUTE_PATH");
   LexemeType const LexemeType::COMMA              ( LexemeKey::COMMA,       "COMMA");
   LexemeType const LexemeType::FROM               ( LexemeKey::FROM,        "FROM");
   LexemeType const LexemeType::IDENTIFIER         ( LexemeKey::IDENTIFIER,  "IDENTIFIER");
   LexemeType const LexemeType::WHERE              ( LexemeKey::WHERE,       "WHERE");
   LexemeType const LexemeType::EQUAL              ( LexemeKey::COMPARASION, "EQUAL");
   LexemeType const LexemeType::NOT_EQUAL          ( LexemeKey::COMPARASION, "NOT_EQUAL");
   LexemeType const LexemeType::STARTS_WITH        ( LexemeKey::COMPARASION, "STARTS_WITH");
   LexemeType const LexemeType::NOT_STARTS_WITH    ( LexemeKey::COMPARASION, "NOT_STARTS_WITH");
   LexemeType const LexemeType::ENDS_WITH          ( LexemeKey::COMPARASION, "ENDS_WITH");
   LexemeType const LexemeType::NOT_ENDS_WITH      ( LexemeKey::COMPARASION, "NOT_ENDS_WITH");
   LexemeType const LexemeType::CONTAINS           ( LexemeKey::COMPARASION, "CONTAINS");
   LexemeType const LexemeType::NOT_CONTAINS       ( LexemeKey::COMPARASION, "NOT_CONTAINS");
   LexemeType const LexemeType::BIGGER             ( LexemeKey::COMPARASION, "BIGGER");
   LexemeType const LexemeType::LESS               ( LexemeKey::COMPARASION, "LESS");
   LexemeType const LexemeType::BIGGER_OR_EQUAL    ( LexemeKey::COMPARASION, "BIGGER_OR_EQUAL");
   LexemeType const LexemeType::LESS_OR_EQUAL      ( LexemeKey::COMPARASION, "LESS_OR_EQUAL");
   LexemeType const LexemeType::AND                ( LexemeKey::LOGICAL,     "AND");
   LexemeType const LexemeType::OR                 ( LexemeKey::LOGICAL,     "OR");
}
