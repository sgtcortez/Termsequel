#ifndef TERMSEQUEL_EVALUATOR_H
#define TERMSEQUEL_EVALUATOR_H

#include "termsequel/lexer/token.hpp"

#include <string>

namespace termsequel {
namespace evaluator {
/**
 * Evaluates a given binary expression.
 * @param left side of the expression.
 * @param operation the operation be to perform.
 * @param right side of the expression.
 * @return the result of the expression.
 */
std::string evaluate ( const std::string &left, const token::Token &operation, const std::string &right );

/**
 * Evaluates a given unary expression.
 * @param value to be used.
 * @param operation the operation be to perform.
 * @return the result of the expression
 */
std::string evaluate ( const std::string &value, const token::Token &operation );
} // namespace evaluator
} // namespace termsequel

#endif // TERMSEQUEL_EVALUATOR_H