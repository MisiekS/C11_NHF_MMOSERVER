#include <iostream>
#include <string>
#include "database.hpp"

int main() {
  Database db{"db.sqlite3"};
  std::string s = "meshons";
  auto p = db.login(s, s);
  if (p)
    std::cout << "siker" << std::endl;
  else
    std::cout << "kamu" << std::endl;

  p->setX(1);

  if (db.save(p))
    std::cout << "siker" << std::endl;
  else
    std::cout << "kamu" << std::endl;
  return 0;
}