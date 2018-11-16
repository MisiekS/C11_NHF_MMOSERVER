#include "database.hpp"

std::shared_ptr<Player> Database::login(std::string &username,
                                        std::string &password) {
    sqlite3_stmt *res;
    std::string query = "SELECT id,x,y FROM players WHERE username='" + username +
                        "' AND password='" + password + "';";
    int ec = sqlite3_prepare_v2(db, query.c_str(), -1, &res, 0);
    if (ec != SQLITE_OK) return nullptr;
    ec = sqlite3_step(res);
    if (ec != SQLITE_ROW) return nullptr;

    const char alphanum[] =
            "0123456789"
            "!@#$%^&*"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";

    std::string secret;
    int stringLength = sizeof(alphanum) - 1;
    srand(time(0));
    for(int i=0; i < 32; ++i)
        secret+=alphanum[rand() % stringLength];

    std::shared_ptr<Player> player_ = std::make_shared<Player>(
            username, sqlite3_column_int(res, 0), sqlite3_column_int(res, 1),
            sqlite3_column_int(res, 2),secret);

    sqlite3_finalize(res);
    return player_;
}

bool Database::registration(std::string &username, std::string &password) {
    char *err_msg = 0;

    std::string query = "INSERT INTO players (username,password,x,y) VALUES('" +
                        username + "','" + password + "',0,0);";
    int ec = sqlite3_exec(db, query.c_str(), 0, 0, &err_msg);
    if (ec != SQLITE_OK) {
        if (err_msg != nullptr) std::cout << err_msg << std::endl;
        if (err_msg != nullptr) sqlite3_free(err_msg);
        return false;
    }

    return true;
}

bool Database::save(const std::shared_ptr<Player> &p) {
    char *err_msg = 0;

    std::string query = "UPDATE players SET x=" + std::to_string(p->getX()) +
                        ",y=" + std::to_string(p->getY()) +
                        " WHERE id=" + std::to_string(p->getId()) + ";";
    int ec = sqlite3_exec(db, query.c_str(), 0, 0, &err_msg);
    if (ec != SQLITE_OK) {
        if (err_msg != nullptr) std::cout << err_msg << std::endl;
        if (err_msg != nullptr) sqlite3_free(err_msg);
        return false;
    }

    return true;
}
