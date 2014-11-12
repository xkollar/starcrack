CC=gcc
PREFIX=/usr
DOCDIR=${PREFIX}/share/doc/rarcrack
BINARY=rarcrack

all:${BINARY}

${BINARY}: ${BINARY}.c ${BINARY}.h
	${CC} -std=c99 -Wall -Wextra -pedantic -O3 -pthread rarcrack.c `xml2-config --libs --cflags` -o rarcrack

clean:
	-rm *.o rarcrack

install:
	install -s rarcrack ${PREFIX}/bin
	-mkdir -p ${DOCDIR}
	chmod 755 ${DOCDIR}
	install -m 644 -t ${DOCDIR} CHANGELOG LICENSE README README.html RELEASE_NOTES

uninstall:
	-rm ${PREFIX}/bin/rarcrack

