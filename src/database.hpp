#ifndef DATABASE_HPP
#define DATABASE_HPP 1

#include <sqlite3.h>
#include <iostream>
#include <memory>
#include <string>

#include "player.hpp"

class Database {
  sqlite3 *db;

 public:
  Database(std::string url) {
    int ret = sqlite3_open(url.c_str(), &db);
    if (ret != SQLITE_OK) {
      std::cerr << sqlite3_errmsg(db) << std::endl;
      throw std::runtime_error("cannot open db");
    }
  }
  ~Database() {
    if (db) sqlite3_close(db);
  }
  std::shared_ptr<Player> login(std::string &username, std::string &password);
  bool registration(std::string &username, std::string &password);
  bool save(const std::shared_ptr<Player> &p);
};

#endif