AM_CFLAGS = --pedantic -Wall -std=c99 -O3
AM_LDFLAGS = -ldb

bin_PROGRAMS = exporter 
exporter_SOURCES = exporter.c 

if ENABLE_TESTS
include Makefile.test.include
endif
