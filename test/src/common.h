#include <stdio.h>
#include <sql.h>
#include <sqlext.h>

static SQLHENV env;
static SQLHDBC conn;

#define CHECK_STMT_RESULT(rc, msg, hstmt)	\
    if (!SQL_SUCCEEDED(rc)) \
    { \
      print_diag(msg, SQL_HANDLE_STMT, hstmt); \
      exit(1); \
    }


static void
print_diag(char *msg, SQLSMALLINT htype, SQLHANDLE handle)
{
  char sqlstate[32];
  char message[1000];
  SQLINTEGER nativeerror;
  SQLSMALLINT textlen;
  SQLRETURN ret;

  if (msg)
    printf("%s\n", msg);

  ret = SQLGetDiagRec(htype, handle, 1, 
		      sqlstate, &nativeerror, message, 256, &textlen);
 
  if (ret != SQL_ERROR)
    printf("%s=%s\n", (CHAR *)sqlstate, (CHAR *)message);
}


static void
test_connect(void)
{
  SQLRETURN ret;
  SQLCHAR str[1024];
  SQLSMALLINT strl;

  SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);

  SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (void *) SQL_OV_ODBC3, 0);

  SQLAllocHandle(SQL_HANDLE_DBC, env, &conn);
  ret = SQLDriverConnect(conn, NULL, "DSN=psqlodbc_test_dsn;", SQL_NTS,
			 str, sizeof(str), &strl,
			 SQL_DRIVER_COMPLETE);
  if (SQL_SUCCEEDED(ret)) {
    printf("connected\n");
  } else {
    print_diag("SQLDriverConnect failed.", SQL_HANDLE_DBC, conn);
    exit(1);
  }
}

static void
test_disconnect(void)
{
  SQLRETURN rc;

  printf("disconnecting\n");
  rc = SQLDisconnect(conn);
  if (!SQL_SUCCEEDED(rc))
  {
    print_diag("SQLDisconnect failed", SQL_HANDLE_DBC, conn);
    exit(1);
  }

  rc = SQLFreeConnect(conn);
  if (!SQL_SUCCEEDED(rc))
  {
    print_diag("SQLFreeConnect failed", SQL_HANDLE_DBC, conn);
    exit(1);
  }
  conn = NULL;

  rc = SQLFreeEnv(env);
  if (!SQL_SUCCEEDED(rc))
  {
    print_diag("SQLFreeEnv failed", SQL_HANDLE_ENV, env);
    exit(1);
  }
  env = NULL;
}

static void
print_result(HSTMT hstmt)
{
  SQLRETURN rc;
  SQLSMALLINT numcols;

  rc = SQLNumResultCols(hstmt, &numcols);
  if (!SQL_SUCCEEDED(rc))
  {
    print_diag("SQLNumResultCols failed", SQL_HANDLE_STMT, hstmt);
    return;
  }

  printf("Result set:\n");
  while(1)
  {
    rc = SQLFetch(hstmt);
    if (rc == SQL_NO_DATA) 
      break;
    if (rc == SQL_SUCCESS)
    {
      char buf[40];
      int i;

      for (i = 1; i <= numcols; i++)
      {
	rc = SQLGetData(hstmt,i, SQL_C_CHAR, buf, sizeof(buf), NULL);
	if (!SQL_SUCCEEDED(rc))
	{
	  print_diag("SQLGetData failed", SQL_HANDLE_STMT, hstmt);
	  return;
	}
	printf("%s%s", (i > 1) ? "\t" : "", buf);
      }
      printf("\n");
    }
    else
    {
      print_diag("SQLFetch failed", SQL_HANDLE_STMT, hstmt);
      exit(1);
    }
  }
}
