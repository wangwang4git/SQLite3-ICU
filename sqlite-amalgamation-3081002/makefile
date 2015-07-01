mysqlite3.out : shell.c sqlite3.c
	gcc -o mysqlite3.out -DSQLITE_ENABLE_FTS4 shell.c sqlite3.c -lpthread -ldl -licuuc -licui18n

clean :
	rm -f mysqlite3.out