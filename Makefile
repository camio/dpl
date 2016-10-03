.PHONY: run clean

run: promise.t
	./promise.t

promise.t : bbp/promise.t.cpp bbp/promise.h
	g++-6.2 -I. -fconcepts -std=c++1z $< -o $@ -lgtest -lpthread

clean:
	$(RM) promise.t
