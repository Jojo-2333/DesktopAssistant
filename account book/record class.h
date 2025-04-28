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