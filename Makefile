#Probably also require bison and flex configured.
CUDD=./cudd
MTL=../mtl-2.1.2-21
READLINE=/home/discus/disk1/charlesg/downloads/readline-4.3
CXX=g++

include Makefile.inc

TARGET=nmrdpp
FLEX_LIB=-L/usr/local/flex-2.5.4/lib/ -lfl

EFENCE=-lefence -lpthread
GENERATED=commandParser commandScanner parser lexer
CLASSES=RegistryItem RegistryItemString RegistryItemFunction Registry \
	CommandListener CommandInterpreter main \
	ADDreward Utils ExtADD \
	ADDvisitor Spudd SpuddWrapper StructuredPLTLtranslatorWrapper \
	StructuredPLTLtranslator ParserWrapper DomainInspector Diagram \
	StateBasedSolutionWrapper Timer ActionADD ActionADDmap \
	AutomaticConstraintGenerator WallTimer CPUtimer MemoryMonitor \
	RandomDomainWrapper ADDstateCounter RandomActionSpec \
	RandomReward Random AutomaticConstraintGeneratorWrapper \
	StructuredSolutionWrapper
NONAUTO=actionSpecification formulaSequenceTraversal Algorithm \
	domainSpecification formula formulaCopier formulaEquality \
	formulaNormaliser formulaPrinter formulaSimplifier \
	formulaLength formulaSize \
	formulaVisitation rewardSpecification \
	FLTLrewardSpecification PLTLrewardSpecification \
	Expansion States domainSpecification_Anytime_or_Explicit \
	formulaUtilities PhaseI EntailmentFilter RewardCalculation \
	Preprocessors LAO ValueIteration PolicyIteration \
	MeasurementThreads Thread preprocessor
OBJECTS=$(CLASSES:=.o) $(GENERATED:=.o) $(NONAUTO:=.o)
HEADERS=$(CLASSES:=.h++)
CLEAN_EXTRA=commandParser.h++ parser.h++
DEPINCLUDES=$(patsubst %.o,.deps/%.Po, $(OBJECTS))
LDFLAGS=$(FLEX_LIB) $(CUDD_LIB) $(MTL_LIB) $(READLINE_LIB) -lpthread
CFLAGS=-ggdb -Wall -I. $(CUDD_CFLAGS) $(READLINE_CFLAGS) $(MTL_CFLAGS) $(EXTRA_CFLAGS)

all: $(GENERATED:=.c++) $(HEADERS) nmrdpp

$(TARGET): $(OBJECTS)
	$(CXX) -o $@ $(OBJECTS) $(CFLAGS) $(LDFLAGS)

%.o: %.c++
	source='$<' object='$@' libtool=no \
	depfile='.deps/$(patsubst %.o,%.Po,$@)' \
	tmpdepfile='.deps/$(patsubst %.o,%.TPo,$@)' \
	depmode='$(DEPMODE)' \
	/bin/sh ./depcomp \
	$(CXX) $(CFLAGS) $< -c -o $@

$(HEADERS): %.h++: %.c++
	./update-header $< $@ $(MAKE_HEADER_OPTS)

$(HEADERS): make-header

headers: $(HEADERS)

%.c++: %.y++
	bison -p$(patsubst %parser.y++,%,$(patsubst %Parser.y++,%,$<))yy -d  -o $@  $<
	mv $@.h $(patsubst %.c++,%,$@).h++ 2>/dev/null || true

%.c++: %.l++
	flex -P$(patsubst %lexer.l++,%,$(patsubst %Scanner.l++,%,$<))yy -o$@ $<

clean:
	rm -f $(HEADERS) $(OBJECTS) $(TARGET) $(GENERATED:=.c++) $(CLEAN_EXTRA)

test: $(TARGET)
	bash ./run-tests

cvs-report: cvs-report.ps

cvs-report.ps: cvs-report-inc.tex cvs-report.tex
	latex cvs-report.tex
	dvips cvs-report.dvi -o $@

cvs-report-inc.tex: generate-cvs-report.rb
	ruby ./generate-cvs-report.rb > $@

include .deps/*.Po
