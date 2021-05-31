all: 
	gcc source/can.c receiver.c -pthread -o execR && gcc source/can.c transmiter.c -pthread -o execT
leak:
	gcc -g source/can.c receiver.c -pthread -o execR && valgrind --leak-check=full -s ./execR && gcc -g source/can.c transmiter.c -pthread -o execT && valgrind --leak-check=full -s ./exec
run:
	./execT && ./execR