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



int main() {
  // freopen("test_data.txt", "r", stdin);
  // freopen("output.txt", "w", stdout);
  int n;
  std::cin >> n;
  BPT<int, int> bpt("database");
  
  for (int i = 0; i < n; ++i) {
    std::string order;
    std::cin >> order;
    if (order == "insert") {
      std::string key;
      int value;
      std::cin >> key >> value;
      bpt.insert(Hash(key), value);
    } else if (order == "find") {
      std::string key;
      std::cin >> key;
      
      auto result = bpt.find(Hash(key));
      if (result.size() == 0) {
        std::cout << "null\n";
      } else {
        for (auto &item : result) {
          std::cout << item << ' ';
        }
        std::cout << '\n';
      }
    } else if (order == "delete") {
      std::string key;
      int value;
      std::cin >> key >> value;
      bpt.remove(Hash(key), value);
    }
  }
  
}