// Includes
#include "ShuntingYard.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <deque>
#include <string>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

// Constructor
ShuntingYard::ShuntingYard()
{
  this->operators_.insert(std::pair<std::string, unsigned int>("+", 2));
  this->operators_.insert(std::pair<std::string, unsigned int>("-", 2));
  this->operators_.insert(std::pair<std::string, unsigned int>("*", 2));
  this->operators_.insert(std::pair<std::string, unsigned int>("/", 2));
  this->operators_.insert(std::pair<std::string, unsigned int>("^", 2));
  this->operators_.insert(std::pair<std::string, unsigned int>("#", 1)); // Replacement for unary minus sign

  this->functions_.insert(std::pair<std::string, unsigned int>("sin", 1));
  this->functions_.insert(std::pair<std::string, unsigned int>("cos", 1));
  this->functions_.insert(std::pair<std::string, unsigned int>("max", 2));
  this->functions_.insert(std::pair<std::string, unsigned int>("min", 2));
}

// Destructor
ShuntingYard::~ShuntingYard()
{

}

// Reads an input string in infix notation and converts it to postfix notation.
// @param infix_string The string to convert.
// @return Deque containing postfix notation of input string.
std::deque<Token> ShuntingYard::getPostfix(std::string infix_string)
{
  std::deque<Token> output;
  std::stack<Token> opstack;
  char current;
  std::string input_sanitized = infix_string;
  input_sanitized.erase(std::remove(input_sanitized.begin(), input_sanitized.end(), ' '), input_sanitized.end());

  // Start parsing the string in infix notation
  for(unsigned int i = 0; i < input_sanitized.size(); i++) {
    current = input_sanitized.at(i);
    // Number
    if(isdigit(current)) {
      int size = this->handleNumber(input_sanitized, i, &output);
      if(size == -1) {
        this->reportError("Infix notation contains invalid numbers. Please use only digits and at most one decimal point (.).");
        return std::deque<Token>();
      }
      i += (size - 1);
    }
    // Parentheses
    else if(current == '(' || current == ')') {
      int ret = this->handleParentheses(current, &output, &opstack);
      if(ret == -1) {
        this->reportError("There seems to be an error with the parentheses.");
        return std::deque<Token>();
      }
    }
    // Function argument separator (,)
    else if(current == ',') {
      int ret = this->handleFunctionArgumentSeparator(&output, &opstack);
      if(ret == -1) {
        this->reportError("There seems to be an error with the function separator or the parentheses.");
        return std::deque<Token>();
      }
    }
    // Operator
    else if(!isalnum(current)) {
      if(current == '-' && (i + 1) < input_sanitized.size() && input_sanitized.at(i + 1) == '-') {
        // Skip double minus (--)
        i++;
        continue;
      }
      int size = this->handleOperator(input_sanitized, i, &output, &opstack);
      if(size == -1) {
        this->reportError("There was an error while trying to parse an operator.");
        return std::deque<Token>();
      }
      i += (size - 1);
    }
    // Function or variable
    else if(isalpha(current)) {
      int size = this->handleFunctionOrVariable(input_sanitized, i, &output, &opstack);
      if(size == -1) {
        this->reportError("There was an error reading a function or variable (alphabetic letter is not part of a function or variable).");
        return std::deque<Token>();
      }
      i += (size - 1);
    }
    else {
      // Error: Unknown input
      this->reportError("There was an error while reading the input (unknown symbols).");
      return std::deque<Token>();
    }
  }

  // No more tokens
  Token token_top;
  while(opstack.size() > 0) {
    token_top = opstack.top();
    if(token_top.type_ == LPARENTHESIS || token_top.type_ == RPARENTHESIS) {
      // Error: Mismatched parentheses
      this->reportError("There seems to be an error with the parentheses.");
      return std::deque<Token>();
    }
    output.push_back(token_top);
    opstack.pop();
  }

  return output;
}

