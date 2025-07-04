#include "table/table.h"

#include "table/table_page.h"

namespace huadb {

Table::Table(BufferPool &buffer_pool, LogManager &log_manager, oid_t oid, oid_t db_oid, ColumnList column_list,
             bool new_table, bool is_empty)
    : buffer_pool_(buffer_pool),
      log_manager_(log_manager),
      oid_(oid),
      db_oid_(db_oid),
      column_list_(std::move(column_list)) {
  if (new_table || is_empty) {
    first_page_id_ = NULL_PAGE_ID;
  } else {
    first_page_id_ = 0;
  }
}

Rid Table::InsertRecord(std::shared_ptr<Record> record, xid_t xid, cid_t cid, bool write_log) {
  if (record->GetSize() > MAX_RECORD_SIZE) {
    throw DbException("Record size too large: " + std::to_string(record->GetSize()));
  }

  // 当 write_log 参数为 true 时开启写日志功能
  // 在插入记录时增加写 InsertLog 过程
  // 在创建新的页面时增加写 NewPageLog 过程
  // 设置页面的 page lsn
  // LAB 2 BEGIN

  // 使用 buffer_pool_ 获取页面
  // 使用 TablePage 类操作记录页面
  // 遍历表的页面，判断页面是否有足够的空间插入记录，如果没有则通过 buffer_pool_ 创建新页面
  // 如果 first_page_id_ 为 NULL_PAGE_ID，说明表还没有页面，需要创建新页面
  // 创建新页面时需设置前一个页面的 next_page_id，并将新页面初始化
  // 找到空间足够的页面后，通过 TablePage 插入记录
  // 返回插入记录的 rid
  // LAB 1 BEGIN

  // 如果表中没有页面则新插入一张页面
  if (first_page_id_ == NULL_PAGE_ID) {
    first_page_id_ = 0;
    TablePage page{buffer_pool_.NewPage(db_oid_, oid_, first_page_id_)};
    page.Init();
  }

  pageid_t cur_page_id{first_page_id_};
  TablePage cur_table_page{buffer_pool_.GetPage(db_oid_, oid_, cur_page_id)};

  // 遍历表中页面，找到第一个有足够空间的页面
  // 遍历过程中如果到末尾就新建页面
  while (cur_table_page.GetFreeSpaceSize() < record->GetSize()) {
    if (cur_table_page.GetNextPageId() == NULL_PAGE_ID) {
      cur_table_page.SetNextPageId(cur_page_id + 1);
      TablePage new_table_page{buffer_pool_.NewPage(db_oid_, oid_, cur_page_id + 1)};
      new_table_page.Init();
    }
    cur_page_id = cur_table_page.GetNextPageId();
    cur_table_page = TablePage{buffer_pool_.GetPage(db_oid_, oid_, cur_page_id)};
  }

  // 在目标页面插入记录
  auto slot_id{cur_table_page.InsertRecord(record, xid, cid)};

  return {cur_page_id, slot_id};
}

void Table::DeleteRecord(const Rid &rid, xid_t xid, bool write_log) {
  // 增加写 DeleteLog 过程
  // 设置页面的 page lsn
  // LAB 2 BEGIN

  // 使用 TablePage 操作页面
  // LAB 1 BEGIN

  auto [page_id, slot_id] = rid;
  TablePage page{buffer_pool_.GetPage(db_oid_, oid_, page_id)};
  page.DeleteRecord(slot_id, xid);
}

Rid Table::UpdateRecord(const Rid &rid, xid_t xid, cid_t cid, std::shared_ptr<Record> record, bool write_log) {
  DeleteRecord(rid, xid, write_log);
  return InsertRecord(record, xid, cid, write_log);
}

void Table::UpdateRecordInPlace(const Record &record) {
  auto rid = record.GetRid();
  auto table_page = std::make_unique<TablePage>(buffer_pool_.GetPage(db_oid_, oid_, rid.page_id_));
  table_page->UpdateRecordInPlace(record, rid.slot_id_);
}

pageid_t Table::GetFirstPageId() const { return first_page_id_; }

oid_t Table::GetOid() const { return oid_; }

oid_t Table::GetDbOid() const { return db_oid_; }

const ColumnList &Table::GetColumnList() const { return column_list_; }

}  // namespace huadb
