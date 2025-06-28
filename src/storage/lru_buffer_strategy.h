#pragma once

#include <list>
#include <unordered_map>

#include "storage/buffer_strategy.h"

namespace huadb {

class LRUBufferStrategy : public BufferStrategy {
 public:
  void Access(size_t frame_no) override;
  size_t Evict() override;

 private:
  std::list<size_t> frame_order_;                                          // 缓存页面链表，按照最近访问顺序排序
  std::unordered_map<size_t, std::list<size_t>::iterator> frame_to_list_;  // 页面编号到链表的映射
};

}  // namespace huadb