// Prints a deque containing tokens in postfix notation.
// @param postfix_deque The deque containing tokens in postfix notation.
void ShuntingYard::printPostfix(std::deque<Token> postfix_deque)
{
  if(postfix_deque.empty()) {
    this->reportError("No tokens could be found.");
    return;
  }

  std::cout << "------------------------------------" << std::endl;
  std::cout << "Number of tokens: " << postfix_deque.size() << std::endl;
  std::cout << "[0] = Number (int or double)" << std::endl;
  std::cout << "[1] = Operator (+, -, *, /, ^)" << std::endl;
  std::cout << "[2] = Variable (x, y, z, ...)" << std::endl;
  std::cout << "[3] = Function (sin, cos, ...)" << std::endl;
  std::cout << "------------------------------------" << std::endl;
  std::string postfix_string = "Simple:";
  for(std::deque<Token>::iterator it = postfix_deque.begin(); it != postfix_deque.end(); it++) {
    postfix_string += (" " + it->content_ );
  }
  postfix_string += "\nDetailed:";
  for(std::deque<Token>::iterator it = postfix_deque.begin(); it != postfix_deque.end(); it++) {
    postfix_string += (" " + it->content_ + "[" + std::to_string(it->type_) + "]");
  }
  std::cout << postfix_string << std::endl;
}

// Evaluates a formula using given variable definitions.
// @param infix_string The deque containing tokens in postfix notation.
// @param definitions A map containing a value for each variable.
double ShuntingYard::evaluate(std::string infix_string, std::map<std::string, double> definitions)
{
  std::deque<Token> postfix_deque = this->getPostfix(infix_string);
  if(postfix_deque.empty()) {
    // Error: faulty input string
    this->reportError("There was an error while trying to evaluate the formula.");
    return 0;
  }

  std::stack<double> valstack;

  Token token;
  for(std::deque<Token>::iterator it = postfix_deque.begin(); it != postfix_deque.end(); it++) {
    token = (*it);
    if(token.type_ == NUMBER) {
      valstack.push(std::stod(token.content_));
    }
    else if(token.type_ == VARIABLE) {
      std::map<std::string, double>::iterator def_it = definitions.find(token.content_);
      if(def_it == definitions.end()) {
        // Error: missing variable definitions
        this->reportError("Missing variable definition for \"" + token.content_ + "\".");
        return 0;
      }
      valstack.push(def_it->second);
    }
    else if(token.type_ == OPERATOR || token.type_ == FUNCTION) {
      int ret = this->calculateFunctionOrOperator(&token, &valstack);
      if(ret == 1 || ret == 2) {
        // Error: not enough values on stack for this operator
        this->reportError("There are not enough values available for this operator.");
        return 0;
      }
    }
    else {
      // Error: invalid token type
      this->reportError("An invalid token type was encountered while trying to evaluate the formula.");
      return 0;
    }
  }

  if(valstack.size() == 1) {
    double result = valstack.top();
    return result;
  }
  else {
    // Error: user input has too many values
    this->reportError("The input contains too many values.");
    return 0;
  }

  return 0;
}

// Handles a number in the original input string.
// @param output The final output queue.
// @return The length of the resulting number or <0 if an error occured.
int ShuntingYard::handleNumber(std::string input, unsigned int start_index, std::deque<Token>* output)
{
  std::string number;
  int decimal_point_count = 0;
  for(unsigned int i = start_index; i < input.size(); i++) {
    if(isdigit(input.at(i))) {
      number.push_back(input.at(i));
    }
    else if(input.at(i) == '.') {
      number.push_back(input.at(i));
      decimal_point_count++;
    }
    else {
      // Break if number is finished
      break;
    }
  }

  if(decimal_point_count == 0 || (decimal_point_count == 1 && number.front() != '.' && number.back() != '.')) {
    Token token;
    token.type_ = NUMBER;
    token.content_ = number;
    token.precedence_ = 0;
    token.associativity_ = 0;
    output->push_back(token);
    return number.size();
  }
  else {
    // Error: invalid number
    return -1;
  }
}

