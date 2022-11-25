all:
	gcc ./user_netlink.c ./user_netlink.h -o a.out
	gcc server.c database.c -l sqlite3 -o server.exe
	gcc client.c -o client.exe

clean:
	rm a.out