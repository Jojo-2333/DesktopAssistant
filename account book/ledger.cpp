#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <time.h>
#include <iomanip>

using namespace std;

//全局时间
void formatted_time(){
    time_t t = time(nullptr);
    tm tm = *localtime(&t);
    cout << put_time(&tm, "%F") << endl;

}

//全局操作的文件
const string ledger_dir = "ledger.csv";
fstream file;

void file_write(string date, string description, string category, double amount) {
    file.open(ledger_dir, ios::app);
    file << date << "," << description << "," << category << "," << amount << endl;
    file.close(); 
}
//下文account有“账目”的意思，注意不要和账户搞混。如make an account
class account {
public:
    string date;
    string description;
    string category;
    double amount;
    account(string date, string description, string category, double amount) : date(date), description(description), category(category), amount(amount) {
        confrim();
    }
    void edit() {
        cout << "时间是否是当日？（y/n）" << endl;
        char choice;
        cin >> choice;
        if(choice == 'y') {
            // date = formatted_time();
        }
        else {
            cout << "请输入新的日期（格式：YYYY-MM-DD）：" << endl;
            cin >> date; 
        }

        cout << "请输入新的描述：" << endl; 
    }
    void show() const {
        cout << left << setw(10) << date << setw(20) << description << setw(10) << category << setw(10) << amount << endl; 
    }
    void record() const {
        file_write(date, description, category, amount);
    }
    void confrim() const {
        cout << "记录如下：" << endl;
        show();
        cout << "是否确认？（y/n）" << endl;
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

class ledger {
public:
    vector<account> accounts;
    
};

int main() {

    formatted_time();

}



