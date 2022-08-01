
test_3:      main.o   thread.o   utiles.o
	gcc  -o $@  -lev  $^

%.o: %.c my.h
	gcc -c $<
