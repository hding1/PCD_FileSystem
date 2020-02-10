test: test.c disk.c db.c sb.c
	gcc test.c disk.c db.c sb.c -o test
clean:	
	rm test
