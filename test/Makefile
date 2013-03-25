TESTS = connect stmthandles select getresult prepare params notice \
	arraybinding insertreturning dataatexecution boolsaschar cvtnulldate \
	alter

REGRESS = sampletables $(TESTS) 

# For each test file, compile the .c file, and create a .sql file that
# when executed from psql, just runs the binary. 
src/%-test sql/%.sql: src/%-test.c
	$(CC) $(CFLAGS) src/$*-test.c -o src/$*-test -lodbc
	echo "\! ./src/$*-test" > sql/$*.sql

TESTBINS = $(patsubst %,src/%-test, $(TESTS))
TESTSQLS = $(patsubst %,sql/%.sql, $(TESTS))

EXTRA_CLEAN = $(TESTBINS) $(TESTSQLS)

REGRESS_OPTS = --launcher=./launcher

installcheck: all

all: $(TESTBINS) $(TESTSQLS)

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)

