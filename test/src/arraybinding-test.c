#include <stdio.h>
#include <stdlib.h>

#include "common.h"

#define ARRAY_SIZE 10000

int main(int argc, char **argv)
{
  SQLRETURN rc;
  HSTMT hstmt = SQL_NULL_HSTMT;
  char *sql;

  int i;

  SQLUINTEGER int_array[ARRAY_SIZE];
  SQLCHAR str_array[ARRAY_SIZE][30];
  SQLLEN int_ind_array[ARRAY_SIZE];
  SQLLEN str_ind_array[ARRAY_SIZE];
  SQLUSMALLINT status_array[ARRAY_SIZE];
  SQLULEN nprocessed;

  test_connect();

  rc = SQLAllocStmt(conn, &hstmt);
  if (!SQL_SUCCEEDED(rc))
  {
    print_diag("failed to allocate stmt handle", SQL_HANDLE_DBC, conn);
    exit(1);
  }

  sql = "CREATE TEMPORARY TABLE tmptable (i int4, t text)";
  rc = SQLExecDirect(hstmt, (SQLCHAR *) sql, SQL_NTS);
  CHECK_STMT_RESULT(rc, "SQLExecDirect failed while creating temp table", hstmt);

  rc = SQLFreeStmt(hstmt, SQL_CLOSE);
  CHECK_STMT_RESULT(rc, "SQLFreeStmt failed", hstmt);

  /* Test column-wise binding */
  for (i = 0; i < ARRAY_SIZE; i++)
  {
    int_array[i] = 0;
    int_ind_array[i] = 0;
    sprintf(str_array[i], "columnwise %d", i);
    str_ind_array[i] = SQL_NTS;
  }

  SQLSetStmtAttr(hstmt, SQL_ATTR_PARAM_BIND_TYPE, SQL_PARAM_BIND_BY_COLUMN, 0);
  SQLSetStmtAttr(hstmt, SQL_ATTR_PARAM_STATUS_PTR, status_array, 0);
  SQLSetStmtAttr(hstmt, SQL_ATTR_PARAMS_PROCESSED_PTR, &nprocessed, 0);
  SQLSetStmtAttr(hstmt, SQL_ATTR_PARAMSET_SIZE, (SQLPOINTER) ARRAY_SIZE, 0);

  // Bind the parameters in column-wise fashion.
  SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_ULONG, SQL_INTEGER, 5, 0,
		   int_array, 0, int_ind_array);
  SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, 29, 0,
                   str_array, 0, str_ind_array);

  sql = "INSERT INTO tmptable VALUES (?, ?)";
  rc = SQLExecDirect(hstmt, (SQLCHAR *) sql, SQL_NTS);
  CHECK_STMT_RESULT(rc, "SQLExecDirect failed", hstmt);

  /* Fetch results */
  printf("Parameter	Status\n");
  for (i = 0; i < nprocessed; i++)
  {
    switch (status_array[i])
    {
      case SQL_PARAM_SUCCESS:
      case SQL_PARAM_SUCCESS_WITH_INFO:
         break;

      case SQL_PARAM_ERROR:
         printf("%d\tError\n", i);
         break;

      case SQL_PARAM_UNUSED:
         printf("%d\tUnused\n", i);
         break;

      case SQL_PARAM_DIAG_UNAVAILABLE:
         printf("%d\tDiag unavailable\n", i);
         break;
    }
  }

  rc = SQLFreeStmt(hstmt, SQL_CLOSE);
  CHECK_STMT_RESULT(rc, "SQLFreeStmt failed", hstmt);

  /*
   * Free and allocate a new handle for the next SELECT statement, as we don't
   * want to array bind that one. The parameters set with SQLSetStmtAttr
   * survive SQLFreeStmt.
   */
  rc = SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
  CHECK_STMT_RESULT(rc, "SQLFreeHandle failed", hstmt);

  rc = SQLAllocStmt(conn, &hstmt);
  if (!SQL_SUCCEEDED(rc))
  {
    print_diag("failed to allocate stmt handle", SQL_HANDLE_DBC, conn);
    exit(1);
  }

  /* Check that all the rows were inserted */
  sql = "SELECT COUNT(*) FROM tmptable";
  rc = SQLExecDirect(hstmt, (SQLCHAR *) sql, SQL_NTS);
  CHECK_STMT_RESULT(rc, "SQLExecDirect failed", hstmt);
  print_result(hstmt);

  rc = SQLFreeStmt(hstmt, SQL_CLOSE);
  CHECK_STMT_RESULT(rc, "SQLFreeStmt failed", hstmt);

  /* Clean up */
  test_disconnect();

  return 0;
}
