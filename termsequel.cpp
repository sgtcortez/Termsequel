#include <iostream>

#include "termsequel/executor/executor.hpp"
#include "termsequel/lexer/lexer.hpp"
#include "termsequel/parser/parser.hpp"

using namespace std;
using namespace termsequel;

void interactive_mode();
void run_sql(const string sql);
executor::Executor query_executor;

int main(
   int argc, 
   char **argv
)
{
   bool be_interactive = false;
   for (int i = 1; i < argc; ++i) {
      if (string(argv[i]) == "--interactive") {
         be_interactive = true;
      }
   }

   if (be_interactive) {
      interactive_mode();
   } else {
      run_sql(argv[1]);
   }
   return 0;
}

void interactive_mode()
{
   cout << "Interactive mode" << endl;
   while (true)
   {
      cout << "\nPlease, enter your input: ";
      string input;
      getline(cin, input);

      if (input == "/exit")
      {
         return;
      }
      run_sql(input);
   }
}

void run_sql(const string sql)
{
   const auto result = query_executor.run(sql);
   for (const auto& row : result)
   {
      cout << "NAME=" << row.name << "VALUE=" << row.value << endl;
   }
}