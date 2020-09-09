# Copyright (c) 2014-2018 The Khronos Group Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# OpenVX Specification makefile
#
# To build the spec with a specific version included, set the
# $(VERSIONS) variable on the make command line to a space-separated
# list of version names (e.g. VX_VERSION_1_2) *including all previous
# versions of the API* (e.g. VX_VERSION_1_2 must also include
# VX_VERSION_1_1). $(VERSIONS) is converted into asciidoc and generator
# script arguments $(VERSIONATTRIBS) and $(VERSIONOPTIONS)
#
# To build the specification and reference pages with optional
# extensions included, set the $(EXTENSIONS) variable on the make
# command line to a space-separated list of extension names.
# $(EXTENSIONS) is converted into asciidoc and generator script
# arguments $(EXTATTRIBS) and $(EXTOPTIONS).

VERSIONS :=
VERSIONATTRIBS := $(foreach version,$(VERSIONS),-a $(version))
VERSIONOPTIONS := $(foreach version,$(VERSIONS),-feature $(version))

EXTS := $(sort $(EXTENSIONS) $(DIFFEXTENSIONS))
EXTATTRIBS := $(foreach ext,$(EXTS),-a $(ext))
EXTOPTIONS := $(foreach ext,$(EXTS),-extension $(ext))

TARGET = pdf

# APITITLE can be set to extra text to append to the document title,
# normally used when building with extensions included.
APITITLE =

# The default 'all' target builds the following sub-targets for
# the spec whose base name is $(SPECBASE) (default 'OpenVX_Specification'):
#  html - HTML single-page API specification
#  pdf - PDF single-page API specification
# Other targets (not yet working for OpenVX)
#  manhtml - HTML5 single-page reference guide
#  manpdf - PDF reference guide
#  manhtmlpages - HTML5 separate per-feature reference pages

default:
	make SPECBASE=OpenVX_Specification html $(TARGET) SPECREVISION='1.3' SPECSUFFIX=1_3

all:
	make
	make SPECBASE=vx_khr_import_kernel html $(TARGET) SPECREVISION='1.3 (provisional)' SPECSUFFIX=1_3
	make SPECBASE=vx_khr_opencl_interop html $(TARGET) SPECREVISION='1.0 (provisional)' SPECSUFFIX=1_0
	make SPECBASE=vx_khr_class html $(TARGET) SPECREVISION='1.2.1 (provisional)' SPECSUFFIX=1_2_1
	make SPECBASE=vx_khr_export_and_import html $(TARGET) SPECREVISION='1.1.1' SPECSUFFIX=1_1_1
	make SPECBASE=vx_khr_icd html $(TARGET) SPECREVISION='1.0.1 (provisional)' SPECSUFFIX=1_0_1
	make SPECBASE=vx_khr_nn html $(TARGET) SPECREVISION='1.3' SPECSUFFIX=1_3
	make SPECBASE=vx_khr_pipelining html $(TARGET) SPECREVISION='1.1.0' SPECSUFFIX=1_1_0
	make SPECBASE=vx_khr_s16 html $(TARGET) SPECREVISION='1.1.1 (provisional)' SPECSUFFIX=1_1_1
	make SPECBASE=vx_khr_tiling html $(TARGET) SPECREVISION='1.0.1 (provisional)' SPECSUFFIX=1_0_1
	make SPECBASE=vx_khr_xml html $(TARGET) SPECREVISION='1.1 (provisional)' SPECSUFFIX=1_1
	make SPECBASE=vx_khr_xml_ug html $(TARGET) SPECREVISION='1.1 (provisional)' SPECSUFFIX=1_1
	make SPECBASE=vx_khr_feature_sets html $(TARGET) SPECREVISION='1.1' SPECSUFFIX=1_1
	make SPECBASE=vx_khr_buffer_aliasing html $(TARGET) SPECREVISION='1.0 (provisional)' SPECSUFFIX=1_0
	make SPECBASE=vx_khr_user_data_object html $(TARGET) SPECREVISION='1.0 (provisional)' SPECSUFFIX=1_0

