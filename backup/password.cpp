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
public:
    string website,username,password,iv;
    
    void key_input(string input){
        key = input;
    }

    Password(string website,string username,string password):website(website),username(username),password(password){}
    Password(string password_line){
        stringstream ss(password_line);
        getline(ss,website,',');
        getline(ss,username,',');
        getline(ss,password,',');
    }
    string csv_format() const{
        return website + "," + username + "," + password + "," + iv + "\n";
    }
    void show() const{
        cout <<  website << username << "******" << endl;
    }

    void edit(){
        cout << "请提供密码库密钥：";
        string key;
        cin >> key;
        edit(key);
    }
    void edit(string key){
        //将key转换为byte数组
        CryptoPP::byte key_byte[AES::DEFAULT_KEYLENGTH];
        for(int i=0;i<AES::DEFAULT_KEYLENGTH;i++){
            key_byte[i] = static_cast<CryptoPP::byte>(key[i]);
        }


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
        CryptoPP::byte *iv_byte;
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
public: 
    vector<Password> _passwords;
    Passwords(){
        if(!file_init()) {   //文件(目录)初始化
            cerr << "error:文件初始化失败" << endl;
            return;
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
            return;
        }

        string line;
        fflush(stdin);
        getline(file, line);
        if(line.compare("Website,Username,Password,IV")!=0){
            cout << "error:密码本文件损坏" << endl;
            return;
        }
        fflush(stdin);
        while(getline(file, line)) {
            Password new_password(line);
            _passwords.push_back(new_password); 
            }
        sort();
    }
    ~Passwords() {   //析构函数,将_passwords数组覆写文件
        ofstream file(dir, ios::out);
        file << "Website,Username,Password,IV" << endl;
        for(const auto& Password : _passwords) {   //遍历_passwords数组,将每个Password对象的csv_format()函数返回值写入文件
            file << Password.csv_format() << endl;
        }
        file.close();
        cout << "\n----------已保存更改----------" << endl;
    }
    void sort(){   //按照website排序
        std::sort(_passwords.begin(), _passwords.end(), [](const Password& a, const Password& b) {   //lambda表达式,按照website排序
            return a.website < b.website;   //升序排序
        });
    }

    void show() const{
        cout << "----------密码本----------" << endl;
        cout << "Website,Username,Password" << endl;
        for (const auto& Password : _passwords) {   //遍历_passwords数组,将每个Password对象的show()函数返回值写入文件
            Password.show();   //调用Password类的show()函数
        }
        cout << "------------------------" << endl;
    }   
    
    void show(int index) const{   //显示指定index的Password对象的show()函数返回值
        cout << "----------密码本----------" << endl;
        cout << "Website,Username,Password" << endl;
        _passwords[index].show();   //调用Password类的show()函数
        cout << "输入密码库密钥以查看密码：" << endl;
        string key;
        cin >> key;
        //将key转换为byte数组
        CryptoPP::byte key_byte[AES::DEFAULT_KEYLENGTH];
        for(int i=0;i<AES::DEFAULT_KEYLENGTH;i++){
            key_byte[i] = static_cast<CryptoPP::byte>(key[i]);
        }
        //将iv转换为byte数组
        CryptoPP::byte iv_byte[AES::BLOCKSIZE];
        HexDecoder decoder(new StringSink((char*)iv_byte, sizeof(iv_byte)));
        decoder.Put((CryptoPP::byte*)_passwords[index].iv.data(), _passwords[index].iv.size());
        decoder.MessageEnd();

        // 解密密码
        AES_Processer aes_processor;
        string decrypted_password = aes_processor.Decrypt(_passwords[index].password, key_byte, iv_byte);
        cout << "密码：" << decrypted_password << endl;
        cout << "------------------------" << endl;
    }


    void add(){
        cout << "请提供密码库密钥：";
        string key;
        cin >> key; 
        add(key);
    }
    void add(string key){
        string website,username,password;
        cout << "请输入网站名称：";
        cin >> website;
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
        HexEncoder encoder(new StringSink(password));
        encoder.Put(iv_byte, sizeof(iv_byte));
        encoder.MessageEnd();

        // 加密密码
        AES_Processer aes_processor;
        password = aes_processor.Encrypt(password_plaintext, key_byte, iv_byte);
        Password new_password(website,username,password);
        _passwords.push_back(new_password);
        sort();
        cout << "----------添加成功----------" << endl;

    }
    
};
    void serach(){   //按照website查找,返回index数组,如果没有找到,返回-1
        string website;
        cout << "请输入网站名称：";
        cin >> website;
        for(int i=0;i<_passwords.size();i++){   //遍历_passwords数组,将每个Password对象的website与输入的website比较,且大小写不敏感
            if(_passwords[i].website.compare(website)==0){   //如果找到,返回index
                show(i);   //调用show()函数,显示指定index的Password对象的show()函数返回值
                return;
            }
        }
int main(){
    Passwords _passwords;
    string key;
    return 0; 
}