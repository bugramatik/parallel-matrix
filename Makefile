all:
	g++ -o hw2 main.cpp hw2_output.c -lpthread

test:
	g++ -o hw2 main.cpp hw2_output.c -lpthread
	$ python3 tester.py

clean:
	rm -f hw2