## alldocs: allspecs allman
alldocs: allspecs

allspecs: html pdf

allman: manhtml manpdf manhtmlpages

# Note that the := assignments below are immediate, not deferred, and
# are therefore order-dependent in the Makefile

QUIET	 ?= @
PYTHON	 ?= python3
ASCIIDOC ?= asciidoctor
RM	 = rm -f
RMRF	 = rm -rf
MKDIR	 = mkdir -p
CP	 = cp
ECHO	 = echo
GS_EXISTS := $(shell command -v gs 2> /dev/null)

# Target directories for output files
# HTMLDIR - 'html' target
# PDFDIR - 'pdf' target
OUTDIR	  := out/$(SPECBASE)
HTMLDIR   := $(OUTDIR)/html
PDFDIR	  := $(OUTDIR)

# PDF Equations are written to SVGs, this dictates the location to store those files (temporary)
PDFMATHDIR:=$(OUTDIR)/equations_temp

# Set VERBOSE to -v to see what asciidoc is doing.
VERBOSE =

# asciidoc attributes to set.
# NOTEOPTS   sets options controlling which NOTEs are generated
# ATTRIBOPTS sets the api revision and enables MathJax generation
# VERSIONATTRIBS sets attributes for enabled API versions (set above
#	     based on $(VERSIONS))
# EXTATTRIBS sets attributes for enabled extensions (set above based on
#	     $(EXTENSIONS))
# ADOCOPTS   options for asciidoc->HTML5 output
NOTEOPTS     = -a editing-notes -a implementation-guide

SPECREVISION = 1.2.1
SPECSUFFIX   = 1_2_1
SPECDIRNAME = $(SPECBASE)_$(SPECSUFFIX)

# Spell out RFC2822 format as not all date commands support -R
SPECDATE     = $(shell echo `date -u "+%a, %d %b %Y %T %z"`)

# Generate Asciidoc attributes for spec remark
GITHEAD := ../../.git/logs/HEAD
ifeq ($(wildcard $(GITHEAD)),)
# If GITHEAD does not exist, don't include branch info.
SPECREMARK = Git branch information not available
else
# Could use `git log -1 --format="%cd"` to get branch commit date
# This used to be a dependency in the spec html/pdf targets,
# but that's likely to lead to merge conflicts. Just regenerate
# when pushing a new spec for review to the sandbox.
# The dependency on HEAD is per the suggestion in
# http://neugierig.org/software/blog/2014/11/binary-revisions.html
SPECREMARK = from git branch: $(shell echo `git symbolic-ref --short HEAD`) \
	     commit: $(shell echo `git log -1 --format="%H"`)
endif

ATTRIBOPTS   = -a revnumber="$(SPECREVISION)" \
	       -a revdate="$(SPECDATE)" \
	       -a revremark="$(SPECREMARK)" \
	       -a apititle="$(APITITLE)" \
	       -a stem=latexmath \
	       $(VERSIONATTRIBS) \
	       $(EXTATTRIBS)

ADOCEXTS     = -r asciidoctor-diagram -r $(CURDIR)/config/vulkan-macros.rb -r $(CURDIR)/config/tilde_open_block.rb
ADOCOPTS     = -d book $(ATTRIBOPTS) $(NOTEOPTS) $(VERBOSE) $(ADOCEXTS)

ADOCHTMLEXTS = -r $(CURDIR)/config/katex_replace.rb

# ADOCHTMLOPTS relies on the relative KATEXDIR path being set correctly
# for each target using the variable.
KATEXDIR     = html/katex
ADOCHTMLOPTS = $(ADOCHTMLEXTS) -a stylesheet=khronos.css \
	       -a stylesdir=config -a katexpath=$(KATEXDIR)

