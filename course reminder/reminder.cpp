#include "record class.h"

string dir ="reminder.csv";

class Reminder : public Record {
public:
    string time;
    string content;
    int priority;
    string csv_format() const {   
        return time + "," + content + "," + to_string(priority);
    }
    Reminder(string time, string content, int priority) : time(time), content(content), priority(priority) {};
    Reminder(string reminder_line){
        stringstream ss(reminder_line);
        string token;
        getline(ss, token, ',');
        time = token;
        getline(ss, token, ',');
        content = token;
        getline(ss, token, ',');
        priority = stoi(token);
    };
    void show() const override {
        cout << csv_format() << endl; 
    }
    void edit() override {
        cout << "请输入新的时间：（YYYY-MM-DD HH:MM）";
        fflush(stdin);
        getline(cin, time);
        while(time.length()!= 16 || time[4]!= '-' || time[7]!= '-' || time[10]!=' '|| (time[13]!= ':'&& time[13]!= '：')){
            cout << "error: 时间格式错误，请重新输入" << endl; 
            fflush(stdin);
            getline(cin, time);
        }
        if(time[13]= '：'){
            time[13] = ':';
        }
        fflush(stdin);
        cout << "请输入新的内容："; 
        getline(cin, content);
        while(content.empty()){
            cout << "error: 内容不能为空，请重新输入" << endl;
            cout << "请输入新的内容：";
            fflush(stdin);
            getline(cin, content); 
        }
        fflush(stdin);
        cout << "请输入新的优先级：(1-3)";
        cin >> priority;
        while(priority<1 || priority>3){
            cout << "error: 优先级超出范围，请重新输入" << endl;
            cout << "请输入新的优先级：(1-3)";
            cin >> priority; 
        }
        fflush(stdin);
    }

};  

class Reminders : public Records {
public:
    vector <Reminder> _reminders;
    Reminders(){
        if(!file_init()) {   //文件(目录)初始化
            cout << "error:文件初始化失败" << endl;
            return;
        }
        ifstream check(dir);
        if (!check) {
            ofstream create(dir);
            create << "Time,Content,Priority" << endl;
            create.close();
        }
        check.close();

        file.open(dir, ios::in);
        if (!file.is_open()) {  //文件打开失败
            cout << "error:文件打开失败" << endl;
            return;
        }

        string line;
        fflush(stdin);
        getline(file, line);
        if(line.compare("Time,Content,Priority")!=0){
            cout << "error:账本文件损坏" << endl;
            return;
        }
        fflush(stdin);
        while(getline(file, line)) {
            Reminder new_reminder(line);
            _reminders.push_back(new_reminder);
            }
        sort();
    }
    ~Reminders() {   //析构函数,将reminders数组覆写文件
        ofstream file(dir, ios::out);
        file << "Time,Content,Priority" << endl;
        for(const auto& Reminder : _reminders) {   //遍历reminders容器中的每一个Reminder对象
            file << Reminder.csv_format() << endl;
        }
        file.close();
        cout << "\n----------已保存更改----------" << endl;
    }
    void sort() {
        std::sort(_reminders.begin(), _reminders.end(), [](const Reminder& a, const Reminder& b) {
            return a.time < b.time;
        });
    }
    void show() const {
        cout <<endl<< "-----------课程提醒 Reminder-----------"<< endl;
        cout << "Time,Content,Priority" << endl;
        int num = 0;
        for(const auto& Reminder : _reminders) {   //遍历Reminders容器中的每一个Reminder对象
            cout << setw(2) << num << " " ;    
            Reminder.show();
            num++;
        } 
        cout << "----------------------------------"<< endl;
    }
    void add_Reminder() {
        cout << "添加新的条目" << endl;
        string time, content;
        int priority;
        cout << "请输入时间（YYYY-MM-DD HH:MM）：" << endl;
        fflush(stdin);
        getline(cin, time);
        while(time.length()!= 16 || time[4]!= '-' || time[7]!= '-' || time[10]!=' '|| (time[13]!= ':'&& time[13]!= '：')){
            cout << "error: 时间格式错误，请重新输入" << endl; 
            fflush(stdin);
            getline(cin, time);
        }
        if(time[13]= '：'){
            time[13] = ':';
        }
        fflush(stdin);
        cout << "请输入课程内容：" << endl;
        getline(cin, content);
        while(content.empty()){
            cout << "error: 内容不能为空，请重新输入" << endl;
            fflush(stdin);
            getline(cin, content);
        }
        cout << "请输入优先级（1-3）：" << endl;
        cin >> priority;
        while(priority<1 || priority>3){
            cout << "error: 优先级超出范围，请重新输入" << endl;
            fflush(stdin);
            cin >> priority;
        }
        Reminder new_Reminder(time, content, priority);
        _reminders.push_back(new_Reminder);    //尾部插入元素
        sort();
        cout << "已添加" << endl;
    }

//删除指定位置的账目，index从0开始，对应Reminders容器中的第一个元素
    void delete_Reminder(int index) {
        while (index < -1 || index >= _reminders.size()) {
            cout << "error:无效的索引，请重新输入" << endl;
            fflush(stdin);
            cin >> index; 
        }
        if(index == -1) {
            cout << "已取消删除" << endl;
            return;
        }
        string temp = _reminders[index].csv_format();
        cout << "确认删除账目：\n" << temp <<"\n吗？(y/n)" <<endl;
        char choice;
        cin >> choice;
        while(choice != 'y' && choice != 'n') {
            cerr << "error:无效的输入，请重新输入(y/n)" << endl;
            fflush(stdin);
            cin >> choice;
        }
        if(choice == 'n') {
            cout << "已取消删除" << endl;
            return; 
        }
        else if(choice == 'y') {
            _reminders.erase(_reminders.begin() + index );  //删除指定位置的元素
            cout << "已删除账目：" <<temp<< endl;
        }
    }

