.PHONY: all clean dll tests flake8

include $(BuildKrotXML)\defaults.mk
PYTHON = C:\Python27\python.exe

TARGET = krotw32.dll
TEST_TARGET = test.exe
KROT_DEF = krotw32.def

KROT_LIST =     \
 pallete.obj    \
 zoom.obj       \
 draw.obj       \
 krotw32.obj    \
 scan.obj       \
 scan2013.obj   \
 filter.obj     \
 util.obj       \
 debuglog.obj   \
 driver.obj

TEST_LIST =     \
 test.obj       \
 tester.obj

KROTW32_API = krotw32.h $(KROT_API) dai.h vbapi.h pallete.h driver.h debuglog.h

DRV_FOLDER = drivers

all: dll
	$(MAKE) -C $(DRV_FOLDER)

tests: flake8
	@$(PYTHON) check_ini.py drivers.gpas.ini
	@$(PYTHON) check_ini.py drivers.arkaline.ini
	@$(PYTHON) check_ini.py drivers.viewer.ini

flake8:
	@$(PYTHON) -m flake8 --max-line-length=110 .

dll: $(TARGET) $(TEST_TARGET)

$(KROT_LIST): %.obj: %.c
	$(cc) /c /DWIN32 /DDLL /GA /O2 /Ot /W4 /wd4996 $<

$(TEST_LIST): %.obj: %.cpp
	$(cc) /c /DWIN32 /GA /EHsc /O2 /Ot /W3 $<

pallete.obj: $(KROTW32_API)
zoom.obj: $(KROTW32_API)
draw.obj: $(KROTW32_API)
krotw32.obj: scan.h $(KROTW32_API)
scan.obj: debuglog.h scan.h filter.h $(KROT_API)
scan2013.obj: debuglog.h scan.h $(KROT_API)
filter.obj: debuglog.h filter.h $(KROT_API)
util.obj: $(KROTW32_API)
debuglog.obj: debuglog.h
driver.obj: driver.h $(KROT_API)

test.obj: tester.h filter.h $(KROTW32_API)
tester.obj: tester.h

$(TARGET): $(KROT_DEF) $(KROT_LIST)
	$(LINK) /DLL /DEF:$(KROT_DEF) /OUT:$(TARGET) $(KROT_LIST) $(WIN_LIB)

$(TEST_TARGET): $(TEST_LIST) $(KROT_LIST)
	$(LINK) /SUBSYSTEM:CONSOLE /OUT:$(TEST_TARGET) $(TEST_LIST) $(KROT_LIST) $(WIN_LIB)

clean:
	$(MAKE) -C $(DRV_FOLDER) clean
	$(RM) $(TARGET) $(TEST_TARGET) $(KROT_LIST) $(TEST_LIST) *.lib *.exp
