#ifndef SHUNTINGYARD_H
#define SHUNTINGYARD_H

// Includes
#include <map>
#include <queue>
#include <stack>
#include <string>

// Additionals
enum TokenType {NUMBER, OPERATOR, VARIABLE, FUNCTION, LPARENTHESIS, RPARENTHESIS};

typedef struct Token
{
  TokenType type_;
  std::string content_;
  int precedence_; // >0 if relevant
  int associativity_; // >0 if relevant, 1 = left, 2 = right
} Token;

class ShuntingYard
{
  public:
    // Constructor
    ShuntingYard();

    // Destructor
    ~ShuntingYard();

    // Methods
    std::deque<Token> getPostfix(std::string);
    void printPostfix(std::deque<Token>);
    double evaluate(std::string, std::map<std::string, double>);

  private:
    std::map<std::string, unsigned int> operators_;
    std::map<std::string, unsigned int> functions_;

    int handleNumber(std::string, unsigned int, std::deque<Token>*);
    int handleParentheses(char, std::deque<Token>*, std::stack<Token>*);
    int handleFunctionArgumentSeparator(std::deque<Token>*, std::stack<Token>*);
    int handleOperator(std::string, unsigned int, std::deque<Token>*, std::stack<Token>*);
    int handleFunctionOrVariable(std::string, unsigned int, std::deque<Token>*, std::stack<Token>*);
    int calculateFunctionOrOperator(Token*, std::stack<double>*);
    std::vector<double> popValuesFromStack(std::stack<double>*, unsigned int);
    void reportError(std::string);
    
};

#endif /* SHUNTINGYARD_H */
