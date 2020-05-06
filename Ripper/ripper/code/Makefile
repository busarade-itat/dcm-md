BINDIR= /home/wcohen/bin/${ARCH}/
DOCDIR= /home/wcohen/ripper/doc/man/man1
SYSBINDIR= /usr/local/bin
SYSMANDIR= /usr/local/man/man1

CC=gcc
CFLAGS= -O4 -DGCC -DSOLARIS
#CFLAGS= -g -DGCC
LDFLAGS=

#possible flags for compiler
# -DSAVEMEMORY     reduce memory usage, at some cost of run time
# -DGCC            declarations to suit usual gcc .h files
# -DPORTABLE       stubs out system-dependent timing calls
# -DSOLARIS        needed to compile on Solaris

EXTRA=	error.o trace.o memory.o time.o dougsort.o vector.o

MDB=	mdb_load.o mdb_names.o mdb_util.o mdb_count.o mdb_opt.o

OBJECT= mainaux.o model.o concept.o fit.o mdl.o value.o  \
	rule.o desref.o gram.o gramaux.o types.o extend-rules.o  \
	lex.o intern.o crossval.o add-redundancy.o ${MDB} ${EXTRA}

BOOSTOBJ= boost.o weak_rule.o

MAINS= ripper-main.o predict-main.o summarize-main.o \
       partition-main.o select-class-main.o rocchio-main.o pair-main.o \
       eliminate-main.o	pprint-rules-main.o test-rules-main.o \
       clean-data-main.o boost-main.o transform-main.o \
       data2text-main.o		
       
EXECS= ripper predict corrupt summarize verify partition corrupt \
       filter-text phrases select-class rocchio pair eliminate test-rules \
       pprint-rules clean-data boost transform \
       data2text

install: all
	cp ${EXECS} ${BINDIR}

