# SPDX-License-Identifier: CC0-1.0
#
# SPDX-FileContributor: Adrian "asie" Siekierka, 2024

export WONDERFUL_TOOLCHAIN ?= /opt/wonderful
export BLOCKSDS ?= /opt/blocksds/core

# Tools
# -----

LUA		:= $(WONDERFUL_TOOLCHAIN)/bin/wf-lua
DLDIPATCH	:= $(BLOCKSDS)/tools/dldipatch/dldipatch
NDSTOOL		:= $(BLOCKSDS)/tools/ndstool/ndstool
CP		:= cp
MAKE		:= make
MKDIR		:= mkdir
RM		:= rm -rf

# Verbose flag
# ------------

ifeq ($(V),1)
_V		:=
else
_V		:= @
endif

# Build rules
# -----------

ARM9ELF		:= build/arm9.elf
ARM7ELF		:= build/arm7.elf
NDSROM		:= build/miniboot.nds

SCRIPT_R4CRYPT		:= scripts/r4crypt.lua
SCRIPT_DSBIZE		:= scripts/dsbize

NDSROM_ACE3DS_DLDI	:= blobs/dldi/ace3ds_sd.dldi
NDSROM_AK2_DLDI		:= blobs/dldi/ak2_sd.dldi
NDSROM_DSONE_DLDI	:= blobs/dldi/scds3.dldi
NDSROM_EZ5_DLDI		:= blobs/dldi/ez5h.dldi
NDSROM_GMTF_DLDI	:= blobs/dldi/gmtf.dldi
NDSROM_M3DS_DLDI	:= blobs/dldi/m3ds.dldi
NDSROM_R4_DLDI		:= blobs/dldi/r4tfv3.dldi
NDSROM_R4IDSN_DLDI	:= blobs/dldi/r4idsn_sd.dldi

NDSROM_ACE3DS		:= dist/ace3dsplus/_ds_menu.dat
NDSROM_AK2		:= dist/generic/akmenu4.nds
NDSROM_DSONE	:= dist/generic/scfw.sc
NDSROM_EZ5		:= dist/generic/ez5sys.bin
NDSROM_GMTF		:= dist/generic/bootme.nds
NDSROM_GWBLUE		:= dist/gwblue/_dsmenu.dat
NDSROM_ITDS_ENG		:= dist/m3ds/boot.eng
NDSROM_ITDS_GB		:= dist/m3ds/boot.gb
NDSROM_ITDS_JP		:= dist/m3ds/boot.jp
NDSROM_M3DS		:= dist/m3ds/SYSTEM/g6dsload.eng
NDSROM_R4		:= dist/generic/_DS_MENU.DAT
NDSROM_R4IDSN		:= dist/r4idsn/_dsmenu.dat
NDSROM_R4ILS		:= dist/ace3dsplus/_dsmenu.dat
NDSROM_R4IRTSB		:= dist/m3ds/_ds_menu.sys
NDSROM_R4ITT		:= dist/r4itt/_ds_menu.dat
NDSROM_R4RTS		:= dist/m3ds/loader.eng

.PHONY: all clean arm9 arm7

all: \
	$(NDSROM) \
	$(NDSROM_ACE3DS) \
	$(NDSROM_AK2) \
	$(NDSROM_DSONE) \
	$(NDSROM_EZ5) \
	$(NDSROM_GMTF) \
	$(NDSROM_GWBLUE) \
	$(NDSROM_ITDS_ENG) \
	$(NDSROM_ITDS_GB) \
	$(NDSROM_ITDS_JP) \
	$(NDSROM_M3DS) \
	$(NDSROM_R4) \
	$(NDSROM_R4IDSN) \
	$(NDSROM_R4ILS) \
	$(NDSROM_R4IRTSB) \
	$(NDSROM_R4ITT) \
	$(NDSROM_R4RTS)
	$(_V)$(CP) LICENSE README.md dist/

$(SCRIPT_DSBIZE): scripts/dsbize.c
	$(_V)$(CC) -o $@ $<

$(NDSROM_ACE3DS): $(NDSROM) $(NDSROM_ACE3DS_DLDI) $(SCRIPT_R4CRYPT)
	@$(MKDIR) -p $(@D)
	@echo "  DLDI    $@"
	$(_V)$(CP) $(NDSROM) $@
	$(_V)$(DLDIPATCH) patch $(NDSROM_ACE3DS_DLDI) $@
	@echo "  R4CRYPT $@"
	$(_V)$(LUA) $(SCRIPT_R4CRYPT) $@ 4002

