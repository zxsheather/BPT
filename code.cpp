#include <iostream>

#include "src/BPT.hpp"
#include "src/vector.hpp"


constexpr long long PR = 998244353;
constexpr long long MOD = 99234523452349217;

long long Hash(const std::string &s) {
  __int128_t hash = 0;
  for (char c : s) {
    hash = (hash * PR + c + 1) % MOD;
  }
  return hash;
}
int main() {
  int n;
  std::cin >> n;
  BPT<long long, int> bpt("database");
  
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