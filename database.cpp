//
// Created by donald on 4/24/22.
//

#include <iostream>
#include <string>
#include <sys/stat.h>
#include <filesystem>
#include <map>
#include "database.h"
#include "sqlite3.h"

using namespace std;

Database::Database(bool debug_i) {
    debug = debug_i;
    filesystem::path home = getenv("HOME");
    filesystem::path db = ".tword.db";
    filesystem::path db_path = home / db;
    db_loc = db_path.generic_string();
}

int callback(void *data, int argc, char **argv, char **azColName){
    auto *results = static_cast<vector<struct Result> *>(data);
    Result result{atoi(argv[0]), static_cast<bool>(atoi(argv[1])), atoi(argv[2])};
    results->push_back(result);
    return 0;
}

bool Database::exists() {
    struct stat buffer{};
    return (stat (db_loc.c_str(), &buffer) == 0);
}

void Database::open() {
    int db_open = sqlite3_open(db_loc.c_str(), &tword_db);
    if(!db_open) {
        if(debug) {
            cout << "[DEBUG] DB Opened" << endl;
        }
    } else {
        cerr << "DB Error: " << sqlite3_errmsg(tword_db) << endl;
    }
}

void Database::close() {
    int db_close = sqlite3_close(tword_db);
    if(!db_close) {
        if(debug) {
            cout << "[DEBUG] DB Closed" << endl;
        }
    } else {
        cerr << "DB ERROR: " <<sqlite3_errmsg(tword_db) << endl;
    }
}

void Database::create() {
    if(debug) {
        cout << "[DEBUG] Creating Database" << endl;
    }
    open();
    string stmt = "CREATE TABLE RESULTS ("  \
      "ID INTEGER PRIMARY KEY AUTOINCREMENT," \
      "RESULT BOOL NOT NULL," \
      "ROUND_NUM INT);";

    char *zErrMsg = nullptr;
    int create_table = sqlite3_exec(tword_db, stmt.c_str(), callback, nullptr, &zErrMsg);
    if (create_table) {
        if(debug) {
            cerr << "DB Table Create Error: " << sqlite3_errmsg(tword_db) << endl;
        }
    } else {
        cout << "[DEBUG] Table Created Successfully!" << endl;
    }
    close();
}

void Database::insert_game(bool result, int round_num) {

    char buffer [100];
    auto stmt = sprintf(buffer, "INSERT INTO RESULTS(RESULT, ROUND_NUM) VALUES(%d,%d)", result, round_num);
    char *zErrMsg = nullptr;
    int insert = sqlite3_exec(tword_db, buffer, callback, nullptr, &zErrMsg);
    if(insert) {
        cerr << "DB Data Insert Error: " << sqlite3_errmsg(tword_db) << endl;
    } else {
        if(debug) {
            cout << "[DEBUG] DB Data Inserted Successfully!" << endl;
        }
    }
}

void Database::get_statistics() {
    cout << "\n== Statistics == " << endl;
    vector<Result> results{};
    string stmt = "SELECT * FROM RESULTS ORDER BY ID DESC";
    char *zErrMsg = nullptr;

    int db_select = sqlite3_exec(tword_db, stmt.c_str(), callback, &results, &zErrMsg);
    if(db_select) {
        cerr << "DB Data Select Error: " << sqlite3_errmsg(tword_db) << endl;
    } else {
        if(debug) {
            cout << "[DEBUG] DB Select All Executed" << endl;
        }

        int games_won = 0;
        map<int, int> distro {{1, 0}, {2,0}, {3,0}, {4,0}, {5, 0}, {6,0} };
        for(auto &result: results) {
            if(result.won > 0) {
                distro[result.round_num] = distro[result.round_num] + 1;
            }
            if(result.won) {
                games_won++;
            }
        }

        cout << "Games played: " << results.size() << endl;
        auto percent = (games_won * 100) / results.size();
        cout << "Win %: " << percent << endl;
        streaks(results);
        cout << "\n== Guess Distribution ==" << endl;
        for(int i = 1; i <= 6; i++) {
            cout << i << ": " << distro[i] << endl;
        }

        cout << endl;
    }
}

void Database::streaks(std::vector<Result> results) {

        bool current_win_streak_found  = false;
        int current_win_streak = 0;
        int max_win_streak = 0;

        int win_streak = 0;

        for(int i = 0; i < results.size(); i++) {
            auto & result = results[i];
            if(i == 0) {
                if(!result.won ) {
                    current_win_streak_found = true;
                    current_win_streak = 0;
                    continue;
                } else {
                    win_streak++;
                    current_win_streak++;
                    continue;
                }
            }

            if(!result.won) {
                if(!current_win_streak_found) {
                    current_win_streak_found = true;
                }

                if(win_streak > max_win_streak) {
                    max_win_streak = win_streak;
                }

                win_streak = 0;
                continue;
            } else {
                if(!current_win_streak_found) {
                    current_win_streak++;
                }
                win_streak++;
                continue;
            }
        }

        cout << "current win streak: " << current_win_streak << endl;
        cout << "max win streak: " << max_win_streak << endl;
}

void Database::clear_data() {
    string stmt = "DELETE FROM RESULTS";
    char *zErrMsg = nullptr;
    if(sqlite3_exec(tword_db, stmt.c_str(), callback, nullptr, &zErrMsg)) {
        cerr << "DB Data Deletion Error: " << sqlite3_errmsg(tword_db) << endl;
    } else {
            cout << "DB Data Deleted Successfully" << endl;
    }
}

