client: game_clnt.h packet.h game_ui.c input.c clnt.c marble.c client.c login_screen.c
	gcc -Wall -o client client.c game_ui.c input.c clnt.c marble.c login_screen.c -lncursesw -lpthread

#client: client.c game.h game_ui.c input.c clnt.c marble.c login_screen.c 
	#gcc client.c -o client game.h game_ui.c input.c clnt.c marble.c login_screen.c -lncursesw -lpthread

server: server.c game.c game.h gamePacket.h gamePacket.c
	gcc -Wall game.h gamePacket.h server.c game.c gamePacket.c -o server -lpthread -L/usr/lib/aarch64-linux-gnu -lmysqlclient -lpthread -ldl -lssl -lcrypto -lresolv -lm -lrt -I/usr/include/mysql  

mysql_init: mysql.c
	gcc -o mysql_init mysql.c -L/usr/lib/aarch64-linux-gnu -lmysqlclient -lpthread -ldl -lssl -lcrypto -lresolv -lm -lrt -I/usr/include/mysql 

