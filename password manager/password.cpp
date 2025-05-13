
//注意编译要用
// g++ -o password password.cpp -std=c++17 -I"C:\Users\TangCheng\Desktop\CodeProjects\C++ software design\DesktopAssistant\password manager\cryptopp" -L"C:\Users\TangCheng\Desktop\CodeProjects\C++ software design\DesktopAssistant\password manager\cryptopp" -lcryptopp
#include "record class.h"
#include "cryptopp/aes.h"
#include "cryptopp/modes.h"
#include "cryptopp/filters.h"
#include "cryptopp/hex.h"
#include "cryptopp/files.h"
#include "cryptopp/osrng.h"

using namespace CryptoPP;
string dir = "passwords.csv";



class AES_Processer {
public:
// AES 加密函数
string Encrypt(const string& plaintext, const CryptoPP::byte* key, const CryptoPP::byte* iv) {
    // 检查密钥和 IV 的长度
    if (key == NULL || iv == NULL) {
        cerr << "密钥或 IV 不能为空" << endl;
        return "";
    }

    // 初始化加密对象
    CBC_Mode<AES>::Encryption encryptor;
    encryptor.SetKeyWithIV(key, AES::DEFAULT_KEYLENGTH, iv, AES::BLOCKSIZE);

    // 创建字符串流进行加密
    string ciphertext;
    StringSource ss(plaintext, true, 
        new StreamTransformationFilter(encryptor,
            new StringSink(ciphertext)
        )
    );

    return ciphertext;
}

// AES 解密函数
string Decrypt(const string& ciphertext, const CryptoPP::byte* key, const CryptoPP::byte* iv) {
    // 检查密钥和 IV 的长度
    if (key == NULL || iv == NULL) {
        cerr << "密钥或 IV 不能为空" << endl;
        return "";
    }

    // 初始化解密对象
    CBC_Mode<AES>::Decryption decryptor;
    decryptor.SetKeyWithIV(key, AES::DEFAULT_KEYLENGTH, iv, AES::BLOCKSIZE);

    // 创建字符串流进行解密
    string plaintext;
    StringSource ss(ciphertext, true, 
        new StreamTransformationFilter(decryptor,
            new StringSink(plaintext)
        )
    );

    return plaintext;
}
};



class Password : public Record {
private:
    string key;
    CryptoPP::byte key_byte[AES::DEFAULT_KEYLENGTH];
public:
    string website,username,password,iv;
    CryptoPP::byte iv_byte[AES::BLOCKSIZE];
    
    void key_input(string input){
        key = input;
        //将key转换为byte数组
        for(int i=0;i<key.length();i++){
            key_byte[i] = static_cast<CryptoPP::byte>(key[i]);
        }
        for(int i=key.length();i<AES::DEFAULT_KEYLENGTH;i++){   //填充key_byte
            key_byte[i] = 0;
        }
    }

    Password(string website,string username,string password, string iv):website(website),username(username),password(password),iv(iv){
        //将iv转换为byte数组
        HexDecoder decoder;
        decoder.Attach(new ArraySink(iv_byte, sizeof(iv_byte)));
        decoder.Put((const CryptoPP::byte*)iv.data(), iv.size());
        decoder.MessageEnd();
    }
    Password(string password_line){
        stringstream ss(password_line);
        getline(ss,website,',');
        getline(ss,username,',');
        getline(ss,password,',');
        getline(ss,iv,',');
        //将iv转换为byte数组
        HexDecoder decoder;
        decoder.Attach(new ArraySink(iv_byte, sizeof(iv_byte)));
        decoder.Put((const CryptoPP::byte*)iv.data(), iv.size());
        decoder.MessageEnd();
        }
    string csv_format() const{
        return website + "," + username + "," + password + "," + iv + "\n";
    }
    void show() const{
        cout <<  website << "  " << username << "  " << "******" << endl;
    }

