#include <iostream>
#include <windows.h>
#include <cstdlib>  

using namespace std;

int main() {
    #ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    #endif
    SetConsoleTitleW(L"开始菜单Start Menu");
    while (true) {
        cout << "\n========= 主菜单Menu =========\n";
        cout << "1. 打开密码管理器Password Manager\n";
        cout << "2. 打开记账本Ledger\n";
        cout << "3. 打开课程提醒器Course Reminder\n";
        cout << "请选择：";

        int choice;
        cin >> choice;

        switch (choice) {
            case 1:
                system("start password.exe"); 
                break;
            case 2:
                system("start ledger.exe");   
                break;
            case 3:
                system("start reminder.exe");
            case 4:
                cout << "退出程序\n";
                return 0;
            default:
                cout << "无效选择，请重新输入\n";
        }

        cout << "程序运行中...\n" << endl;
        cout << "按任意键返回主菜单...\n" << endl;
        cin.ignore();
        cin.get();

    }
}
