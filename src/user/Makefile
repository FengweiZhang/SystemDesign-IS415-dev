all:
	gcc server.c database.c log.c databaseExtention.c -l sqlite3 -o server.exe
	gcc client.c log.c databaseExtention.c -o client.exe

test:
	gcc ./user_netlink.c ./databaseExtension.c -o a.out

clean:
	rm a.out