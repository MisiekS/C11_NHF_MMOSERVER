valami:
	g++ src/database.cpp -o out/database.o -c -g
	g++ src/command.cpp -o out/command.o -c -g
	g++ src/monster.cpp -o out/monster.o -c -g
	g++ src/informationserver.cpp -o out/informationserver.o -c -g
	g++ src/actionserver.cpp -o out/actionserver.o -c -g
	g++ src/server.cpp -o out/server.o -c -g
	g++ src/main.cpp out/database.o out/monster.o out/command.o out/informationserver.o out/actionserver.o out/server.o -o server.out -lpthread -lboost_system -lsqlite3 -g -Werror
	
db:
	g++ src/database.cpp -o out/database.o -c -g
	g++ src/dbtest.cpp out/database.o -o dbtest.o -lpthread -lboost_system -lsqlite3 -g
