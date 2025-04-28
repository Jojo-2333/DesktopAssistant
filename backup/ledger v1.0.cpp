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
    cerr << "error: inventory doesn't exsit " << p.parent_path() << endl;
    return false;
}

// 检查文件名合法性
if (p.filename().string().find_first_of("*?\"<>|") != string::npos) {
    cerr << "error: illegal  " << endl;
    return false;}

return true;
}

// 已经弃用的日期类，因为可以直接字符串比较
// class date {
// public:
//     int year;
//     int month;
//     int day;
//     string strdate;
//     date(int year, int month, int day) : year(year), month(month), day(day) {    
//     if(month < 10) 
//         strdate = to_string(year) + "-0" + to_string(month) + "-" + to_string(day);
//     else 
//         strdate = to_string(year) + "-" + to_string(month) + "-" + to_string(day);
//     }
//     date(string _formatted_time) {
//         year=(_formatted_time[0]-'0')*1000+(_formatted_time[1]-'0')*100+(_formatted_time[2]-'0')*10+(_formatted_time[3]-'0');
//         month=(_formatted_time[5]-'0')*10+(_formatted_time[6]-'0');
//         day=(_formatted_time[8]-'0')*10+(_formatted_time[9]-'0');
//         strdate = _formatted_time;
//     }
//     void show() const {
//         if(month < 10) {
//             cout << year << "-0" << month << "-" << day << endl;
//         }
//         else {
//             cout << year << "-" << month << "-" << day << endl; 
//         }
//     }
//     //date的运算符重载
//     bool operator<(const date& other) const {
//         if(year < other.year) return true;
//         else if(year > other.year) return false;
//         else if(month < other.month) return true;
//         else if(month > other.month) return false;
//         else return day < other.day; 
//     }
//     bool operator==(const date& other) const {
//         return year == other.year && month == other.month && day == other.day; 
//     }
//     bool operator>(const date& other) const {
//        if(*this < other | *this == other) return false;
//        else return true;
//     }
// };

