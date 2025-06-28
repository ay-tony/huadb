#include "table/table_scan.h"

#include "table/table_page.h"

namespace huadb {

TableScan::TableScan(BufferPool &buffer_pool, std::shared_ptr<Table> table, Rid rid)
    : buffer_pool_(buffer_pool), table_(std::move(table)), rid_(rid) {}

std::shared_ptr<Record> TableScan::GetNextRecord(xid_t xid, IsolationLevel isolation_level, cid_t cid,
                                                 const std::unordered_set<xid_t> &active_xids) {
  // 根据事务隔离级别及活跃事务集合，判断记录是否可见
  // LAB 3 BEGIN

  // 每次调用读取一条记录
  // 读取时更新 rid_ 变量，避免重复读取
  // 扫描结束时，返回空指针
  // 注意处理扫描空表的情况（rid_.page_id_ 为 NULL_PAGE_ID）
  // LAB 1 BEGIN

  auto &[page_id, slot_id] = rid_;

  // 移动 rid_ 指向下一条记录
  auto next_record = [&] -> void {
    TablePage table_page{buffer_pool_.GetPage(table_->GetDbOid(), table_->GetOid(), page_id)};
    if (slot_id + 1 < table_page.GetRecordCount()) {
      slot_id++;
    } else {
      page_id = table_page.GetNextPageId();
      slot_id = 0;
    }
  };

  // 找到当前及之后的第一条未被删除的记录
  while (page_id != NULL_PAGE_ID && TablePage{buffer_pool_.GetPage(table_->GetDbOid(), table_->GetOid(), page_id)}
                                        .GetRecord(rid_, table_->GetColumnList())
                                        ->IsDeleted()) {
    next_record();
  }

  // 遍历结束
  if (page_id == NULL_PAGE_ID) {
    return nullptr;
  }

  // 读取当前 rid_ 记录
  TablePage table_page{buffer_pool_.GetPage(table_->GetDbOid(), table_->GetOid(), page_id)};
  auto record = table_page.GetRecord(rid_, table_->GetColumnList());

  // 移动指针
  next_record();

  return record;
}

}  // namespace huadb
