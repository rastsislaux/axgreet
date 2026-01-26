.PHONY: build
build:
	cc -o main main.c json.c hexutil.c greetd.c

.PHONY: run
run:
	./main

.PHONY: clean
clean:
	rm -f main
