#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

int main(int argc, char **argv)
{
  SQLRETURN rc;
  HSTMT hstmt = SQL_NULL_HSTMT;
  char *param1, *param2;
  SQLLEN cbParam1, cbParam2;
  long longparam;
  SQLLEN param1bytes, param2bytes;
  SQL_INTERVAL_STRUCT intervalparam;
  SQLSMALLINT colcount;
  PTR paramid;
  char buf[40];
  SQLLEN lenOrInd;

  test_connect();

  rc = SQLAllocStmt(conn, &hstmt);
  if (!SQL_SUCCEEDED(rc))
  {
    print_diag("failed to allocate stmt handle", SQL_HANDLE_DBC, conn);
    exit(1);
  }

  /****
   * Bind with data-at-execution params. (VARBINARY)
   */

  /* Prepare a statement */
  rc = SQLPrepare(hstmt, (SQLCHAR *) "SELECT id FROM byteatab WHERE t = ? OR t = ?", SQL_NTS);
  CHECK_STMT_RESULT(rc, "SQLPrepare failed", hstmt);

  /* prepare the parameter values */
  param1 = "bar";
  param1bytes = strlen(param1);
  cbParam1 = SQL_DATA_AT_EXEC;
  param2 = "foobar";
  param2bytes = strlen(param2);
  cbParam2 = SQL_DATA_AT_EXEC;

  /* bind them. */
  rc = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT,
			SQL_C_BINARY,	/* value type */
			SQL_VARBINARY,	/* param type */
			param1bytes,	/* column size */
			0,		/* dec digits */
			(VOID *) 1,	/* param value ptr. For a data-at-exec
					 * param, this is a "parameter id" */
			0,		/* buffer len */
			&cbParam1	/* StrLen_or_IndPtr */);
  CHECK_STMT_RESULT(rc, "SQLBindParameter failed", hstmt);

  rc = SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT,
			SQL_C_BINARY,	/* value type */
			SQL_VARBINARY,	/* param type */
			param2bytes,	/* column size */
			0,		/* dec digits */
			(VOID *) 2,	/* param value ptr. For a data-at-exec
					 * param, this is a "parameter id" */
			0,		/* buffer len */
			&cbParam2	/* StrLen_or_IndPtr */);
  CHECK_STMT_RESULT(rc, "SQLBindParameter failed", hstmt);

  /* Execute */
  rc = SQLExecute(hstmt);
  if (rc != SQL_NEED_DATA)
    CHECK_STMT_RESULT(rc, "SQLExecute failed", hstmt);

  /* set parameters */
  paramid = 0;
  while ((rc = SQLParamData(hstmt, &paramid)) == SQL_NEED_DATA)
  {
    if (paramid == (VOID *) 1)
    {
      rc = SQLPutData(hstmt, param1, param1bytes);
      CHECK_STMT_RESULT(rc, "SQLPutData failed", hstmt);
    }
    else if (paramid == (VOID *) 2)
    {
      rc = SQLPutData(hstmt, param2, param2bytes);
      CHECK_STMT_RESULT(rc, "SQLPutData failed", hstmt);
    }
    else
    {
      printf("unexpected parameter id returned by SQLParamData: %d\n", paramid);
      exit(1);
    }
  }
  CHECK_STMT_RESULT(rc, "SQLParamData failed", hstmt);

  /* Fetch result */
  print_result(hstmt);

  rc = SQLFreeStmt(hstmt, SQL_CLOSE);
  CHECK_STMT_RESULT(rc, "SQLFreeStmt failed", hstmt);

  /* Clean up */
  test_disconnect();

  return 0;
}