    void edit(){
        cout << "请提供密码库密钥：";
        string user_key;
        cin >> user_key;
        edit(key);
    }
    void edit(string key){
        cout << "请输入新的网站名称：";
        cin >> website;
        cout << "请输入新的用户名：";
        cin >> username;
        cout << "请输入新的密码：";
        string password_plaintext;
        cin >> password_plaintext;
        char choice='n';
        cout << "是否使用密码(y/n)：";
        cin >> choice;
        while(choice!='y' &&  choice!='n'){
            cout << "无效的输入，请重新输入：";
            cin >> choice;
        }
        if(choice=='n'){
            cout << "操作取消" << endl;
            return;
        }

        // 生成随机的 IV
        AutoSeededRandomPool rng;
        CryptoPP::byte *iv_byte = new CryptoPP::byte[AES::BLOCKSIZE];
        rng.GenerateBlock(iv_byte, sizeof(iv_byte));

        // 将 IV_byte 转换为十六进制字符串IV
        HexEncoder encoder(new StringSink(iv));
        encoder.Put(iv_byte, sizeof(iv_byte));
        encoder.MessageEnd();

        // 加密密码
        AES_Processer aes_processor;
        password = aes_processor.Encrypt(password_plaintext, key_byte, iv_byte);
        

    }

};


class Passwords : public Records {
private:
    string user_key;
    CryptoPP::byte key_byte[AES::DEFAULT_KEYLENGTH];
public: 
    vector<Password> _passwords;
    bool Isempty(string& key){
        while(key == ""){
                cout << "密码库密钥不能为空，请重新输入：";
                cin >> key;
            }
        return true;
    }
    string set_key(){   //设置密码库密钥,返回密钥
    start:
        string key="";
        cin >> key;
        Isempty(key);

        cout << "请再次输入密码库密钥：";
        string key2="";
        cin >> key2;
        Isempty(key2);
            
        while(key.compare(key2)!=0){
                cout << "两次输入的密码库密钥不一致，请重新输入：";
                goto start;
        }
        return key;
    }
    int file_check(){   //检查文件是否存在,如果不存在则创建。注意会开不会关闭文件流，且执行完毕后文件流在第二行。
        if(!file_init()) {   //文件(目录)初始化
            cerr << "error:文件初始化失败" << endl;
            return -1;
        }
        ifstream check(dir);
        if (!check) {
            ofstream create(dir);
            create << "Website,Username,Password,IV" << endl;
            create.close();
        }
        check.close();

        file.open(dir, ios::in);
        if (!file.is_open()) {  //文件打开失败
            cerr << "error:文件打开失败" << endl;
            return -1;
        }

        string line;
        fflush(stdin);
        getline(file, line);
        if(line.compare("Website,Username,Password,IV")!=0){
            cerr << "error:密码本文件损坏" << endl;
            return -1;
        }
        fflush(stdin);
        return 0;
    }

    Passwords(){
        if(file_check()!=0){   //文件检查
            cerr << "error:程序关键错误，请检查文件权限或尝试删除文件后重试" << endl;
            return;
        }
        file.close();
        file.open(dir, ios::in); //先打开才能getline!!!!
        string line;
        getline(file, line); //读取第一行
        if(line.compare("Website,Username,Password,IV")!=0){   //检查文件
            cerr << "error:密码本文件损坏" << endl;
            file.close();
            return;
        }
        //读取密钥
        while(!getline(file, line) || line.compare("")==0){
            cout << "请设置密码库密钥：";
            file.close();
            file.open(dir, ios::out);
            file << "Website,Username,Password,IV" << endl;
            file << set_key() << endl;
            file.flush();
            file.close();
            if(file_check()!=0){   //文件检查
            cerr << "error:程序关键错误，请检查文件权限或尝试删除文件后重试" << endl;
            file.close();
            return;}
        }
        file.close();
        user_key = line;
        file.open(dir, ios::in); //先打开才能getline!!!!
        getline(file, line); //读取第一行
        getline(file, line); //读取第二行
        while(getline(file, line)) {
            if(line.compare("")==0){   //跳过空行
                continue;
            }
            Password new_password(line);
            _passwords.push_back(new_password); 
            }
        sort();

        cout << "调试：密码本加载完成，输出密码本数组" << endl;
        for(const auto& Password : _passwords) {   //遍历_passwords数组,将每个Password对象的show()函数返回值写入文件
            Password.show();   //调用Password类的show()函数
        }

        //将user_key转换为byte数组
        for(int i=0;i<user_key.length();i++){
            key_byte[i] = static_cast<CryptoPP::byte>(user_key[i]);
        }
        for(int i=user_key.length();i<AES::DEFAULT_KEYLENGTH;i++){   //填充key_byte
            key_byte[i] = 0;
        }
        file.close();
    }
    ~Passwords() {   //析构函数,将_passwords数组覆写文件
        file.open(dir, ios::out);
        file << "Website,Username,Password,IV" << endl;
        file << user_key << endl;
        for(const auto& Password : _passwords) {   //遍历_passwords数组,将每个Password对象的csv_format()函数返回值写入文件
            file << Password.csv_format() << endl;
        }
        file.flush();
        file.close();
        cout << "\n----------已保存更改----------" << endl;
    }
    void sort(){   //按照website排序
        std::sort(_passwords.begin(), _passwords.end(), [](const Password& a, const Password& b) {   //lambda表达式,按照website排序
            return a.website < b.website;   //升序排序
        });
    }