ADOCPDFEXTS  = -r asciidoctor-pdf -r asciidoctor-mathematical -r $(CURDIR)/config/asciidoctor-mathematical-ext.rb
ADOCPDFOPTS  = $(ADOCPDFEXTS) -a mathematical-format=svg \
	       -a pdf-stylesdir=config/themes -a pdf-style=pdf \
	       -a imagesoutdir=$(PDFMATHDIR)

.PHONY: directories

# Images used by the spec. These are included in generated HTML now.
IMAGEPATH := images
SVGFILES  := $(wildcard $(IMAGEPATH)/*.svg)

# Top-level spec source file
SPECBASE = OpenVX_Specification
SPECSRC  = $(SPECBASE).adoc

# Files making up sections of the API spec. The wildcard expression
# should work in extension branches to pull in those files as well.
SPECFILES = $(wildcard [A-Za-z]*.adoc)
GENINCLUDE = $(wildcard api/*/[A-Za-z]*.txt)

# Generated dependencies of the spec
GENDEPENDS = api/timeMarker
# All non-format-specific dependencies
COMMONDOCS = $(SPECFILES) $(GENINCLUDE) $(GENDEPENDS)

# Install katex in $(OUTDIR)/katex for reference by all HTML targets
# README.md is a proxy for all the katex files that need to be installed
katexinst: KATEXDIR = katex
katexinst: $(HTMLDIR)/$(KATEXDIR)/README.md

$(HTMLDIR)/$(KATEXDIR)/README.md: katex/README.md
	$(QUIET)$(MKDIR) $(OUTDIR)
	$(QUIET)$(MKDIR) $(HTMLDIR)
	$(QUIET)$(RMRF)  $(HTMLDIR)/$(KATEXDIR)
	$(QUIET)$(CP) -rf katex $(HTMLDIR)

# Spec targets
# There is some complexity to try and avoid short virtual targets like 'html'
# causing specs to *always* be regenerated.
html: $(HTMLDIR)/index.html $(SPECSRC) $(COMMONDOCS)
	$(QUIET)(cd $(OUTDIR); rm -rf $(SPECDIRNAME); mv html $(SPECDIRNAME); mv $(SPECDIRNAME)/index.html $(SPECDIRNAME)/$(SPECDIRNAME).html; zip -q -r $(SPECDIRNAME).zip $(SPECDIRNAME))

$(HTMLDIR)/index.html: KATEXDIR = katex
$(HTMLDIR)/index.html: $(SPECSRC) $(COMMONDOCS) katexinst
	$(QUIET)$(ASCIIDOC) -b html5 $(ADOCOPTS) $(ADOCHTMLOPTS) -o $@ $(SPECSRC)

diff_html: $(HTMLDIR)/diff.html $(SPECSRC) $(COMMONDOCS)

$(HTMLDIR)/diff.html: KATEXDIR = katex
$(HTMLDIR)/diff.html: $(SPECSRC) $(COMMONDOCS) katexinst
	$(QUIET)$(ASCIIDOC) -b html5 $(ADOCOPTS) $(ADOCHTMLOPTS) -a diff_extensions="$(DIFFEXTENSIONS)" -r $(CURDIR)/config/extension-highlighter.rb --trace -o $@ $(SPECSRC)

pdf: $(PDFDIR)/$(SPECDIRNAME).pdf $(SPECSRC) $(COMMONDOCS)

$(PDFDIR)/$(SPECDIRNAME).pdf: $(SPECSRC) $(COMMONDOCS)
	$(QUIET)$(MKDIR) $(PDFDIR)
	$(QUIET)$(MKDIR) $(PDFMATHDIR)
	$(QUIET)$(ASCIIDOC) -b pdf $(ADOCOPTS) $(ADOCPDFOPTS) -o $@ $(SPECSRC)
ifndef GS_EXISTS
	$(QUIET) echo "Warning: Ghostscript not installed, skipping pdf optimization"
