all: logger.c main.c projtemp.c
	gcc -o main.out main.c usrled.c -lrt -lpthread
	gcc -o temp.out projtemp.c usrled.c i2c.c -lrt -lpthread
	gcc -o light.out projlight.c i2c.c usrled.c -lrt -lpthread -lm
	gcc -o sock.out socket.c -lrt -lpthread
	gcc -o log.out logger.c -lpthread -lrt
	gcc -o send_socket.out send_socket.c

clean:
	find . -type f | xargs touch
	rm *.out
