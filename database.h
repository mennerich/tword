//
// Created by donald on 4/24/22.
//

#ifndef TWORD_DATABASE_H
#define TWORD_DATABASE_H

#include <string>
#include <sqlite3.h>

class Database {
private:
    std::string db_loc = "/home/donald/.config/tword.sql";
    sqlite3 *tword_db;
    std::string get_home();

public:
    bool exists();
    void create();
    void insert_game(bool result, int round_num);
    void open();
    void close();
};


#endif //TWORD_DATABASE_H