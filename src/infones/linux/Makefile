CC = gcc
TARBALL = InfoNES08J

# InfoNES
.CFILES =	./../K6502.cpp \
		./../InfoNES.cpp \
		./../InfoNES_Mapper.cpp \
		./../InfoNES_pAPU.cpp \
		./InfoNES_System_Linux.cpp

.OFILES	=	$(.CFILES:.cpp=.o)

CCFLAGS = `gtk-config --cflags` -O2
LDFILGS = `gtk-config --libs gthread` -lstdc++		# gcc3.x.x

all: InfoNES

InfoNES: $(.OFILES)
	$(CC) $(INCLUDES) -o $@ $(.OFILES) $(LDFILGS) -lm -lz

.cpp.o:
	$(CC) $(INCLUDES) -c $(CCFLAGS) $*.cpp  -o $@

clean:
	rm -f $(.OFILES) ../*~ ../*/*~ core

cleanall:
	rm -f $(.OFILES) ../*~ ../*/*~ core InfoNES

release: clean all

tar:
	( cd ..; \
	tar cvf $(TARBALL).tar ./*; \
	gzip $(TARBALL).tar \
	)

install:
	install ./InfoNES /usr/local/bin
