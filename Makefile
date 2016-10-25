.PHONY: run clean

CXX=g++
# CXX=g++-6.2
# TODO: use pkg-config to get gtest.
# GTEST=
# GTEST=-lgtest -lpthread

run: promise.t
	./promise.t

promise.t : bbp/promise.t.cpp bbp/promise.h
	${CXX} -I. -fconcepts -std=c++1z $< -o $@ `pkg-config --cflags --libs gtest`

clean:
	$(RM) promise.t
