
.SUFFIXES: .cpp .cc .c .o

all: main sendcmd

ACE_ROOT=/d/dev/ace/ACE_wrappers

include $(ACE_ROOT)/include/makeinclude/wrapper_macros.GNU
include $(ACE_ROOT)/include/makeinclude/macros.GNU
# include $(ACE_ROOT)/include/makeinclude/rules.common.GNU
# include $(ACE_ROOT)/include/makeinclude/rules.nonested.GNU

# include $(ACE_ROOT)/include/makeinclude/rules.local.GNU


#g++  -g   -I../..  -Wl,--enable-auto-import -L/d/dev/ace/ACE_wrappers/lib -L.  -L../../lib     -o logging_client.exe .obj/Logging_Client.o -lACE -lws2_32 -lmswsock -liphlpapi -lwsock32 -lnetapi32

CPPFLAGS=-Wnon-virtual-dtor -g -mthreads -fno-strict-aliasing -Wall -Wextra -Wpointer-arith -pipe \
  -DACE_HAS_CUSTOM_EXPORT_MACROS=0 -Wno-unknown-pragmas  -I$(ACE_ROOT) -I../include -DACE_NO_INLINE

LDFLAGS=-Wnon-virtual-dtor -O3 -g -mthreads -fno-strict-aliasing -Wall -Wextra -Wpointer-arith -pipe \
  -Wno-unknown-pragmas -Wl,--enable-auto-import -L$(ACE_ROOT)/lib -Wl,-rpath,/d/dev/ace/local/lib -L../lib

LIBS=-lACE -lmswsock -liphlpapi -lwsock32 -lnetapi32 -ljsonc -lhiredis -lws2_32 -lyaml-cpp
LIBS_JSON=-ljsonc


# SRC = Logging_Client.cpp
# OBJS = $(patsubst %.cpp,.obj/%.o,$(SRC))

# Logging_Client: $(OBJS)
# 	g++ $(LDFLAGS) -o $@ $(OBJS) $(LIBS) 


# https://stackoverflow.com/questions/4102469/makefile-to-put-object-files-from-source-files-different-directories-into-a-sing
SRC1 = pkt.cpp pkt_def.cpp loguru.cpp md5.cpp main.cpp comm.cpp util.cpp busi.cpp
OBJS1 = $(patsubst %.cpp,.obj/%.o,$(SRC1))
main: $(OBJS1)
	g++ $(LDFLAGS) -o $@ $(OBJS1) $(LIBS)

inc=../include
.obj/pkt.o: $(inc)/pkt.h
.obj/pkt_def.o: $(inc)/pkt.h $(inc)/pkt_def.h
.obj/pkt_def.o: $(inc)/pkt.h $(inc)/pkt_def.h $(inc)/main.h
.obj/main.o: $(inc)/main.h $(inc)/busi.h $(inc)/pkt.h $(inc)/busi.h
.obj/busi.o: $(inc)/busi.h $(inc)/pkt.h $(inc)/pkt_def.h

SRC2=pkt.cpp pkt_def.cpp loguru.cpp md5.cpp comm.cpp util.cpp sendcmd.cpp
OBJS2 = $(patsubst %.cpp,.obj/%.o,$(SRC2))
sendcmd: $(OBJS2)
	g++ $(LDFLAGS) -o $@ $(OBJS2) $(LIBS)

share: $(OBJS1)
	g++ $(LDFLAGS) -o share.dll $(OBJS1) -s -shared -Wl,--subsystem,windows $(LIBS)


$(OBJS): | .obj

.obj/%.o : %.cpp
	@echo $< 
	g++ $(CPPFLAGS) -Wno-unused-parameter -c $< -o $@

.obj:
	mkdir -p $@



json-test: .obj/json-test.o .obj/comm.o
	g++ $(LDFLAGS) -o $@ $^ $(LIBS)


#OBJS   = $(foreach var, $(addsuffix .$(OBJEXT), $(basename $(FILES)) $(RESOURCES)), $(if $(findstring ../,$(var)),$(notdir $(var)),$(var)))
#SRC    = $(FILES)

# Logging_Client: $(OBJS)
# 	echo "[[[$(OBJS)]]]"
# 	g++ $(LDFLAGS) -o $@ $(OBJS) $(LIBS)


# .cpp.o:
# 	g++ $(CPPFLAGS) -c $*.cpp
