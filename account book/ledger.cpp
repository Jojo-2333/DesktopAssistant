#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <ctime>
#include <time.h>
#include <iomanip>
#include <algorithm>
#include <filesystem>

using namespace std;
namespace fs = filesystem;
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

//全局操作的文件
const string ledger_dir = "ledger.csv";
fstream file;

bool file_init() {
    fs::path p(ledger_dir);
    if (!p.parent_path().empty() && !fs::exists(p.parent_path())) {
    cerr << "错误：目录不存在 " << p.parent_path() << endl;
    return false;
}

// 检查文件名合法性
if (p.filename().string().find_first_of("*?\"<>|") != string::npos) {
    cerr << "错误：非法文件名" << endl;
    return false;
}
return true;}


void file_write(string date, string description, string category, double amount) {
    file.open(ledger_dir, ios::app);
    file << date << "," << description << "," << category << "," << amount << endl;
    file.close(); 
}




class date {
public:
    int year;
    int month;
    int day;
    string strdate;
    date(int year, int month, int day) : year(year), month(month), day(day) {    
    if(month < 10) 
        strdate = to_string(year) + "-0" + to_string(month) + "-" + to_string(day);
    else 
        strdate = to_string(year) + "-" + to_string(month) + "-" + to_string(day);
    }
    date(string _formatted_time) {
        year=(_formatted_time[0]-'0')*1000+(_formatted_time[1]-'0')*100+(_formatted_time[2]-'0')*10+(_formatted_time[3]-'0');
        month=(_formatted_time[5]-'0')*10+(_formatted_time[6]-'0');
        day=(_formatted_time[8]-'0')*10+(_formatted_time[9]-'0');
        strdate = _formatted_time;
    }
    void show() const {
        if(month < 10) {
            cout << year << "-0" << month << "-" << day << endl;
        }
        else {
            cout << year << "-" << month << "-" << day << endl; 
        }
    }

    //date的运算符重载
    bool operator<(const date& other) const {
        if(year < other.year) return true;
        else if(year > other.year) return false;
        else if(month < other.month) return true;
        else if(month > other.month) return false;
        else return day < other.day; 
    }
    bool operator==(const date& other) const {
        return year == other.year && month == other.month && day == other.day; 
    }
    bool operator>(const date& other) const {
       if(*this < other | *this == other) return false;
       else return true;
    }
};

//下文account有“账目”的意思，注意不要和账户搞混。如make an account
class account {     //单条账目
public:
    string date;
    string description;
    string category;
    double amount;
    account(string date, string description, string category, double amount) : date(date), description(description), category(category), amount(amount) {
        confirm();
    }
    account(string account_line) {   //从文件中读取
        stringstream ss(account_line);
        string token;
        getline(ss, token, ',');
        date = token;
        getline(ss, token, ',');
        description = token;
        getline(ss, token, ',');
        category = token;
        ss >> amount;

    }
    void edit() {
        cout << "时间是否是当日?(y/n)" << endl;
        char choice;
        cin >> choice;
        if(choice == 'y') {
            date = formatted_time();
        }
        else {
            cout << "请输入新的日期(格式:YYYY-MM-DD):" << endl;
            cin >> date; 
        }

        cout << "请输入新的描述：" << endl; 
    }
    void show() const {
        cout << fixed << setprecision(2);
        cout << date << "," << description << "," << category << "," << amount << endl;
    }
    void record() const {
        file_write(date, description, category, amount);
    }
    void confirm() const {
        cout << "记录如下：" << endl;
        show();
        cout << "是否确认?(y/n)" << endl;
        char choice;
        cin >> choice; 
        if(choice == 'y') {
            record();
            cout << "已记录" << endl; 
        }
        else {
            
        }
    }

};

class accounts {    //账目本
public:
    vector<account> ledger;
    accounts() {
        if(!file_init()) {   //文件初始化
            cout << "error:文件初始化失败" << endl;
            return;
        }
        ifstream check(ledger_dir);
        if (!check) {
            ofstream create(ledger_dir);
            create << "Date,Description,Category,Amount" << endl;
            create.close();
        }
        file.open(ledger_dir, ios::in);
        if (!file.is_open()) {  //文件打开失败
            cout << "error:文件打开失败" << endl;
            return;
        }
        string line;
        getline(file, line);
        cout << line << endl;
        if(line.compare("Date,Description,Category,Amount")!=0){
            cout << "error:账本文件损坏" << endl;
            return;
        }
        while(getline(file, line)) {
            account new_account(line);
            ledger.push_back(new_account);
            sort();}
    }

    void sort() {
        std::sort(ledger.begin(), ledger.end(), [](const account& a, const account& b) {
            return a.date < b.date;
        });
    }
    void show() const {
        cout << "记账本Ledger"<< endl;
        cout << "Date,Description,Category,Amount" << endl;
        for(const auto& account : ledger) {   //遍历accounts容器中的每一个account对象
            account.show();
        } 
    }
    void add_account(string date, string description, string category, double amount) {
        account new_account(date, description, category, amount);
        ledger.push_back(new_account);    //尾部插入元素
        sort();
    }
    void delete_account(int index) {
        ledger.erase(ledger.begin() + index );  //删除指定位置的元素
    }

};



int main() {
    cout << "欢迎使用Ledger" << endl;
    cout << "当前日期: "<< formatted_time()<< endl;
    accounts Ledger;
    
    Ledger.add_account("2024-01-01", "工资", "工资", 1000);
    Ledger.show();
    Ledger.delete_account(1);
    Ledger.show();
}