    void show() const{
        for (const auto& Password : _passwords) {   //遍历_passwords数组,将每个Password对象的show()函数返回值写入文件
            Password.show();   //调用Password类的show()函数
        }
    }   
    
    void show(int index) {   //显示指定index的Password对象的show()函数返回值
        // cout << "----------密码本----------" << endl;
        // cout << "Website,Username,Password" << endl;
        // _passwords[index].show();   //调用Password类的show()函数
        // cout << "输入密码库密钥以查看密码：" << endl;
        // string key;
        // cin >> key;

        // 解密密码
        AES_Processer aes_processor;
        string decrypted_password = aes_processor.Decrypt(_passwords[index].password, key_byte, _passwords[index].iv_byte);
        cout << _passwords[index].website << _passwords[index].username << decrypted_password << endl;
        // cout << "------------------------" << endl;
    }


    void add(){
        cout << "请提供密码库密钥：";
        string key;
        cin >> key; 
        add(key);
    }
    void add(string key){
        string website,username,password,iv;
        cout << "请输入网站名称：";
        cin >> website;
        while(serach(website)!=-1){   //如果已经存在,则提示并重新输入
            cout << "该网站已经存在,请重新输入。（注意：仅大小写不同的网址被视为同一网站）" << endl;
            cin >> website;
        }
        cout << "请输入用户名：";
        cin >> username;
        cout << "请输入密码：";
        string password_plaintext;
        cin >> password_plaintext;
        char choice='n';
        cout << "确认密码(y/n)：" << password_plaintext << endl;
        cin >> choice;
        while(choice!='y' &&  choice!='n'){
            cout << "无效的输入，请重新输入：";
            cin >> choice;
        }
        if(choice=='n'){
            cout << "操作取消" << endl;
            return;
        }

        // 生成随机的 IV
        AutoSeededRandomPool rng;
        CryptoPP::byte iv_byte[AES::BLOCKSIZE];
        rng.GenerateBlock(iv_byte, sizeof(iv_byte));
        
        // 将 IV_byte 转换为十六进制字符串IV
        HexEncoder encoder(new StringSink(iv));
        encoder.Put(iv_byte, sizeof(iv_byte));
        encoder.MessageEnd();

        // 加密密码
        AES_Processer aes_processor;
        password = aes_processor.Encrypt(password_plaintext, key_byte, iv_byte);
        Password new_password(website,username,password,iv);
        _passwords.push_back(new_password);
        sort();
        cout << "----------添加成功----------" << endl;

    }
    int serach(string website){   //按照website查找,返回index数组,如果没有找到,返回-1
        transform(website.begin(), website.end(), website.begin(), ::tolower);
        for(int i=0;i<_passwords.size();i++){   //遍历_passwords数组,将每个Password对象的website与输入的website比较,且大小写不敏感
            transform(_passwords[i].website.begin(), _passwords[i].website.end(), _passwords[i].website.begin(), ::tolower);
            if(_passwords[i].website.compare(website)==0){   //如果找到,返回index
                return i;
            }
        }
        return -1;   //如果没有找到,返回-1
    }