//下文account有“账目”的意思，注意不要和账户搞混。如make an account
class account {     //单条账目
public:
    string date;
    string description;
    string category;
    double amount;
    account(string date, string description, string category, double amount) : date(date), description(description), category(category), amount(amount) {
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
    //保留两位小数，输出为csv格式
    string csv_format() const {   
        std::stringstream ss;
        ss << std::fixed << std::setprecision(2);  // 强制保留两位小数
        ss << amount;                              
        return date + "," + description + "," + category + "," + ss.str();
    }
    void edit() {
        cout << "时间是否是当日？(y/n)" << endl;
        char choice;
        cin >> choice;
        if(choice == 'y') {
            date = formatted_time();
        }
        else {
            cout << "请输入新的日期(格式:YYYY-MM-DD):" << endl;
            cin >> date;
            while(date.length()!= 10 || (date[4]!='-')||(date[7]!='-')){
                cout << "error:日期格式错误，请重新输入(格式:YYYY-MM-DD)" << endl;
                cin.clear();
                cin >> date;
            }
        }
        cin.ignore();//忽略换行符
        cout << "请输入新的描述：" << endl; 
        getline(cin, description);
        cout << "请输入新的分类：" << endl;
        getline(cin, category);
        cout << "请输入新的金额：" << endl;
        cin >> amount;
    }
    void show() const {
        cout << csv_format() << endl;
    }
};

class accounts {    //账目本
public:
    vector<account> ledger;
    accounts() {    //构造函数,从文件中读取
        if(!file_init()) {   //文件(目录)初始化
            cout << "error:文件初始化失败" << endl;
            return;
        }
        ifstream check(ledger_dir);
        if (!check) {
            ofstream create(ledger_dir);
            create << "Date,Description,Category,Amount" << endl;
            create.close();
        }
        check.close();

        file.open(ledger_dir, ios::in);
        if (!file.is_open()) {  //文件打开失败
            cout << "error:文件打开失败" << endl;
            return;
        }

        string line;
        fflush(stdin);
        getline(file, line);
        if(line.compare("Date,Description,Category,Amount")!=0){
            cout << "error:账本文件损坏" << endl;
            return;
        }
        fflush(stdin);
        while(getline(file, line)) {
            account new_account(line);
            ledger.push_back(new_account);
            }
        sort();
    }
    ~accounts() {   //析构函数,将ledger中的账目覆写文件
        ofstream file(ledger_dir, ios::out);
        file << "Date,Description,Category,Amount" << endl;
        for(const auto& account : ledger) {   //遍历ledger容器中的每一个account对象
            file << account.csv_format() << endl;
        }
        file.close();
        cout << "\n----------已保存更改----------" << endl;
    }
    void sort() {
        std::sort(ledger.begin(), ledger.end(), [](const account& a, const account& b) {
            return a.date < b.date;
        });
    }
    void show() const {
        cout <<endl<< "----------记账本Ledger----------"<< endl;
        cout << "Date,Description,Category,Amount" << endl;
        for(const auto& account : ledger) {   //遍历accounts容器中的每一个account对象
            account.show();
        } 
        cout << "--------------------------------"<< endl;
    }
    void add_account() {
        cout << "添加新的条目" << endl;
        string date, description, category;
        double amount;
        cout << "请输入日期(格式:YYYY-MM-DD):" << endl;
        cin >> date;
        while(date.length()!= 10 || (date[4]!='-')||(date[7]!='-')){
            cout << "error:日期格式错误，请重新输入(格式:YYYY-MM-DD)" << endl;
            cin.clear();
            cin >> date;
        }
        cin.ignore();//忽略换行符
        cout << "请输入描述：" << endl;
        getline(cin, description);
        cout << "请输入分类：" << endl;
        getline(cin, category);
        cout << "请输入金额：" << endl;
        cin >> amount;
        account new_account(date, description, category, amount);
        ledger.push_back(new_account);    //尾部插入元素
        sort();
        cout << "已添加" << endl;
    }

//重载add_account函数，用于测试
    void add_account(string date, string description, string category, double amount) {
        account new_account(date, description, category, amount);
        ledger.push_back(new_account);    //尾部插入元素
        sort(); 
    }
//删除指定位置的账目，index从0开始，对应accounts容器中的第一个元素
    void delete_account(int index) {
        while (index < 0 || index >= ledger.size()) {
            cout << "error:无效的索引，请重新输入" << endl;
            cin.clear();
            cin >> index; 
        }
        string temp = ledger[index].csv_format();
        cout << "确认删除账目：\n" << temp <<"\n吗？(y/n)" <<endl;
        char choice;
        cin >> choice;
        while(choice != 'y' && choice != 'n') {
            cerr << "error:无效的输入，请重新输入(y/n)" << endl;
            cin.clear();
            cin >> choice;
        }
        if(choice == 'n') {
            cout << "已取消删除" << endl;
            return; 
        }
        else if(choice == 'y') {
            ledger.erase(ledger.begin() + index );  //删除指定位置的元素
            cout << "已删除账目：" <<temp<< endl;
        }
    }

    void edit_account(int index) {
        if(index < 0 || index >= ledger.size()) {
            cerr << "error:无效的索引" << endl;
            return;
        }
        else {
            string temp = ledger[index].csv_format();
            cout << "你将修改条目：" << temp << endl << endl;
            ledger[index].edit();
            cout << "你将条目："  << temp << endl << "修改为" << ledger[index].csv_format() << endl ;
            cout << "确认修改？(y/n)" << endl;
            char choice;
            cin >> choice;

            while(choice != 'y' && choice != 'n') {
                cerr << "error:无效的输入，请重新输入(y/n)" << endl;
                cin.clear();
                cin >> choice;
            }
            if(choice == 'n') {
                ledger[index] = account(temp);
                cout << "修改已取消" << endl;
                return;
            }
            else if(choice == 'y') {
                cout << "修改已保存" << endl;
                sort();
                return; 
            }
            
        }
    }

    void monthly_statistics(string year_month){
        cout << "\n-------------------------" << endl;
        cout << "正在生成"<< year_month <<"月度统计..." <<"\n-------------------------"<< endl;
        double income = 0;
        double expense = 0;
        for(const auto& account : ledger) {   //遍历ledger容器中的每一个account对象
            if(account.date.substr(0, 7) == year_month) {
                cout << account.csv_format() << endl;
                if(account.amount > 0) {
                    income += account.amount;
                }
                else {
                    expense += account.amount;
                }
            }
        }
        cout <<"-------------------------\n"<< "收入：" << income << endl;
        cout << "支出：" << expense << endl;
        cout << "结余：" << income + expense << endl;
        cout << "-------------------------" << endl;
    }