system-install: all
	cp ${EXECS} ${SYSBINDIR}
	cp ${DOCDIR}/*.1 ${SYSMANDIR}

backup:
	cp *.c *.h Makefile Backups

all: ${EXECS}

ripper: ripper-main.o ${OBJECT} 
	${CC} ${LDFLAGS} -o ripper ripper-main.o ${OBJECT} -lm ${CLIB}

boost: boost-main.o  ${OBJECT} ${BOOSTOBJ}
	${CC} ${LDFLAGS} -o boost boost-main.o ${OBJECT} ${BOOSTOBJ} -lm ${CLIB}

data2text: data2text-main.o  ${OBJECT}
	${CC} ${LDFLAGS} -o data2text data2text-main.o ${OBJECT} -lm ${CLIB}

#frame-text: frame-text-main.o ${OBJECT} 
#	${CC} ${LDFLAGS} -o frame-text frame-text-main.o ${OBJECT} -lm ${CLIB}

transform: transform-main.o ${OBJECT} 
	${CC} ${LDFLAGS} -o transform transform-main.o ${OBJECT} -lm ${CLIB}

predict: predict-main.o ${OBJECT} 
	${CC} ${LDFLAGS} -o predict predict-main.o ${OBJECT} -lm ${CLIB}

test-rules: test-rules-main.o ${OBJECT}
	${CC} ${LDFLAGS} -o test-rules test-rules-main.o ${OBJECT} -lm ${CLIB}

pprint-rules: pprint-rules-main.o ${OBJECT}
	${CC} ${LDFLAGS} -o pprint-rules pprint-rules-main.o ${OBJECT} -lm ${CLIB}

eliminate: eliminate-main.o ${OBJECT}
	${CC} ${LDFLAGS} -o eliminate eliminate-main.o ${OBJECT} -lm ${CLIB}

#add-redundancy: add-redundancy-main.o ${OBJECT}
#	${CC} ${LDFLAGS} -o add-redundancy add-redundancy-main.o ${OBJECT} -lm ${CLIB}

pair: pair-main.o ${OBJECT} 
	${CC} ${LDFLAGS} -o pair pair-main.o ${OBJECT} -lm ${CLIB}

rocchio: rocchio-main.o ${OBJECT} 
	${CC} ${LDFLAGS} -o rocchio rocchio-main.o ${OBJECT} -lm ${CLIB}

select-class: select-class-main.o ${OBJECT} 
	${CC} ${LDFLAGS} -o select-class select-class-main.o ${OBJECT} -lm ${CLIB}

#browser: browser-main.o ${OBJECT} 
#	${CC} ${LDFLAGS} -o browser browser-main.o ${OBJECT} -lm ${CLIB}

#browser-index: browser-index-main.o ${OBJECT} 
#	${CC} ${LDFLAGS} -o browser-index browser-index-main.o ${OBJECT} -lm ${CLIB}

#browse: browse.pl
#	cp browse.pl browse

#browser-html: browser-html-main.o ${OBJECT} 
#	${CC} ${LDFLAGS} -o browser-html browser-html-main.o ${OBJECT} -lm ${CLIB}

partition: partition-main.o ${OBJECT} 
	${CC} ${LDFLAGS} -o partition partition-main.o ${OBJECT} -lm ${CLIB}

summarize: summarize-main.o ${OBJECT} 
	${CC} ${LDFLAGS} -o summarize summarize-main.o ${OBJECT} -lm ${CLIB}

verify: verify-main.o ${OBJECT} 
	${CC} ${LDFLAGS} -o verify verify-main.o ${OBJECT} -lm ${CLIB}

corrupt: corrupt-main.o ${OBJECT} 
	${CC} ${LDFLAGS} -o corrupt corrupt-main.o ${OBJECT} -lm ${CLIB}

clean-data: clean-data-main.o ${OBJECT} 
	${CC} ${LDFLAGS} -o clean-data clean-data-main.o ${OBJECT} -lm ${CLIB}

filter-text: filter-text-main.o ${OBJECT} 
	${CC} ${LDFLAGS} -o filter-text filter-text-main.o ${OBJECT} -lm ${CLIB}

phrases: phrases-main.o ${OBJECT} 
	${CC} ${LDFLAGS} -o phrases phrases-main.o ${OBJECT} -lm ${CLIB}

tar:
	tar -cvf ripper.tar Makefile *.h *.c
	gzip ripper.tar

shar: 
	cd ..; \
	shard ripper.shar code code/*.c code/*.h code/Makefile* \
 	pdata tdata \
	pdata/promoters.names pdata/promoters.data \
	pdata/labor.names pdata/labor.data \
	pdata/vote1.names pdata/vote1.test pdata/vote1.data \
	pdata/move1.names pdata/move1.info \
	pdata/move1.data pdata/move1.test pdata/move1dir.gram \
	pdata/move1dir2.gram \
	tdata/talks.names tdata/talks.data \
	tdata/talks.test tdata/talks.info \
	doc doc/man doc/man/man1 \
	doc/man/man1/ripper.1 doc/man/man1/ripperaux.1 \
	doc/ml95-paper.ps doc/aaai96-paper.ps

release-tar:
	cd ..; \
	tar -vcf ripper.tar code/*.c code/*.h code/Makefile \
	pdata/promoters.names pdata/promoters.data \
	pdata/labor.names pdata/labor.data \
	pdata/vote1.names pdata/vote1.test pdata/vote1.data \
	pdata/move1.names pdata/move1.info \
	pdata/move1.data pdata/move1.test pdata/move1dir.gram \
	tdata/talks.names tdata/talks.data \
	tdata/talks.test tdata/talks.info \
	doc/man/man1/ripper.1 doc/man/man1/ripperaux.1 \
	doc/ml95-paper.ps doc/aaai96-paper.ps

clean: 
	rm -f *.o
	rm -f ${EXECS}

for-sun:
	cp Makefile.sun Makefile

for-mips:
	cp Makefile.mips Makefile

${MAINS} ${OBJECT}: vector.h ripper.h protos.h mdb.h

${BOOSTOBJ} boost-main.o: vector.h ripper.h protos.h mdb.h boost.h

${EXTRA}: vector.h extras.h