$(NDSROM_GWBLUE): arm9 arm7 $(NDSROM_ACE3DS_DLDI) $(SCRIPT_R4CRYPT)
	@$(MKDIR) -p $(@D)
	@echo "  NDSTOOL $@"
	$(_V)$(BLOCKSDS)/tools/ndstool/ndstool -c $@ \
		-9 build/arm9.bin -7 build/arm7.bin \
		-r7 0x2380000 -e7 0x2380000 \
		-r9 0x2000450 -e9 0x2000450 -h 0x200 \
		-g "####" "##" "R4IT"
	@echo "  DLDI    $@"
	$(_V)$(DLDIPATCH) patch $(NDSROM_ACE3DS_DLDI) $@
	@echo "  R4CRYPT $@"
	$(_V)$(LUA) $(SCRIPT_R4CRYPT) $@ 4002

$(NDSROM_R4ILS): arm9 arm7 $(NDSROM_ACE3DS_DLDI) $(SCRIPT_R4CRYPT)
	@$(MKDIR) -p $(@D)
	@echo "  NDSTOOL $@"
	$(_V)$(BLOCKSDS)/tools/ndstool/ndstool -c $@ \
		-9 build/arm9.bin -7 build/arm7.bin \
		-r7 0x2380000 -e7 0x2380000 \
		-r9 0x2000450 -e9 0x2000450 -h 0x200 \
		-g "####" "##" "R4XX"
	@echo "  DLDI    $@"
	$(_V)$(DLDIPATCH) patch $(NDSROM_ACE3DS_DLDI) $@
	@echo "  R4CRYPT $@"
	$(_V)$(LUA) $(SCRIPT_R4CRYPT) $@ 4002

$(NDSROM_R4IDSN): arm9 arm7 $(NDSROM_R4IDSN_DLDI)
	@$(MKDIR) -p $(@D)
	@echo "  NDSTOOL $@"
	$(_V)$(BLOCKSDS)/tools/ndstool/ndstool -c $@ \
		-9 build/arm9.bin -7 build/arm7.bin \
		-r7 0x2380000 -e7 0x2380000 \
		-r9 0x2000000 -e9 0x2000000 -h 0x200
	@echo "  DLDI    $@"
	$(_V)$(DLDIPATCH) patch $(NDSROM_R4IDSN_DLDI) $@

$(NDSROM_R4ITT): arm9 arm7 $(NDSROM_AK2_DLDI)
	@$(MKDIR) -p $(@D)
	@echo "  NDSTOOL $@"
	$(_V)$(BLOCKSDS)/tools/ndstool/ndstool -c $@ \
		-9 build/arm9.bin -7 build/arm7.bin \
		-r7 0x2380000 -e7 0x2380000 \
		-r9 0x2000800 -e9 0x2000800 -h 0x200
	@echo "  DLDI    $@"
	$(_V)$(DLDIPATCH) patch $(NDSROM_AK2_DLDI) $@

$(NDSROM_DSONE): arm9 arm7 $(NDSROM_DSONE_DLDI)
	@$(MKDIR) -p $(@D)
	@echo "  NDSTOOL $@"
	$(_V)$(BLOCKSDS)/tools/ndstool/ndstool -c $@ \
		-9 build/arm9.bin -7 build/arm7.bin \
		-r7 0x2380000 -e7 0x2380000 \
		-r9 0x2000450 -e9 0x2000450 -h 0x200 \
		-g "ENG0"
	@echo "  DLDI    $@"
	$(_V)$(DLDIPATCH) patch $(NDSROM_DSONE_DLDI) $@

$(NDSROM_M3DS): arm9 arm7 $(NDSROM_M3DS_DLDI) $(SCRIPT_DSBIZE)
	@$(MKDIR) -p $(@D)
	@echo "  NDSTOOL $@"
	$(_V)$(BLOCKSDS)/tools/ndstool/ndstool -c $@ \
		-9 build/arm9.bin -7 build/arm7.bin \
		-r7 0x23ad800 -e7 0x23ad800 \
		-r9 0x2380000 -e9 0x2380000 -h 0x200
	@echo "  DLDI    $@"
	$(_V)$(DLDIPATCH) patch $(NDSROM_M3DS_DLDI) $@
	@echo "  DSBIZE  $@"
	$(_V)./$(SCRIPT_DSBIZE) $@ 0x12
	@printf "kari \012" > $(@D)/g6dsload.1

$(NDSROM_ITDS_ENG): arm9 arm7 $(NDSROM_M3DS_DLDI) $(SCRIPT_DSBIZE)
	@$(MKDIR) -p $(@D)
	@echo "  NDSTOOL $@"
	$(_V)$(BLOCKSDS)/tools/ndstool/ndstool -c $@ \
		-9 build/arm9.bin -7 build/arm7.bin \
		-r7 0x23ad800 -e7 0x23ad800 \
		-r9 0x2380000 -e9 0x2380000 -h 0x200
	@echo "  DLDI    $@"
	$(_V)$(DLDIPATCH) patch $(NDSROM_M3DS_DLDI) $@
	@echo "  DSBIZE  $@"
	$(_V)./$(SCRIPT_DSBIZE) $@ 0x32

