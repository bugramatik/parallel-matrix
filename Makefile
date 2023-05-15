all:
	g++ -o hw2 main.cpp hw2_output.c -lpthread
	./hw2 < input2.txt

test:
	g++ -o hw2 main.cpp hw2_output.c -lpthread
	./hw2 < input1.txt
	$ python3 tester.py
clean:
	rm -f hw2