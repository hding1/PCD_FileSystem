block_test: block_test.c disk.c db.c sb.c
	gcc block_test.c disk.c db.c sb.c -o block_test
clean:	
	rm block_test
