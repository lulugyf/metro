

.SUFFIXES: .cpp .cc .c .o

ORA_HOME=/d/dev/ace/instantclient_12_2
CPPFLAGS=
INCLUDE=-I$(ORA_HOME)/sdk/include -I./include

all: ex10_oci8

hello.dll: hello.c
	gcc -c -O3 -DBUILD_DLL hello.c -o hello.o
	gcc -shared -Wl,--output-def,hello.def -Wl,--out-implib,libhello.a -Wl,-Bsymbolic -o hello.dll -Wl,hello.o

ex10_oci8: ex10_oci8.o
	#g++ -L$(ORA_HOME)/ -loci -o $@ $?
	#g++ -L$(ORA_HOME)/sdk/lib/msvc -loci -o $@ $?
	g++ $(ORA_HOME)/oci.dll -o $@ $?

.cpp.o:
	g++ $(CPPFLAGS) $(INCLUDE)   -c $*.cpp

#ex10_oci8.o: ex10_oci8.cpp
#	g++ -c  -o $@ $?