$(NDSROM_ITDS_GB): arm9 arm7 $(NDSROM_M3DS_DLDI) $(SCRIPT_DSBIZE)
	@$(MKDIR) -p $(@D)
	@echo "  NDSTOOL $@"
	$(_V)$(BLOCKSDS)/tools/ndstool/ndstool -c $@ \
		-9 build/arm9.bin -7 build/arm7.bin \
		-r7 0x23ad800 -e7 0x23ad800 \
		-r9 0x2380000 -e9 0x2380000 -h 0x200
	@echo "  DLDI    $@"
	$(_V)$(DLDIPATCH) patch $(NDSROM_M3DS_DLDI) $@
	@echo "  DSBIZE  $@"
	$(_V)./$(SCRIPT_DSBIZE) $@ 0x33

$(NDSROM_ITDS_JP): arm9 arm7 $(NDSROM_M3DS_DLDI) $(SCRIPT_DSBIZE)
	@$(MKDIR) -p $(@D)
	@echo "  NDSTOOL $@"
	$(_V)$(BLOCKSDS)/tools/ndstool/ndstool -c $@ \
		-9 build/arm9.bin -7 build/arm7.bin \
		-r7 0x23ad800 -e7 0x23ad800 \
		-r9 0x2380000 -e9 0x2380000 -h 0x200
	@echo "  DLDI    $@"
	$(_V)$(DLDIPATCH) patch $(NDSROM_M3DS_DLDI) $@
	@echo "  DSBIZE  $@"
	$(_V)./$(SCRIPT_DSBIZE) $@ 0x37

$(NDSROM_R4IRTSB) $(NDSROM_R4RTS): arm9 arm7 $(NDSROM_M3DS_DLDI) $(SCRIPT_DSBIZE)
	@$(MKDIR) -p $(@D)
	@echo "  NDSTOOL $@"
	$(_V)$(BLOCKSDS)/tools/ndstool/ndstool -c $@ \
		-9 build/arm9.bin -7 build/arm7.bin \
		-r7 0x23ad800 -e7 0x23ad800 \
		-r9 0x2380000 -e9 0x2380000 -h 0x200
	@echo "  DLDI    $@"
	$(_V)$(DLDIPATCH) patch $(NDSROM_M3DS_DLDI) $@
	@echo "  DSBIZE  $@"
	$(_V)./$(SCRIPT_DSBIZE) $@ 0x72

$(NDSROM_R4): $(NDSROM) $(NDSROM_R4_DLDI) $(SCRIPT_R4CRYPT)
	@$(MKDIR) -p $(@D)
	@echo "  DLDI    $@"
	$(_V)$(CP) $(NDSROM) $@
	$(_V)$(DLDIPATCH) patch $(NDSROM_R4_DLDI) $@
	@echo "  R4CRYPT $@"
	$(_V)$(LUA) $(SCRIPT_R4CRYPT) $@

$(NDSROM_AK2): $(NDSROM) $(NDSROM_AK2_DLDI)
	@$(MKDIR) -p $(@D)
	@echo "  DLDI    $@"
	$(_V)$(CP) $(NDSROM) $@
	$(_V)$(DLDIPATCH) patch $(NDSROM_AK2_DLDI) $@

$(NDSROM_EZ5): $(NDSROM) $(NDSROM_EZ5_DLDI)
	@$(MKDIR) -p $(@D)
	@echo "  DLDI    $@"
	$(_V)$(CP) $(NDSROM) $@
	$(_V)$(DLDIPATCH) patch $(NDSROM_EZ5_DLDI) $@

$(NDSROM_GMTF): $(NDSROM) $(NDSROM_GMTF_DLDI)
	@$(MKDIR) -p $(@D)
	@echo "  DLDI    $@"
	$(_V)$(CP) $(NDSROM) $@
	$(_V)$(DLDIPATCH) patch $(NDSROM_GMTF_DLDI) $@

$(NDSROM): arm9 arm7
	@$(MKDIR) -p $(@D)
	@echo "  NDSTOOL $@"
	$(_V)$(BLOCKSDS)/tools/ndstool/ndstool -c $@ \
		-9 build/arm9.bin -7 build/arm7.bin \
		-r7 0x2380000 -e7 0x2380000 \
		-r9 0x2000450 -e9 0x2000450 -h 0x200

clean:
	@echo "  CLEAN"
	$(_V)$(RM) build dist $(SCRIPT_DSBIZE)

arm9:
	$(_V)+$(MAKE) -f Makefile.miniboot CPU=arm9 --no-print-directory

arm7:
	$(_V)+$(MAKE) -f Makefile.miniboot CPU=arm7 --no-print-directory

$(ROM): arm9 arm7