// Handles a parenthesis in the original input string.
// @param parentheses The parenthesis.
// @param output The final output queue.
// @param opstack The operator stack.
// @return An error code, 0 = no error.
int ShuntingYard::handleParentheses(char parentheses, std::deque<Token>* output, std::stack<Token>* opstack)
{
  Token token;
  if(parentheses ==  '(')
    token.type_ = LPARENTHESIS;
  else if(parentheses == ')')
    token.type_ = RPARENTHESIS;
  token.content_ = parentheses;
  token.precedence_ = 0;
  token.associativity_ = 0;

  if(token.type_ == LPARENTHESIS) {
    opstack->push(token);
    return 0;
  }
  else if(token.type_ == RPARENTHESIS) {
    // Push all stack tokens to the deque until left parenthesis is found
    Token token_top;
    while(!opstack->empty() && (token_top = opstack->top()).type_ != LPARENTHESIS) {
      output->push_back(token_top);
      opstack->pop();
    }

    if(opstack->empty()) {
      // Error: Mismatched parentheses
      return -1;
    }

    // Remove left parenthesis from stack now
    opstack->pop();

    // If top token is now function, push it to the deque
    if(!opstack->empty() && (token_top = opstack->top()).type_ == FUNCTION) {
      output->push_back(token_top);
      opstack->pop();
    }

  }

  return 0;
}

// Handles a function argument separator in the original input string.
// @param output The final output queue.
// @param opstack The operator stack.
// @return An error code, 0 = no error.
int ShuntingYard::handleFunctionArgumentSeparator(std::deque<Token>* output, std::stack<Token>* opstack)
{
  // Push all stack tokens to the deque until left parenthesis is found
  if(opstack->empty()) {
    // Error: Mismatched parentheses
    return -1;
  }
  Token token_top = opstack->top();
  while(token_top.type_ != LPARENTHESIS) {
    output->push_back(token_top);
    opstack->pop();
    if(opstack->empty()) {
      // Error: Mismatched parentheses
      return -1;
    }
    token_top = opstack->top();
  }
  if(token_top.type_ != LPARENTHESIS) {
    // Error: Mismatched parentheses
    return -1;
  }
  return 0;
}

// Handles an operator in the original input string.
// @param input The input string.
// @param start_index The starting index of this operation.
// @param output The final output queue.
// @param opstack The operator stack.
// @return The length of the resulting operator or <0 if an error occured.
int ShuntingYard::handleOperator(std::string input, unsigned int start_index, std::deque<Token>* output, std::stack<Token>* opstack)
{
  std::string op;
  for(unsigned int i = start_index; i < input.size(); i++) {
    if(input.at(i) == '(' || isalnum(input.at(i))) {
      break;
    }
    op.push_back(input.at(i));
  }

  if(this->operators_.find(op) == this->operators_.end()) {
    // Error: no such operator
    std::cout << "op: " << op << std::endl;
    return -1;
  }

  Token token;
  token.type_ = OPERATOR;
  token.content_ = op;
  if(op == "+" || op == "-") {
    token.precedence_ = 4;
    // TODO: Adapt for multi-character operators
    std::string predecessor(1, input.at(start_index - 1));
    if(op == "-" && start_index == 0 || this->operators_.find(predecessor) != this->operators_.end() || input.at(start_index - 1) == '(') {
      // Unary minus operator
      token.content_ = "#";
      token.precedence_ = 1;
    }
  }
  else if(op == "*" || op == "/")
    token.precedence_ = 3;
  else if(op == "^")
    token.precedence_ = 2;
  if(op == "+" || op == "-" || op == "*" || op == "/")
    token.associativity_ = 1;
  else if(op == "^")
    token.associativity_ = 2;
  
  Token token_top;
  while((!opstack->empty() && (token_top = opstack->top()).type_ == OPERATOR) &&
    ((token_top.associativity_ == 1 && token_top.precedence_ <= token.precedence_) ||
    (token_top.associativity_ == 2 && token_top.precedence_ < token.precedence_))) {
    // Left-associative and precedence less then or equal to that of op OR right-associative and precedence less than that of op
    output->push_back(token_top);
    opstack->pop();
  }

  opstack->push(token);

  return op.size();
}

