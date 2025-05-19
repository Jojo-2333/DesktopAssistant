//注意编译要用
// g++ -o password password.cpp -std=c++17 -I"C:\Users\TangCheng\Desktop\CodeProjects\C++ software design\DesktopAssistant\password manager\cryptopp" -L"C:\Users\TangCheng\Desktop\CodeProjects\C++ software design\DesktopAssistant\password manager\cryptopp" -lcryptopp

#ifdef _WIN32
#include <windows.h>
#endif

#include <algorithm>
#include <cctype>
#include "record class.h"
#include "cryptopp/aes.h"
#include "cryptopp/modes.h"
#include "cryptopp/filters.h"
#include "cryptopp/hex.h"
#include "cryptopp/osrng.h"

using namespace std;
using namespace CryptoPP;

string dir = "passwords.csv"; // 外部变量使用时需要声明

class AES_Processor {
public:
    // AES 加密：输入明文和 key/iv，返回原始二进制密文
    vector<CryptoPP::byte> Encrypt(const string& plaintext, const CryptoPP::byte* key, const CryptoPP::byte* iv) {
        CBC_Mode<AES>::Encryption enc;
        enc.SetKeyWithIV(key, AES::DEFAULT_KEYLENGTH, iv, AES::BLOCKSIZE);
        vector<CryptoPP::byte> cipher;
        StringSource ss(plaintext, true,
            new StreamTransformationFilter(enc,
                new VectorSink(cipher)
            )
        );
        return cipher;
    }

    // AES 解密：输入二进制密文和 key/iv，返回明文
    string Decrypt(const vector<CryptoPP::byte>& cipher, const CryptoPP::byte* key, const CryptoPP::byte* iv) {
        CBC_Mode<AES>::Decryption dec;
        dec.SetKeyWithIV(key, AES::DEFAULT_KEYLENGTH, iv, AES::BLOCKSIZE);
        string plain;
        StringSource ss(cipher.data(), cipher.size(), true,
            new StreamTransformationFilter(dec,
                new StringSink(plain)
            )
        );
        return plain;
    }
};

struct Entry : public Record {
    string website, username;
    vector<CryptoPP::byte> cipher, iv;
    // hex 编码缓存
    string hex_cipher, hex_iv;

    Entry() = default;

    // 构造新条目：给定 plaintext，key_byte
    Entry(const string& site, const string& user,
          const string& plain, const CryptoPP::byte* key_byte)
        : website(site), username(user)
    {
        // 生成随机 IV
        AutoSeededRandomPool rng;
        iv.resize(AES::BLOCKSIZE);
        rng.GenerateBlock(iv.data(), iv.size());

        // 加密
        AES_Processor proc;
        cipher = proc.Encrypt(plain, key_byte, iv.data());

        // 转 hex 存储
        StringSource(cipher.data(), cipher.size(), true,
            new HexEncoder(
                new StringSink(hex_cipher), false));
        StringSource(iv.data(), iv.size(), true,
            new HexEncoder(
                new StringSink(hex_iv), false));
    }

    // 从 CSV 行读取
    Entry(const string& line) {
        string cs, is;
        stringstream ss(line);
        getline(ss, website, ',');
        getline(ss, username, ',');
        getline(ss, hex_cipher, ',');
        getline(ss, hex_iv, ',');

        // hex 解码回二进制
        StringSource(hex_cipher, true,
            new HexDecoder(
                new VectorSink(cipher)));
        StringSource(hex_iv, true,
            new HexDecoder(
                new VectorSink(iv)));
    }

    // 将条目格式化为 CSV
    string ToCsv() const {
        return website + "," + username + "," + hex_cipher + "," + hex_iv + "\n";
    }
    void show() const {
        cout << website << "  " << username << "  " << "******" << endl; 
    }
    // 列表显示（隐藏密码）
    void ShowSummary() const {
        show();
    }

    // 查看并解密
    void ShowFull(const CryptoPP::byte* key_byte) const {
        AES_Processor proc;
        string plain = proc.Decrypt(cipher, key_byte, iv.data());
        cout << website << "  " << username << "  " << plain << endl;
    }
    void edit() {
       cout << "请输入密码库密钥" << endl;
       string key;
       cin >> key;
       const CryptoPP::byte* key_byte = reinterpret_cast<const CryptoPP::byte*>(key.c_str());
       edit(key_byte); 
    }
    void edit(const CryptoPP::byte* key_byte) {
        string new_website, new_username, new_password;
        cout << "请输入新网站（留空保持不变）："; getline(cin, new_website);
        cout << "请输入新用户名（留空保持不变）："; getline(cin, new_username);
        cout << "请输入新密码（留空保持不变）："; getline(cin, new_password);
        if (!new_website.empty()) website = new_website;
        if (!new_username.empty()) username = new_username;
        if (!new_password.empty()) {
            // 重新加密
            AES_Processor proc;
            cipher = proc.Encrypt(new_password, key_byte, iv.data());
            StringSource(cipher.data(), cipher.size(), true,
                new HexEncoder(
                    new StringSink(hex_cipher), false)); 
        }
        cout << "\n----- 已更新密码 -----\n";
    }
};

class PasswordManager {
private:
    vector<Entry> entries_;
    string user_key_;
    CryptoPP::byte key_byte_[AES::DEFAULT_KEYLENGTH];

