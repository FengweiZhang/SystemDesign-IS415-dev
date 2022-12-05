all:
	gcc server.c database.c log.c databaseExtension.c user_netlink.c -l sqlite3 -o server.exe
	gcc client.c log.c -o client.exe

clean:
	rm ./client.exe ./server.exe