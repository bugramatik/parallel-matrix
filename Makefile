all:
	g++ -o hw2 main.cpp hw2_output.c -lpthread
	./hw2 < input1.txt

clean:
	rm -f hw2