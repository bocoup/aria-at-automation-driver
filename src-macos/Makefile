# This Makefile is loosely based on the following guide:
#
# OS X: Creating Packages from the Command Line - Tutorial and a Makefile - Part II
# http://thegreyblog.blogspot.com/2014/06/os-x-creating-packages-from-command.html

PROGRAM = AutomationVoice
VOLUME_NAME = Bocoup Automation Voice
IDENTIFIER = com.bocoup.automation-voice
DISTDIR = ./dist
DEPSDIR = ./deps
BINARYDIR = /tmp/AutomationVoice.dst
BINARY = $(BINARYDIR)/System/Library/Speech/Synthesizers/BenchmarkSynthesizer.SpeechSynthesizer/Contents/MacOS/BenchmarkSynthesizer
DMGFILE = $(PROGRAM).dmg
PRODUCT = $(DISTDIR)/$(PROGRAM).pkg
COMPONENT = $(DEPSDIR)/$(PROGRAM)Component.pkg
COMPONENT_PFILE = $(PROGRAM).plist
DISTRIBUTION_FILE = distribution.plist
REQUIREMENTS = requirements.plist

.PHONY: all
all: $(DISTDIR) $(DEPSDIR) $(PRODUCT) $(DMGFILE)

$(DISTDIR):
	mkdir $(DISTDIR)

$(DEPSDIR):
	mkdir $(DEPSDIR)

$(PRODUCT): $(BINARY) $(REQUIREMENTS) $(COMPONENT_PFILE) $(COMPONENT) $(DISTRIBUTION_FILE)
	productbuild \
		--distribution $(DISTRIBUTION_FILE) \
		--resources . \
		--package-path $(DEPSDIR) \
		--package-path $(COMPONENT) \
		$(PRODUCT)

$(BINARY):
	mkdir -p $(BINARYDIR)
	echo A
	xcodebuild install \
		DSTROOT=$(BINARYDIR) \
		MACOSX_DEPLOYMENT_TARGET=10.15 \
		SDKROOT=macosx10.15
	echo B

$(COMPONENT_PFILE):
	pkgbuild \
		--min-os-version 10.15 \
		--analyze \
		--root $(BINARYDIR) \
		$(COMPONENT_PFILE)

$(COMPONENT): $(DEPSDIR) $(BINARY) $(COMPONENT_PFILE)
	pkgbuild \
		--min-os-version 10.15 \
		--identifier $(IDENTIFIER) \
		--root $(BINARYDIR) \
		--component-plist $(COMPONENT_PFILE) \
		$(COMPONENT)

$(DISTRIBUTION_FILE): $(COMPONENT)
	productbuild \
		--synthesize \
		--product $(REQUIREMENTS) \
		--package $(COMPONENT) \
		$(DISTRIBUTION_FILE)

$(DMGFILE):
	hdiutil create \
		-volname "$(VOLUME_NAME)" \
		-srcfolder ./dist \
		-ov \
		$(DMGFILE)

.PHONY: clean
clean:
	-rm -f $(DMGFILE) $(PRODUCT) $(COMPONENT)
	-rm -rf $(BINARYDIR)

.PHONY: distclean
distclean:
	-rm -f $(DISTRIBUTION_FILE)

.PHONY: compclean
compclean:
	-rm -f $(COMPONENT_PFILE)
