#pragma once
#ifndef record_class_h
#define record_class_h

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <ctime>
#include <time.h>
#include <iomanip>
#include <algorithm>
#include <filesystem>

using namespace std;
namespace fs = filesystem;

// 定义文件路径
extern string dir;  //注意使用时要声明
fstream file;

// 检查文件路径合法性
bool file_init() {
    fs::path p(dir);
    if (!p.parent_path().empty() && !fs::exists(p.parent_path())) {
    cerr << "error: inventory doesn't exsit " << p.parent_path() << endl;
    return false;
}

// 检查文件名合法性
if (p.filename().string().find_first_of("*?\"<>|") != string::npos) {
    cerr << "error: illegal  " << endl;
    return false;}

return true;
}


//全局获取当前日期
string formatted_time(){
    time_t t = time(nullptr);
    tm* local_time = localtime(&t);
    if(local_time != nullptr) {
        // cout << "当前时间：" << put_time(local_time, "%Y-%m-%d %H:%M:%S") << endl;
        stringstream ss;
        ss << put_time(local_time, "%Y-%m-%d");
        return ss.str();
    }
    else {
        cerr << "error: failed to get the time" << endl;
        return ""; 
    }
}

class Record {
public:
    virtual void show() const = 0;
    virtual void edit() = 0;

};

class Records {
    vector <Record> records;
    virtual void show() {
        for (auto& record : records) {
            record.show();
        }
    }
    virtual void edit(int index) {
        records[index].edit();
    }
    virtual void add() {

    }
    virtual void records_delete(int index) {
        records.erase(records.begin() + index); 
    }
    int main_menu(){
        int choice;
        return choice;
    }
    virtual int menu(){
        int choice;
        return choice;
    }

};


#endif