LLVMVER=$(shell opt --version | awk '/LLVM version/{print $$NF}')
NEWPASSMANAGER=$(if $(shell opt --help | sed -n '/--passes=/p'),implemented,not implemented)
$(info LLVMVER=$(LLVMVER) NEWPASSMANAGER=$(NEWPASSMANAGER))

install: deps versioncheck all
	mkdir -p ../bin
	mv sb ../bin

all: sb

sb: sb.cc

clean:
	$(RM) ../bin/sb

versioncheck:
	@echo "you appear to be running LLVM version $(LLVMVER) and the new passmanager is $(NEWPASSMANAGER)"
	@echo LLVMVER=$(LLVMVER) > ../bin/.config
	@echo NEWPASSMANAGER=$(word 1,$(NEWPASSMANAGER)) >> ../bin/.config

deps:
	@which awk cpio find grep python sed sort ; test $$? = 0 || (echo please install any commands not found; exit 1)

.PHONY: deps versioncheck all
