.PHONY : build unit stat clean make_outdir

APP := a.out
UNIT_APP := unit_tests.exe
#STAT_APP := stat.exe

SRC_DIR := ./src
INC_DIR := ./inc
OUT_DIR := ./out
APPS_DIR := ./apps1

UNIT_DIR := ./unit_tests
FTEST_DIR := ./func_tests
STAT_DIR := ./stat

CC := gcc
CFLAGS := -std=c99 -I$(INC_DIR) -Wall -Wpedantic -Wextra -Wfloat-equal -Wfloat-conversion -Wvla -pthread
LD_FLAGS := `pkg-config --cflags --libs check`

ifeq ($(mode), debug)
    CFLAGS += -g3 -O0
endif

ifeq ($(mode), release)
    CFLAGS += -O3 -g0
endif

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(SRCS:$(SRC_DIR)/%.c=$(OUT_DIR)/%.o)
UNIT_SRCS := $(wildcard $(UNIT_DIR)/*.c)
UNIT_OBJS := $(UNIT_SRCS:$(UTEST_DIR)/%.c=$(OUT_DIR)/%.o)
STAT_SRCS := $(wildcard $(STAT_DIR)/*.c)
STAT_OBJS := $(STAT_SRCS:$(STAT_DIR)/%.c=$(OUT_DIR)/%.o)

build: $(APP)

unit: $(UNIT_APP)

stat: $(STAT_APP)

$(APP): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -lm $(LD_FLAGS)

$(UNIT_APP) : $(filter-out $(OUT_DIR)/main.o, $(OBJS)) $(UNIT_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -lm $(LD_FLAGS)

$(STAT_APP) : $(STAT_OBJS)
	$(CC) $(CFLAGS) -DN_MAX="$(CURR_SIZE)" -DCODE="$(CURR_CODE)" -o $@ $^ #$(LD_FLAGS)

make_outdir:
	mkdir -p $(OUT_DIR)

${OUT_DIR}/%.o : ${SRC_DIR}/%.c make_outdir
	$(CC) $(CFLAGS) -c $< -o $@

$(OUT_DIR)/check_%.o : $(UNIT_DIR)/check_%.c make_outdir
	$(CC) $(CFLAGS) -o $@ -c $<

$(OUT_DIR)/stat_%.o : $(STAT_DIR)/stat_%.c make_outdir
	$(CC) $(CFLAGS) -DN_MAX="$(CURR_SIZE)" -DCODE="$(CURR_CODE)" -DMYSORT_CODE="$(SORTC)" -o $@ -c $<

clean:
	rm -f $(APP) $(UNIT_EXEC) $(STAT_EXEC) ${OUT_DIR}/*
	rm -rf $(OUT_DIR)
