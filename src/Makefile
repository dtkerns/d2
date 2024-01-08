LLVMVER=$(shell opt --version | awk '/LLVM version/{print $$NF}')
NEWPASSMANAGER=$(if $(shell opt --help | grep -q -- --passes=;echo $?),implemented,not implemented)
$(info LLVMVER=$(LLVMVER) NEWPASSMANAGER=$(NEWPASSMANAGER))

install: versioncheck all
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

.PHONY: versioncheck all