else
	$(QUIET)$(CURDIR)/config/optimize-pdf $@
	$(QUIET)rm $@
	$(QUIET)mv $(PDFDIR)/$(SPECDIRNAME)-optimized.pdf $@
endif

# Reflow text in spec sources
REFLOW = reflow.py
REFLOWOPTS = -overwrite

reflow:
	$(QUIET) echo "Warning: please verify the spec outputs build without changes!"
	$(PYTHON) $(REFLOW) $(REFLOWOPTS) $(SPECSRC) $(SPECFILES) $(STYLESRC) $(STYLEFILES)

# Clean generated and output files

clean: clean_html clean_pdf clean_man clean_generated

clean_html:
	$(QUIET)$(RMRF) $(HTMLDIR) $(OUTDIR)/katex
	$(QUIET)$(RM) $(OUTDIR)/apispec.html

clean_pdf:
	$(QUIET)$(RMRF) $(PDFDIR)

clean_man:
	$(QUIET)$(RMRF) $(MANHTMLDIR)

clean_generated:
	$(QUIET)$(RMRF) api vxapi.py
	$(QUIET)$(RM) config/extDependency.stamp config/extDependency.pyc config/extDependency.sh config/extDependency.py
	$(QUIET)$(RM) man/apispec.txt $(LOGFILE) man/[Vv][Kk]*.txt man/PFN*.txt
	$(QUIET)$(RMRF) $(PDFMATHDIR)
	$(QUIET)$(RMRF) xml/__pycache__/
	$(QUIET)$(RMRF) out

# No longer done, SVGs are checked in
#	 (cd $(IMAGEPATH) && $(MAKE) clean)

# Ref page targets for individual pages
MANDIR	    := man
MANSECTION  := 3

# These lists should be autogenerated

# Ref page sources, split up by core API (CORE), KHR extensions (KHR), and
# other extensions (VEN). This is a hacky approach to ref page generation
# now that the single-branch model is in place, and there are outstanding
# issues to resolve it. For the moment, we always just build the core
# ref pages.

