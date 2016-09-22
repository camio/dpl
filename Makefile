.PHONY: run
run: promise.t
	./promise.t

promise.t : bbp/promise.t.cpp bbp/promise.h
	g++ -I. -fconcepts $< -o $@ $(shell pkg-config --libs gtest)