    void edit_Reminder(int index) {
        if(index < 0 || index >= _reminders.size()) {
            cerr << "error:无效的索引" << endl;
            return;
        }
        else {
            string temp = _reminders[index].csv_format();
            cout << "你将修改条目：" << temp << endl << endl;
            _reminders[index].edit();
            cout << "你将条目："  << temp << endl << "修改为" << _reminders[index].csv_format() << endl ;
            cout << "确认修改？(y/n)" << endl;
            char choice;
            fflush(stdin);
            cin >> choice;

            while(choice != 'y' && choice != 'n') {
                cerr << "error:无效的输入，请重新输入(y/n)" << endl;
                fflush(stdin);
                cin >> choice;
            }
            if(choice == 'n') {
                _reminders[index] = Reminder(temp);
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

    void daily_statistics(string year_month_day){   //按日期统计
        cout << "\n-------------------------" << endl;
        cout << "正在生成"<< year_month_day <<"提醒事项..." <<"\n-------------------------"<< endl;
        for(const auto& Reminder : _reminders) {   //遍历_reminders容器中的每一个Reminder对象
            if(Reminder.time.substr(0, 10) == year_month_day) {
                cout << Reminder.csv_format() << endl;
            }
        }
        cout << "-------------------------" << endl;
    }

    //对齐列宽输出表哥
    void formatted_show(){
        cout << "------------- 课程提醒Reminder -------------"<< endl;
        int max_width_content = 0; // 内容列的最大宽度
        for(const auto& Reminder : _reminders) {   //遍历_reminders容器中的每一个Reminder对象
            if(Reminder.content.length() > max_width_content) {
                max_width_content = Reminder.content.length();
            }
        }
        // 计算表格宽度
        int total_width = 16 + max_width_content + 8 + 9; // 包括分隔符和框架

        // 输出表格的表头，表头宽度与数据列宽度一致
        cout << "+" << string(total_width-1, '-') << "+" << endl;
        cout << "| " << left << setw(16) << "Time" << " "
            << "| " << left << setw(max_width_content) << "Content"<<" "
            << "| " << right << setw(8) << "Priority"
            << " |" << endl;
        cout << "+" << string(total_width-1, '-') << "+" << endl;

        // 输出每一行的数据
        for (const auto& Reminder : _reminders) {
            cout << "| " << left << setw(16) << Reminder.time << " "
                << "| " << left << setw(max_width_content) << Reminder.content <<" "
                << "| " << right << setw(8) << Reminder.priority
                << " |" << endl;
        }

        // 输出表格的底线
        cout << "+" << string(total_width - 1, '-') << "+" << endl;
    }

    void pause() {  //暂停程序，按任意键继续
        cout << "\n按任意键返回主菜单\n";
        fflush(stdin);
        getchar();
        fflush(stdin);
    }

    int menu(Reminders& _reminders) {

        cout << "欢迎使用Reminder" << endl;
        cout << "当前日期: "<< formatted_time()<< endl;
    
        cout << "请选择操作：" << endl;
        cout << "1. 查看全部提醒事项" << endl;
        cout << "2. 添加提醒事项" << endl;
        cout << "3. 删除提醒事项" << endl;
        cout << "4. 编辑提醒事项" << endl;
        cout << "5. 查看某日提醒事项" << endl;
        cout << "6. 保存并退出" << endl;
        int choice;
        cin >> choice;  
        while(choice < 1 || choice > 6) {
            cerr << "error:无效的输入，请重新输入" << endl;
            fflush(stdin);
            cin >> choice; 
        }
        if (choice == 6) {
            cout << "感谢使用_reminders，请按任意键以安全保存并退出" << endl;
            return 6; 
        }
        else {
            switch(choice) {
                case 1:
                    _reminders.formatted_show();
                    pause();
                    break;
                case 2:
                    _reminders.add_Reminder();
                    pause();
                    break;
                case 3:
                    _reminders.show();
                    cout << "请输入要删除的条目索引：(从0开始)\n输入-1返回主菜单" << endl;
                    int index;
                    cin >> index;
                    _reminders.delete_Reminder(index);
                    pause();
                    break; 
                case 4:
                    _reminders.show();
                    cout << "请输入要编辑的条目索引：(从0开始)" << endl;
                    int index1;
                    cin >> index1;
                    _reminders.edit_Reminder(index1);
                    pause();
                    break;
                case 5:
                    cout << "请输入要查询的年月日(格式:YYYY-MM-DD):" << endl;
                    string year_month_day;
                    cin >> year_month_day;
                    while(year_month_day.length()!= 10 || (year_month_day[4]!='-') || (year_month_day[7]!='-')){
                        cout << "error:时间格式错误，请重新输入(格式:YYYY-MM-DD)" << endl;
                        fflush(stdin);
                        cin >> year_month_day;
                    }
                    _reminders.daily_statistics(year_month_day);
                    pause();
                    break;
            } 
        }
        fflush(stdin);
        menu(_reminders);
    }
};

int main() {
    system("chcp 65001 > nul");   //设置编码为UTF-8,并禁用输出提示
    Reminders _reminders;
    _reminders.menu(_reminders);
    fflush(stdin);
    getchar();
    return 0;
}