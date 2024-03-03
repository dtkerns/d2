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
	@echo export LLVMVER=$(LLVMVER) > ../bin/.config
	@echo export NEWPASSMANAGER=$(word 1,$(NEWPASSMANAGER)) >> ../bin/.config
	@test $$(echo $(LLVMVER) | cut -d. -f1) -ge 19 || echo "[7mwarning: due to a bug in LLVM versions < 19, you will need to add a -O1 (minimum) to CFLAGS in your makefiles[m"

deps:
	@which awk cpio cut find grep python sed sort ; test $$? = 0 || (echo please install any commands not found; exit 1)

.PHONY: deps versioncheck all