    void del(string website){   //按照website删除,如果没有找到,返回-1
        int index = serach(website);
        del(index);
    }
    void del(int index){   //按照index删除,如果没有找到,返回-1
        if(index==-1){   //如果没有找到,返回-1
            cout << "未找到该网站" << endl;
            return;
        }
        _passwords.erase(_passwords.begin() + index);   //删除_passwords数组中index位置的元素
        sort();   //排序
        cout << "----------删除成功----------" << endl;
    }
    void edit(int index){
        _passwords[index].edit();   //调用Password类的edit()函数
        sort();   //排序
        cout << "----------修改成功----------" << endl;
    }

    void pause() {  //保存并暂停程序，按任意键继续
        cout << "\n按任意键返回主菜单\n";
        fflush(stdin);
        getchar();
        fflush(stdin);
    }

    // int menu_old(Passwords& _passwords) {
    //     cout << "欢迎使用Passwords" << endl;
    //     cout << "请选择操作：" << endl;
    //     cout << "1. 查看全部密码" << endl;
    //     cout << "2. 添加密码" << endl;
    //     cout << "3. 删除密码" << endl;
    //     cout << "4. 编辑密码" << endl;
    //     cout << "5. 搜索密码" << endl;
    //     cout << "6. 保存并退出" << endl;  
    //     int choice;
    //     cin >> choice;  
    //     while(choice < 1 || choice > 6) {
    //         cerr << "error:无效的输入，请重新输入" << endl;
    //         fflush(stdin);
    //         cin >> choice; 
    //     }
    //     if (choice == 6) {
    //         cout << "感谢使用Passwords，请按任意键以安全保存并退出" << endl;
    //         return 6; 
    //     }
    //     //密码校验环节
    //     else if (choice != 1) {
    //         cout << "执行该操作需要输入密码库密钥：" << endl;
    //         string key_temp;
    //         cin >> key_temp;
    //         while(key_temp.compare(_passwords.user_key)!=0){
    //             cout << "密码错误，请重新输入：" << endl;
    //             cin >> key_temp;
    //         }
    //     }
    //     else {
    //         switch(choice) {
    //             case 1:
    //                 _passwords.show();
    //                 pause();
    //                 break;
    //             case 2:
    //                 _passwords.add();
    //                 pause();
    //                 break;
    //             case 3:
    //                 cout << "请输入要删除的网站名称；或进入输入#进入索引删除模式\n输入-1返回主菜单" << endl;
    //                 string temp_input;
    //                 cin >> temp_input;
    //                 if(temp_input.compare("-1")==0){   //返回主菜单
    //                     return 0;
    //                 }
    //                 else if(temp_input.compare("#")==0){   //进入索引删除模式
    //                     _passwords.show();
    //                     cout << "请输入要删除的条目索引：(从0开始)" << endl;
    //                     int index;
    //                     cin >> index;
    //                     _passwords.del(index);
    //                     pause();
    //                     break;
    //                 }
    //                 else{   //按照website删除
    //                     _passwords.del(website);
    //                     pause();
    //                     break;
    //                 }
    //             case 4:
    //                 _passwords.show();
    //                 cout << "请输入要编辑的条目索引：(从0开始)" << endl;
    //                 int index1;
    //                 cin >> index1;
    //                 _passwords.edit
    //                 pause();
    //                 break;
    //             case 5:
    //                 cout << "请输入要查询的年月日(格式:YYYY-MM-DD):" << endl;
    //                 string year_month_day;
    //                 cin >> year_month_day;
    //                 while(year_month_day.length()!= 10 || (year_month_day[4]!='-') || (year_month_day[7]!='-')){
    //                     cout << "error:时间格式错误，请重新输入(格式:YYYY-MM-DD)" << endl;
    //                     fflush(stdin);
    //                     cin >> year_month_day;
    //                 }
    //                 _reminders.daily_statistics(year_month_day);
    //                 pause();
    //                 break;
    //         } 
    //     }
    //     fflush(stdin);
    //     menu(_passwords);
    // }

    void unlock(){   //密码校验环节
        int try_times = 0;   //尝试次数
        cout << "该操作需要输入密码库密钥：" << endl;
        string key_temp;
        cin >> key_temp;
        while(key_temp.compare(user_key)!=0){
            if(try_times++ >= 3){   //尝试次数超过3次,退出程序
                cout << "尝试次数过多，程序将自动退出并锁定，请执行恢复模式" << endl;
                exit(-1);
            }
            else{}
            cout << "密码错误，请重新输入：" << endl;
            cin >> key_temp;
            }
        }

