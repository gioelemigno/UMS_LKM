all:
	gcc -c ./src/ums.c 					-o ./build/ums.o  					-lpthread
	gcc -c ./src/ums_context.c 			-o ./build/ums_context.o 			-lpthread
	gcc -c ./src/ums_scheduler.c		-o ./build/ums_scheduler.o  		-lpthread
	gcc -c ./src/ums_completion_list.c 	-o ./build/ums_completion_list.o  	-lpthread
	ar rcs ../../UMS_Test/lib/libums.a ./build/ums.o ./build/ums_context.o ./build/ums_scheduler.o ./build/ums_completion_list.o
clean:
	rm -rfv ./build/*.o
 
