NAME=starcrack
CC=gcc
PREFIX=/usr
DOCDIR=${PREFIX}/share/doc/${NAME}
BINARY=${NAME}

all:${BINARY}

${BINARY}: ${BINARY}.c ${BINARY}.h
	${CC} -std=c99 -Wall -Wextra -pedantic -O3 -pthread ${BINARY}.c `xml2-config --libs --cflags` -o ${BINARY}

clean:
	-rm *.o ${BINARY}

install:
	install -s ${BINARY} ${PREFIX}/bin
	-mkdir -p ${DOCDIR}
	chmod 755 ${DOCDIR}
	install -m 644 -t ${DOCDIR} CHANGELOG LICENSE README README.html RELEASE_NOTES

uninstall:
	-rm ${PREFIX}/bin/${BINARY}
	-rm ${DOCDIR}

