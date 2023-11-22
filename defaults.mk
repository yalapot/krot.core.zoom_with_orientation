# set env.variable BuildKrotXML to folder with this file
# set env.variable PF to "Program Files" folder
# set PF = C:\Program Files (x86)

set PF = C:\Program Files

GNU_FOLDER = D:\gnu\bin
VB_FOLDER = $(PF)\Microsoft Visual Studio\VB98
VC_BIN_FOLDER = $(PF)\Microsoft Visual Studio 8\VC\bin

PRG_VB6 = "$(VB_FOLDER)\vb6.exe"
PRG_RC = "$(VB_FOLDER)\Wizards\rc.exe"
HHC = "$(PF)\HTML Help Workshop\hhc.exe"

DEL = $(GNU_FOLDER)\rm.exe
#MAKE = $(GNU_FOLDER)\make.exe
MAKE = make.exe

CORE_DIR = D:\work\krot.core.zoom_with_orientation
KROT_API = $(CORE_DIR)\krtapi2.h $(CORE_DIR)\krtapi.h $(CORE_DIR)\krtBase.h $(CORE_DIR)\compiler.h
WIN_LIB = kernel32.lib user32.lib gdi32.lib

cc = @"$(VC_BIN_FOLDER)\cl.exe" /nologo /I$(CORE_DIR)
LINK = @"$(VC_BIN_FOLDER)\link.exe" /NOLOGO /LIBPATH:$(BuildKrotXML)\lib
