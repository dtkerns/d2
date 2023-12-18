
ifeq ($(ll),1)
export ll
endif

ifeq ($(llg),1)
export llg
endif

APPS = app1 app2

all: $(APPS)

app1 app2:
	$(MAKE) -C $@ $(MAKECMDGOALS)

clean:
	for d in $(APPS); do \
		$(MAKE) -C $$d $@; \
	done

.PHONY: app1 app2
