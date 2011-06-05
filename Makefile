# Options
CLIPSYNC_DIR=src/clipsync
SHELLCLIP_DIR=src/shellclip
GTKCLIP_DIR=src/gtkclip
REPORT_DIR=report

# Rules
all:
	@(cd $(CLIPSYNC_DIR) && $(MAKE))
	@chmod +x $(CLIPSYNC_DIR)/genconfig.py
	@chmod +x $(SHELLCLIP_DIR)/shellcopy.py
	@chmod +x $(SHELLCLIP_DIR)/shellpaste.py
	@chmod +x $(GTKCLIP_DIR)/gtkclip.py

.PHONY: clean mrproper doc report install uninstall

install: all
	cp $(CLIPSYNC_DIR)/clipsync.exe /usr/bin/clipsync
	cp $(CLIPSYNC_DIR)/genconfig.py /usr/bin/clipsync_genconfig
	cp $(SHELLCLIP_DIR)/shellcopy.py /usr/bin/shellcopy
	cp $(SHELLCLIP_DIR)/shellpaste.py /usr/bin/shellpaste
	cp $(GTKCLIP_DIR)/gtkclip.py /usr/bin/gtkclip

uninstall:
	rm /usr/bin/clipsync
	rm /usr/bin/shellcopy
	rm /usr/bin/shellpaste
	rm /usr/bin/gtkclip

doc:
	doxygen doxygen.conf

report:
	@(cd $(REPORT_DIR) && $(MAKE))

clean:
	@(cd $(CLIPSYNC_DIR) && $(MAKE) $@)
	@(cd $(REPORT_DIR) && $(MAKE) $@)

mrproper: clean
	@(cd $(CLIPSYNC_DIR) && $(MAKE) $@)