    // 将 user_key_ 填充到 key_byte_
    void LoadKeyBytes() {
        size_t n = min(user_key_.size(), sizeof(key_byte_));
        memcpy(key_byte_, user_key_.data(), n);
        if (n < sizeof(key_byte_)) memset(key_byte_ + n, 0, sizeof(key_byte_) - n);
    }

    // 保存到文件（会覆盖）
    void SaveToFile() {
        sort(entries_.begin(), entries_.end(), [](const Entry& a, const Entry& b) {
            return a.website < b.website; 
        });//lambda表达式,无名函数。
        ofstream ofs(dir, ios::out);
        ofs << "Website,Username,CipherHex,IVHex\n";
        ofs << user_key_ << "\n";
        for (auto& e : entries_) ofs << e.ToCsv();
        ofs.close();
        cout << "\n----- 已保存更改 -----\n";
    }

    // 加载文件，返回 false 表示失败
    bool LoadFromFile() {
        ifstream ifs(dir);
        if (!ifs) {
            // 文件不存在，创建新文件
            ofstream create(dir);
            create << "Website,Username,CipherHex,IVHex\n\n";
            create.close();
            return false;
        }
        string header;
        getline(ifs, header);
        getline(ifs, user_key_);
        LoadKeyBytes();
        string line;
        while (getline(ifs, line)) {
            if (line.empty()) continue;
            entries_.emplace_back(line);
        }
        return true;
    }

    int FindIndex(const string& site) {
        string lower = site;
        transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        for (int i = 0; i < (int)entries_.size(); i++) {
            string w = entries_[i].website;
            transform(w.begin(), w.end(), w.begin(), ::tolower);
            if (w == lower) return i;
        }
        return -1;
    }

    void Pause() {
        cout << "\n按任意键返回主菜单...";
        cin.get();
    }

public:
    PasswordManager() {
        LoadFromFile();
        if (user_key_.empty()) {
            cout << "请设置密码库密钥：";
            getline(cin, user_key_);
            LoadKeyBytes();
            SaveToFile();
        }
    }

    void Run() {
        while (true) {
            cout << "\n=== 密码管理器 ===\n";
            for (auto& e : entries_) e.ShowSummary();
            cout << "1. 添加密码\n"
                 << "2. 查看/编辑/删除某条\n"
                 << "3. 修改库密钥\n"
                 << "4. 保存并退出\n"
                 << "请选择：";
            int choice; cin >> choice; cin.ignore();
            if (choice == 4) {
                SaveToFile();
                break;
            }
            switch (choice) {
                case 1: {
                    string site, user, pwd;
                    cout << "请输入网站："; getline(cin, site);
                    while(site.empty() && isdigit(site[0])){
                        cout << "网站不能为空或以数字开头" << endl;
                        cout << "请输入网站："; getline(cin, site);
                    }
                    cout << "请输入用户名："; getline(cin, user);
                    cout << "请输入密码："; getline(cin, pwd);
                    entries_.emplace_back(site, user, pwd, key_byte_);
                    cout << "添加成功。\n";
                    SaveToFile();
                    Pause();
                } break;
                case 2: {
                    cout << "请输入网站名或空行列索引：";
                    string in; getline(cin, in);
                    int idx = -1;
                    if (!in.empty() && isdigit(in[0])) idx = stoi(in);
                    else idx = FindIndex(in);
                    if (idx < 0 || idx >= (int)entries_.size()) {
                        cout << "未找到。\n"; Pause(); break;
                    }
                    cout << "1. 查看密码  2. 编辑条目  3. 删除条目  4. 返回\n请选择：";
                    int op; cin >> op; cin.ignore();
                    if (op == 1) {
                        cout << "请输入库密钥：";
                        string k; getline(cin, k);
                        if (k == user_key_) {
                            entries_[idx].ShowFull(key_byte_);
                        } else {
                            cout << "密钥错误。\n";
                        }
                    } else if (op == 2) {
                        cout << "请输入新网站："; getline(cin, entries_[idx].website);
                        cout << "请输入新用户名："; getline(cin, entries_[idx].username);
                        cout << "请输入新密码："; string newpwd; getline(cin, newpwd);
                        entries_[idx] = Entry(entries_[idx].website,
                                              entries_[idx].username,
                                              newpwd, key_byte_);
                        cout << "编辑成功。\n";
                        SaveToFile();
                    } else if (op == 3) {
                        entries_.erase(entries_.begin() + idx);
                        cout << "删除成功。\n";
                        SaveToFile();
                    }
                    Pause();
                } break;
                case 3: {
                    cout << "请输入当前库密钥：";
                    string old; getline(cin, old);
                    if (old != user_key_) {
                        cout << "密钥错误。\n"; Pause(); break;
                    }
                    cout << "请输入新库密钥："; getline(cin, user_key_);
                    LoadKeyBytes();
                    cout << "请确认新库密钥："; string c2; getline(cin, c2);
                    if (c2 == user_key_) {
                        cout << "修改成功。\n";
                        SaveToFile();
                    } else {
                        cout << "两次密码输入不一致\n";
                    }
                    Pause();
                } break;
                default:
                    cout << "无效的输入\n";
                    Pause();
            }
        }
    }
};

int main() {
    #ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    #endif
    SetConsoleTitleW(L"密码管理器Password Manager");
    cin.sync_with_stdio(false);
    PasswordManager mgr;
    mgr.Run();
    return 0;
}
