CC=gcc
PREFIX=/usr
DOCDIR=${PREFIX}/share/doc/starcrack
BINARY=starcrack

all:${BINARY}

${BINARY}: ${BINARY}.c ${BINARY}.h
	${CC} -std=c99 -Wall -Wextra -pedantic -O3 -pthread starcrack.c `xml2-config --libs --cflags` -o starcrack

clean:
	-rm *.o starcrack

install:
	install -s starcrack ${PREFIX}/bin
	-mkdir -p ${DOCDIR}
	chmod 755 ${DOCDIR}
	install -m 644 -t ${DOCDIR} CHANGELOG LICENSE README README.html RELEASE_NOTES

uninstall:
	-rm ${PREFIX}/bin/starcrack