KHRSOURCES   = $(wildcard $(MANDIR)/*KHR.txt)
MACROSOURCES = $(wildcard $(MANDIR)/VX_*[A-Z][A-Z].txt)
VENSOURCES   = $(filter-out $(KHRSOURCES) $(MACROSOURCES),$(wildcard $(MANDIR)/*[A-Z][A-Z].txt))
CORESOURCES  = $(filter-out $(KHRSOURCES) $(VENSOURCES),$(wildcard $(MANDIR)/[Vv][Kk]*.txt $(MANDIR)/PFN*.txt))
MANSOURCES   = $(CORESOURCES)
MANCOPYRIGHT = $(MANDIR)/copyright-ccby.txt $(MANDIR)/footer.txt

# Automatic generation of ref pages. Needs to have a proper dependency
# causing the man page sources to be generated by running genRef (once),
# but adding $(MANSOURCES) to the targets causes genRef to run
# once/target.
#
# @@ Needs to pass in $(EXTOPTIONS) and use that to determine which
# pages to generate. As it stands, all the extension ref pages are
# also generated, though they are not usable at present.

LOGFILE = man/logfile
man/apispec.txt: $(SPECFILES) genRef.py reflib.py vxapi.py
	$(PYTHON) genRef.py -log $(LOGFILE) $(SPECFILES)

# These dependencies don't take into account include directives

# These targets are HTML5 ref pages

MANHTMLDIR  := $(OUTDIR)/man/html
MANHTML     = $(MANSOURCES:$(MANDIR)/%.txt=$(MANHTMLDIR)/%.html)

manhtmlpages: man/apispec.txt $(MANHTML)

$(MANHTMLDIR)/%.html: KATEXDIR = ../../katex
$(MANHTMLDIR)/%.html: $(MANDIR)/%.txt $(MANCOPYRIGHT) $(GENINCLUDE) $(GENDEPENDS) katexinst
	$(QUIET)$(MKDIR) $(MANHTMLDIR)
	$(QUIET)$(ASCIIDOC) -b html5 -a cross-file-links -a html_spec_relative='../../html/index.html' $(ADOCOPTS) $(ADOCHTMLOPTS) -d manpage -o $@ $<

# These targets are HTML5 and PDF single-file versions of the ref pages

manpdf: $(OUTDIR)/apispec.pdf

$(OUTDIR)/apispec.pdf: $(SPECVERSION) man/apispec.txt $(MANSOURCES) $(MANCOPYRIGHT) $(SVGFILES) $(GENINCLUDE) $(GENDEPENDS)
	$(QUIET)$(MKDIR) $(OUTDIR)
	$(QUIET)$(MKDIR) $(PDFMATHDIR)
	$(QUIET)$(ASCIIDOC) -b pdf -a html_spec_relative='html/index.html' $(ADOCOPTS) $(ADOCPDFOPTS) -o $@ man/apispec.txt
ifndef GS_EXISTS
	$(QUIET) echo "Warning: Ghostscript not installed, skipping pdf optimization"
else
	$(QUIET)$(CURDIR)/config/optimize-pdf $@
	$(QUIET)rm $@
	$(QUIET)mv $(OUTDIR)/apispec-optimized.pdf $@
endif

manhtml: $(OUTDIR)/apispec.html

$(OUTDIR)/apispec.html: KATEXDIR = katex
$(OUTDIR)/apispec.html: $(SPECVERSION) man/apispec.txt $(MANSOURCES) $(MANCOPYRIGHT) $(SVGFILES) $(GENINCLUDE) $(GENDEPENDS) katexinst
	$(QUIET)$(MKDIR) $(OUTDIR)
	$(QUIET)$(ASCIIDOC) -b html5 -a html_spec_relative='html/index.html' $(ADOCOPTS) $(ADOCHTMLOPTS) -o $@ man/apispec.txt

# Targets generated from the XML and registry processing scripts
#   vxapi.py - Python encoding of the registry
#   api/timeMarker - proxy for 'apiincludes' - API include files under api/*/*.txt
#
# $(VERSIONOPTIONS) specifies the core API versions which are included
# in these targets, and is set above based on $(VERSIONS)
#
# $(EXTOPTIONS) specifies the extensions which are included in these
# targets, and is set above based on $(EXTENSIONS).

REGISTRY = xml
VXXML	 = $(REGISTRY)/vx.xml
GENVX	 = $(REGISTRY)/genvx.py
GENVXOPTS= $(VERSIONOPTIONS) $(EXTOPTIONS) -registry $(VXXML)

vxapi.py: $(VXXML) $(GENVX)
	$(PYTHON) $(GENVX) $(GENVXOPTS) -o . vxapi.py

apiinc: api/timeMarker

api/timeMarker: $(VXXML) $(GENVX)
	$(QUIET)$(MKDIR) api
	$(QUIET)$(PYTHON) $(GENVX) $(GENVXOPTS) -o api apiinc

# Debugging aid - generate all files from registry XML
# This leaves out config/extDependency.sh intentionally as it only
# needs to be updated when the extension dependencies in vx.xml change.

generated: vxapi.py api/timeMarker
	cd $(IMAGEPATH) && $(MAKE)

# Extension dependencies derived from vx.xml.
# Both Bash and Python versions are generated

config/extDependency.sh: config/extDependency.stamp
config/extDependency.py: config/extDependency.stamp

DEPSCRIPT = $(REGISTRY)/extDependency.py
config/extDependency.stamp: $(VXXML) $(DEPSCRIPT)
	$(QUIET)$(PYTHON) $(DEPSCRIPT) -registry $(VXXML) \
	    -outscript config/extDependency.sh \
	    -outpy config/extDependency.py
	$(QUIET)touch $@