// Handles a function or variable in the original input string.
// @param input The input string.
// @param start_index The starting index of this operation.
// @param output The final output queue.
// @param opstack The operator stack.
// @return The length of the resulting function or variable or <0 if an error occured.
int ShuntingYard::handleFunctionOrVariable(std::string input, unsigned int start_index, std::deque<Token>* output, std::stack<Token>* opstack)
{
  std::string thing;
  unsigned int i = start_index;
  for(; i < input.size(); i++) {
    if(isalpha(input.at(i))) {
      thing.push_back(input.at(i));
    }
    else {
      // Break on first occurrence of non-alpha character
      break;
    }
  }

  Token token;
  if(this->functions_.find(thing) != this->functions_.end() && input.at(i) == '(') {
    token.type_ = FUNCTION;
    token.precedence_ = 1;
  }
  else if(thing.size() == 1) {
    token.type_ = VARIABLE;
    token.precedence_ = 0;
  }
  else {
    // Error: alphabetic letter is not part of a function or variable
    return -1;
  }
  token.content_ = thing;
  token.associativity_ = 0;

  if(token.type_ == FUNCTION)
    opstack->push(token);
  else if(token.type_ == VARIABLE)
    output->push_back(token);
  
  return thing.size();
}

// Calculates a new value using an operator or a function and pushes the new value to the stack.
// @param token The token to use as an operator or function.
// @param valstack A stack containing all previous values.
// @return An error code, 0 = no error.
int ShuntingYard::calculateFunctionOrOperator(Token* token, std::stack<double>* valstack)
{
  unsigned int val_count;
  if(token->type_ == OPERATOR && (valstack->size() < (val_count = ((this->operators_.find(token->content_))->second)))) {
    // Error: not enough values on stack for this operator
    return 1;
  }
  if(token->type_ == FUNCTION && (valstack->size() < (val_count = ((this->functions_.find(token->content_))->second)))) {
    // Error: not enough values on stack for this function
    return 2;
  }

  std::vector<double> values = this->popValuesFromStack(valstack, val_count);

  // Remark: Values are on stack in reverse order
  if(token->type_ == OPERATOR) {
    if(token->content_ == "+") {
      double result = values.at(1) + values.at(0);
      valstack->push(result);
    }
    else if(token->content_ == "-") {
      double result = values.at(1) - values.at(0);
      valstack->push(result);
    }
    else if(token->content_ == "*") {
      double result = values.at(1) * values.at(0);
      valstack->push(result);
    }
    else if(token->content_ == "/") {
      double result = values.at(1) / values.at(0);
      valstack->push(result);
    }
    else if(token->content_ == "^") {
      double result = pow(values.at(1), values.at(0));
      valstack->push(result);
    }
    else if(token->content_ == "#") {
      double result = values.at(0) * -1.0;
      valstack->push(result);
    }
  }
  else if(token->type_ == FUNCTION) {
    if(token->content_ == "sin") {
      double result = sin(values.at(0));
      valstack->push(result);
    }
    else if(token->content_ == "cos") {
      double result = cos(values.at(0));
      valstack->push(result);
    }
    else if(token->content_ == "max") {
      double result = (values.at(1) > values.at(0)) ? values.at(1) : values.at(0);
      valstack->push(result);
    }
    else if(token->content_ == "min") {
      double result = (values.at(1) < values.at(0)) ? values.at(1) : values.at(0);
      valstack->push(result);
    }
  }

  return 0;
}

// Removes values from the stack and stores them into a vector.
// @param valstack The stack to remove values from.
// @param val_count A number indicating how many values to remove.
// @return A vector containing the removed values.
std::vector<double> ShuntingYard::popValuesFromStack(std::stack<double>* valstack, unsigned int val_count)
{
  std::vector<double> values;
  while(val_count-- > 0) {
    values.push_back(valstack->top());
    valstack->pop();
  }
  return values;
}

// Helper function to print errors.
// @param message The error message.
void ShuntingYard::reportError(std::string message)
{
  std::cout << "[ERROR] " << message << std::endl;
}
