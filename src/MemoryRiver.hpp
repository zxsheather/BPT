
#ifndef BPT_MEMORYRIVER_HPP
#define BPT_MEMORYRIVER_HPP

#include <fstream>

using std::fstream;
using std::ifstream;
using std::ofstream;
using std::string;

template <class T, int info_len = 2>
class MemoryRiver {
 private:
  /* your code here */
  fstream file;
  string file_name;
  int sizeofT = sizeof(T);

 public:
  MemoryRiver() = default;

  MemoryRiver(const string &file_name) : file_name(file_name) {}

  void initialise(string FN = "") {
    if (FN != "") file_name = FN;
    file.open(file_name, std::ios::out);
    int tmp = 0;
    for (int i = 0; i < info_len; ++i) {
      file.write(reinterpret_cast<char *>(&tmp), sizeof(int));
    }

    file.close();
  }

  // 读出第n个int的值赋给tmp，1_base
  void get_info(int &tmp, int n) {
    if (n > info_len) return;
    file.open(file_name, std::ios::in);
    file.seekg((n - 1) * sizeof(int), std::ios::beg);
    file.read(reinterpret_cast<char *>(&tmp), sizeof(int));
    file.close();
    /* your code here */
  }

  // 将tmp写入第n个int的位置，1_base
  void write_info(int tmp, int n) {
    if (n > info_len) return;
    file.open(file_name, std::ios::in | std::ios::out);
    file.seekp((n - 1) * sizeof(int), std::ios::beg);
    file.write(reinterpret_cast<char *>(&tmp), sizeof(int));
    file.close();
    /* your code here */
  }

  // 在文件合适位置写入类对象t，并返回写入的位置索引index
  // 位置索引意味着当输入正确的位置索引index，在以下三个函数中都能顺利的找到目标对象进行操作
  // 位置索引index可以取为对象写入的起始位置
  int write(T &t) {
    file.open(file_name, std::ios::in | std::ios::out);
    file.seekp(0, std::ios::end);
    int index = file.tellp();
    file.write(reinterpret_cast<char *>(&t), sizeof(T));
    file.close();
    return index;
    /* your code here */
  }

  // 用t的值更新位置索引index对应的对象，保证调用的index都是由write函数产生
  void update(T &t, const int index) {
    file.open(file_name, std::ios::in | std::ios::out);
    file.seekp(index, std::ios::beg);
    file.write(reinterpret_cast<char *>(&t), sizeof(T));
    file.close();
    /* your code here */
  }

  // 读出位置索引index对应的T对象的值并赋值给t，保证调用的index都是由write函数产生
  void read(T &t, const int index) {
    file.open(file_name, std::ios::in | std::ios::out);
    file.seekg(index, std::ios::beg);
    file.read(reinterpret_cast<char *>(&t), sizeof(T));
    file.close();
    /* your code here */
  }

  // 删除位置索引index对应的对象(不涉及空间回收时，可忽略此函数)，保证调用的index都是由write函数产生
  void Delete(int index) {
    file.open(file_name, std::ios::in | std::ios::out);
    file.seekg(0, std::ios::end);
    std::streamsize size = file.tellg();
    file.seekg(index + sizeof(T), std::ios::beg);
    char *buffer = new char[size - index - sizeof(T)];
    file.read(buffer, size - index - sizeof(T));
    file.seekp(index, std::ios::beg);
    file.write(buffer, size - index - sizeof(T));
    file.close();
    delete[] buffer;
    /* your code here */
  }

  bool exist() const {
    std::ifstream file(file_name, std::ios::binary);
    if (file) {
      return true;
    } else {
      return false;
    }
  }
};

#endif  // BPT_MEMORYRIVER_HPP