    int menu(Passwords& _passwords){   //菜单
        cout << "欢迎使用Passwords" << endl;
        cout <<"----------密码本----------" << endl;
        _passwords.show();   //显示密码本
        cout <<"--------------------------" << endl;
        cout << "请选择操作：" << endl;
        cout << "1. 添加密码" << endl;
        cout << "2. 定位密码（可进行查看、编辑、删除等功能）" << endl;
        cout << "3. 修改密码库密钥" << endl;
        cout << "4. 保存并退出" << endl;
        int choice;
        cin >> choice;
        while(choice < 1 || choice > 4) {
            cerr << "error:无效的输入，请重新输入" << endl;
            fflush(stdin);
            cin >> choice;
        }
        if (choice == 4) {
            cout << "感谢使用Passwords，请按任意键以安全保存并退出" << endl;
            return 6;
        }
        else{
            
            switch(choice) {
                case 1:
                    _passwords.add();
                    pause();
                    break; 
                case 2:
                    {int index_input;
                    cout << "请输入要定位的网站名称(或直接输入#进入索引模式)：" << endl;
                    string website_input;
                    cin >> website_input;
                    if(website_input.compare("#")==0){   //进入索引模式
                        _passwords.show();
                        cout << "请输入要定位的条目索引：(从0开始)" << endl;
                        cin >> index_input;
                    }
                    else{  //按照website查找
                        index_input = _passwords.serach(website_input);
                        while(index_input==-1){   //如果没有找到,返回-1
                            cout << "未找到该网站，请检查输入后重试" << endl;
                            cout << "请输入要定位的网站名称：" << endl;
                            cin >> website_input;
                            index_input = _passwords.serach(website_input);
                        }       
                        } 
                    cout << "网址：" << _passwords._passwords[index_input].website << "  " << "用户名：" << _passwords._passwords[index_input].username << endl;
                    cout << "请选择操作：" << endl;
                    cout << "1. 查看密码" << endl;
                    cout << "2. 编辑密码" << endl;
                    cout << "3. 删除密码" << endl;
                    cout << "4. 返回主菜单" << endl;
                    int choice2;
                    cin >> choice2;
                    while(choice2 < 1 || choice2 > 4) {
                        cerr << "error:无效的输入，请重新输入" << endl;
                        fflush(stdin);
                        cin >> choice2;
                    }
                    if (choice2 == 4) {
                        pause();
                        return 0;
                    }
                    else{
                        _passwords.unlock();
                        switch(choice2) {
                            case 1:
                                _passwords.show(index_input);   //显示指定index的Password对象的show()函数返回值
                            
                                pause();
                                break;
                            case 2:
                                _passwords.edit(index_input);   //编辑指定index的Password对象的edit()函数返回值
                                
                                pause();
                                break;
                            case 3:
                                _passwords.del(index_input);   //删除指定index的Password对象的del()函数返回值
                                
                                pause();
                                break; 
                        } 
                        return 0;
                    }}
                case 3:
                    {string key_temp,key_temp2;
                    _passwords.unlock();
                    cout << "请输入新的密码库密钥：" << endl;
                    cin >> key_temp;
                    cout << "请再次输入密码库密钥：" << endl;
                    cin >> key_temp2;
                    while(key_temp.compare(key_temp2)!=0){
                        cout << "两次输入的密码库密钥不一致，请重新输入：" << endl;
                        cin >> key_temp;
                        cout << "请再次输入密码库密钥：" << endl;
                        cin >> key_temp2;
                    }
                    user_key = key_temp;   //修改user_key
                    _passwords.~Passwords();   //析构函数,将_passwords数组覆写文件
                    cout << "密码库密钥修改成功" << endl;
                    pause();
                    break;}
            }
        }
        fflush(stdin);
        menu(_passwords);
    }
};
    

int main() {
    system("chcp 65001 > nul");   //设置编码为UTF-8,并禁用输出提示
    Passwords _passwords;
    _passwords.menu(_passwords);
    fflush(stdin);
    getchar();
    return 0;
}