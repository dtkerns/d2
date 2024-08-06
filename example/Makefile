
ifeq ($(ll),1)
export ll
endif

ifeq ($(llg),1)
export llg
endif

APPS = app1 app2
export CFLAGS = -O1

all: $(APPS)

app1 app2:
	$(MAKE) -C $@ $(filter-out test,$(MAKECMDGOALS))

clean:
	for d in $(APPS); do \
		$(MAKE) -C $$d $@; \
	done

test: all
	mkdir /tmp/testout$$; ../bin/d2 -i /tmp/testout$$

.PHONY: app1 app2
