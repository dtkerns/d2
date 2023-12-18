CFLAGS +=  -std=c99
LDFLAGS += -lm

SRC = app2.c
APP = $(SRC:.c=)

ifeq ($(ll),1)
CFLAGS+= -S -emit-llvm
SFX=ll
APP=$(SRC:.c=.$(SFX))
endif

ifeq ($(llg),1)
CFLAGS+= -S -emit-llvm -g
SFX=llg
APP=$(SRC:.c=.$(SFX))
endif

all: $(APP)

app2: app2.c

%.$(SFX): %.c
	clang $(CFLAGS) $^ -o $@

clean:
	$(RM) $(APP) *.ll *.llg
