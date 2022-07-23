compile:
	gcc -Wall -g3 -fsanitize=address -pthread -lm server.c -o server
	gcc -Wall -g3 -fsanitize=address -pthread -lm client.c -o client
	gcc -Wall -g3 -fsanitize=address -pthread -lm client2.c -o client2

clean: 
	rm -f server client client2
	rm -r *dSYM
	rm -r .vscode