    //对齐列宽输出表哥
    void formatted_show(){
        cout << "----------------------记账本 Ledger----------------------"<< endl;
        int max_width_description=0;
        int max_width_category=0;
        int max_width_amount=0;
        for(const auto& account : ledger) {   //遍历ledger容器中的每一个account对象
            if(account.description.length() > max_width_description) {
                max_width_description = account.description.length();
            }
            if(account.category.length() > max_width_category) {
                max_width_category = account.category.length(); 
            }
            std::stringstream ss;
            ss << std::fixed << std::setprecision(2);  // 强制保留两位小数
            ss << account.amount;
            if(ss.str().length() > max_width_amount) {
                max_width_amount = ss.str().length();
            }
        }
        // 计算表格宽度
        int total_width = 10 + max_width_description + max_width_category + max_width_amount + 12; // 包括分隔符和框架

        // 输出表格的表头，表头宽度与数据列宽度一致
        cout << "+" << string(total_width-1, '-') << "+" << endl;
        cout << "| " << left << setw(11) << "Date"
            << "| " << left << setw(max_width_description) << "Description"<<" "
            << "| " << left << setw(max_width_category) << "Category"<<" "
            << "| " << right << setw(max_width_amount) << "Amount"
            << " |" << endl;
        cout << "+" << string(total_width-1, '-') << "+" << endl;

        // 输出每一行的数据
        for (const auto& account : ledger) {
            cout << "| " << left << setw(11) << account.date
                << "| " << left << setw(max_width_description) << account.description <<" "
                << "| " << left << setw(max_width_category) << account.category<<" "
                << "| " << right << setw(max_width_amount) << fixed << setprecision(2) << account.amount
                << " |" << endl;
        }

        // 输出表格的底线
        cout << "+" << string(total_width - 1, '-') << "+" << endl;
    }
    int menu(accounts& Ledger) {
        cout << "欢迎使用Ledger" << endl;
        cout << "当前日期: "<< formatted_time()<< endl;
    
        cout << "请选择操作：" << endl;
        cout << "1. 查看账本" << endl;
        cout << "2. 添加条目" << endl;
        cout << "3. 删除条目" << endl;
        cout << "4. 编辑条目" << endl;
        cout << "5. 月度统计" << endl;
        cout << "6. 退出" << endl;
        int choice;
        cin >> choice;  
        while(choice < 1 || choice > 6) {
            cerr << "error:无效的输入，请重新输入" << endl;
            cin.clear();
            cin >> choice; 
        }
        if (choice == 6) {
            cout << "感谢使用Ledger" << endl;
            return 6; 
        }
        else {
            switch(choice) {
                case 1:
                    Ledger.formatted_show();
                    cout << "\n按任意键返回主菜单\n";
                    fflush(stdin);
                    getchar();
                    fflush(stdin);
                    break;
                case 2:
                    Ledger.add_account();
                    cout << "\n按任意键返回主菜单\n";
                    fflush(stdin);
                    getchar();
                    fflush(stdin);
                    break;
                case 3:
                    Ledger.show();
                    cout << "请输入要删除的条目索引：" << endl;
                    int index;
                    cin >> index;
                    Ledger.delete_account(index);
                    cout << "\n按任意键返回主菜单\n";
                    fflush(stdin);
                    getchar();
                    fflush(stdin);
                    break; 
                case 4:
                    Ledger.show();
                    cout << "请输入要编辑的条目索引：(从0开始)" << endl;
                    int index1;
                    cin >> index1;
                    Ledger.edit_account(index1);
                    cout << "\n按任意键返回主菜单\n";
                    fflush(stdin);
                    getchar();
                    fflush(stdin);
                    break;
                case 5:
                    cout << "请输入要查询的年月(格式:YYYY-MM):" << endl;
                    string year_month;
                    cin >> year_month;
                    while(year_month.length()!= 7 || (year_month[4]!='-')){
                        cout << "error:日期格式错误，请重新输入(格式:YYYY-MM)" << endl;
                        cin.clear();
                        cin >> year_month;
                    }
                    Ledger.monthly_statistics(year_month);
                    cout << "\n按任意键返回主菜单\n";
                    fflush(stdin);
                    getchar();
                    fflush(stdin);
                    break;
            } 
        }
        menu(Ledger);
        return 6;
    }
};


int main() {
    system("chcp 65001 > nul");   //设置编码为UTF-8,并禁用输出提示
    accounts Ledger;
    Ledger.menu(Ledger);
    fflush(stdin);
    getchar();
}
