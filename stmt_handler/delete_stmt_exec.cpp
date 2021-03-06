/*
 * Copyright [2012-2015] DaSE@ECNU
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * /CLAIMS/stmt_handler/delete_stmt_exec.cpp
 *
 *  Created on: Nov 19, 2015
 *      Author: yuyang
 *		   Email: youngfish93@hotmail.com
 *
 * Description:
 *
 */

#include <string>
#include <vector>
#include <iostream>
#include "../Environment.h"
#include "../stmt_handler/delete_stmt_exec.h"

#include "../common/Block/BlockStream.h"
#include "../common/Block/ResultSet.h"
#include "../catalog/table.h"
#include "../Daemon/Daemon.h"
#include "../loader/data_injector.h"
#include "../sql_parser/ast_node/ast_select_stmt.h"
#include "../stmt_handler/select_exec.h"
#include "../common/error_define.h"
using claims::loader::DataInjector;
using std::endl;
using std::string;
using std::vector;
using std::cout;
namespace claims {
namespace stmt_handler {

DeleteStmtExec::DeleteStmtExec(AstNode* stmt) : StmtExec(stmt) {
  assert(stmt_);
  delete_stmt_ast_ = dynamic_cast<AstDeleteStmt*>(stmt_);
}

DeleteStmtExec::~DeleteStmtExec() {}

RetCode DeleteStmtExec::Execute(ExecutedResult* exec_result) {
  RetCode ret = rSuccess;
  string tablename =
      dynamic_cast<AstTable*>(
          dynamic_cast<AstFromList*>(delete_stmt_ast_->from_list_)->args_)
          ->table_name_;
  TableDescriptor* new_table =
      Environment::getInstance()->getCatalog()->getTable(tablename);

  SemanticContext sem_cnxt;
  ret = delete_stmt_ast_->SemanticAnalisys(&sem_cnxt);
  if (rSuccess != ret) {
    exec_result->error_info_ =
        "Semantic analysis error.\n" + sem_cnxt.error_msg_;
    exec_result->status_ = false;
    LOG(ERROR) << "semantic analysis error result= : " << ret;
    cout << "semantic analysis error result= : " << ret << endl;
    return ret;
  }

  /**
   * step1 : create new sql query including row_id, for example:
   * DELETE FROM tbA WHERE colA = 10;
   * =>
   * SELECT row_id FROM tbA WHERE colA = 10;
   */

  AstNode* appended_query_col =
      new AstColumn(AST_COLUMN, tablename, "row_id", NULL);
  AstNode* appended_query_sel_expr =
      new AstSelectExpr(AST_SELECT_EXPR, "", appended_query_col);
  AstNode* appended_query_sel_list =
      new AstSelectList(AST_SELECT_LIST, false, appended_query_sel_expr, NULL);
  AstNode* appended_query_sel_stmt = new AstSelectStmt(
      AST_SELECT_STMT, 0, appended_query_sel_list, delete_stmt_ast_->from_list_,
      delete_stmt_ast_->where_list_, NULL, NULL, NULL, NULL, NULL);

  // ExecutedResult* appended_result = new ExecutedResult();
  SelectExec* appended_query_exec = new SelectExec(appended_query_sel_stmt);

  appended_query_exec->Execute(exec_result);
  ostringstream ostr;
  ostr << exec_result->result_->getNumberOftuples() << " tuples deleted.";
  exec_result->info_ = ostr.str();
  // set the flag weather it contains the deleted the tuples or not in the base
  // table
  TableDescriptor* table =
      Environment::getInstance()->getCatalog()->getTable(tablename);
  table->SetDeletedTuples(true);

  string del_table_name = tablename + "_DEL";
  /**
   * step2 : Insert delete data into _DEL table.
   */
  InsertDeletedDataIntoTableDEL(del_table_name, exec_result);

  return ret;
}

void DeleteStmtExec::InsertDeletedDataIntoTableDEL(
    string tablename, ExecutedResult* exec_result) {
  DynamicBlockBuffer::Iterator it = exec_result->result_->createIterator();
  BlockStreamBase* block;
  BlockStreamBase::BlockStreamTraverseIterator* tuple_it;

  cout << "insert del table name : " << tablename.c_str();

  TableDescriptor* tabledel =
      Environment::getInstance()->getCatalog()->getTable(tablename);
  if (NULL == tabledel) {
    LOG(ERROR) << "The table DEL " + tablename +
                      " is not existed during delete data." << std::endl;
    return;
  }

  /**
   *    Prepare the data which will be insert into TABLE_DEL.
   */
  ostringstream ostr;
  while (block = it.nextBlock()) {
    tuple_it = block->createIterator();
    void* tuple;
    while (tuple = tuple_it->nextTuple()) {
      for (unsigned i = 0; i < exec_result->result_->column_header_list_.size();
           i++) {
        // check whether the row has been deleted or not.
        if (true) {
          ostr << exec_result->result_->schema_->getcolumn(i)
                      .operate->toString(exec_result->result_->schema_
                                             ->getColumnAddess(i, tuple))
                      .c_str();
          ostr << "|";
        } else {
          continue;
        }
      }
      ostr << "\n";
    }
    delete tuple_it;
  }

  DataInjector* injector = new DataInjector(tabledel);
  injector->InsertFromString(ostr.str(), exec_result);

  //  HdfsLoader* Hl = new HdfsLoader(tabledel);
  //  string tmp = ostr.str();
  //  Hl->append(ostr.str());
  //  cout << tmp << endl;
  Environment::getInstance()->getCatalog()->saveCatalog();
}

} /* namespace stmt_handler */
} /* namespace claims */
