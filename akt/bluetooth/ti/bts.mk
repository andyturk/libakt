BUILD = build
OBJ = $(BUILD)/host
LIBAKT = ../libakt

CC2564_OEM_URL = http://processors.wiki.ti.com/images/1/1e/CC2564_BT_BLE_SP_BTS.zip
CC2564_OEM_BTS = $(wildcard $(BUILD)/$(basename $(notdir $(CC2564_OEM_URL)))/*.bts)

BTS_SOURCES = $(LIBAKT)/akt/bluetooth/bts.cc
BTS_OBJECTS = $(patsubst $(LIBAKT)/%.cc,$(OBJ)/%.o,$(BTS_SOURCES))
BUILD_DIRS = $(sort $(dir $(BTS_OBJECTS) $(BUILD)))
CFLAGS += -g
CFLAGS += -I$(LIBAKT)

BTS = $(BUILD)/bts

vpath $(sort $(dir $(BTS_SOURCES)))

default : $(BUILD)/bluetooth_init_cc2564.cc

clean :
	rm -rf $(BUILD)

get_patch :
	mkdir -p $(BUILD)
	curl -o $(BUILD)/$(notdir $(CC2564_OEM_URL)) $(CC2564_OEM_URL)
	unzip -d $(BUILD) $(BUILD)/$(notdir $(CC2564_OEM_URL))

test :
	@echo $(CC2564_OEM_BTS)

$(BTS_OBJECTS) : | $(BUILD_DIRS)

$(BUILD_DIRS) :
	mkdir -p $@

$(OBJ)/%.o : $(LIBAKT)/%.cc
	$(CXX) -c $(CFLAGS) $< -o $@

$(BTS) : $(BTS_OBJECTS)
	$(CXX) -o $(BTS) $(BTS_OBJECTS)

$(BUILD)/bluetooth_init_cc2564.cc : $(BTS) $(CC2564_OEM_BTS)
ifeq ($(CC2564_OEM_BTS),)
	@echo OEM patch not found
	@echo please run "make [...] get_patch" first
else
	$(BTS) $(CC2564_OEM_BTS) bluetooth_init_cc2564 >$(BUILD)/bluetooth_init_cc2564.cc
endif


