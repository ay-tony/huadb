#include "storage/lru_buffer_strategy.h"

namespace huadb {

void LRUBufferStrategy::Access(size_t frame_no) {
  // 缓存页面访问
  // LAB 1 BEGIN

  if (auto it{frame_to_list_.find(frame_no)}; it != frame_to_list_.end()) {
    frame_order_.erase(it->second);
  }
  frame_order_.push_back(frame_no);
  frame_to_list_[frame_no] = std::prev(frame_order_.end());
};

size_t LRUBufferStrategy::Evict() {
  // 缓存页面淘汰，返回淘汰的页面在 buffer pool 中的下标
  // LAB 1 BEGIN

  size_t frame_no{frame_order_.front()};
  frame_order_.pop_front();
  frame_to_list_.erase(frame_to_list_.find(frame_no));

  return frame_no;
}

}  // namespace huadb
