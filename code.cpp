#include <iostream>

#include "src/BPT.hpp"
#include "src/vector.hpp"

template class BPT<int, int>;

constexpr int PR = 31;
constexpr int MOD = 1e9 + 7;

int Hash(const std::string &s) {
  long long hash = 0;
  for (char c : s) {
    hash = (hash * PR + c) % MOD;
  }
  return hash;
}

// 调试模式标志
const bool DEBUG_MODE = false;

// 调试辅助函数
void debugBPlusTree(BPT<int, int>& bpt) {
  std::cout << "\n===== 调试信息 =====" << std::endl;
  std::cout << "打印当前B+树结构:" << std::endl;
  bpt.printTree();
  std::cout << "===================" << std::endl;
}

int main() {
  freopen("test_data.txt", "r", stdin);
  freopen("output.txt", "w", stdout);
  int n;
  std::cin >> n;
  BPT<int, int> bpt("database");
  
  // 正常模式下执行输入命令
  for (int i = 0; i < n; ++i) {
    std::string order;
    std::cin >> order;
    if (order == "insert") {
      std::string key;
      int value;
      std::cin >> key >> value;
      bpt.insert(Hash(key), value);
      
      // 插入后可以打印树结构进行调试
      if (DEBUG_MODE) {
        std::cout << "插入 " << key << ":" << value << " 后的树结构：" << std::endl;
        //debugBPlusTree(bpt);
        bpt.debugSearchPath(Hash(key));
      }
    } else if (order == "find") {
      std::string key;
      std::cin >> key;
      auto result = bpt.find(Hash(key));
      Qsort(result, 0, result.size() - 1);
      if (result.size() == 0) {
        std::cout << "null" << std::endl;
      } else {
        for (auto &item : result) {
          std::cout << item << " ";
        }
        std::cout << std::endl;
      }
    } else if (order == "delete") {
      std::string key;
      int value;
      std::cin >> key >>value;
      bpt.remove(Hash(key), value);
      
      // 删除后可以打印树结构进行调试
      if (DEBUG_MODE) {
        std::cout << "删除 " << key << ":" << value << " 后的树结构：" << std::endl;
        debugBPlusTree(bpt);
      }
    }
  }
}