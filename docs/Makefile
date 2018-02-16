# Minimal makefile for Sphinx documentation
#

# You can set these variables from the command line.
SPHINXOPTS    =
SPHINXBUILD   = python -msphinx
SPHINXPROJ    = HELICS-src
SOURCEDIR     = .
BUILDDIR      = _build
BRANCH := $(shell git rev-parse --abbrev-ref HEAD)

# Put it first so that "make" without argument is like "make help".
help:
	@$(SPHINXBUILD) -M help "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)

.PHONY: help Makefile

# Catch-all target: route all unknown targets to Sphinx using the new
# "make mode" option.  $(O) is meant as a shortcut for $(SPHINXOPTS).
%: Makefile
	@$(SPHINXBUILD) -M $@ "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)

doxygen:
	cd ../; mkdir -p build-doxygen; cd build-doxygen; cmake -DCMAKE_INSTALL_PREFIX=./install/helics_install -DHELICS_GENERATE_DOXYGEN_DOC=ON ../; make doc;

sphinxdoxygen: doxygen
	-rm -rf ./_build/html/doxygen
	cp -r ../build-doxygen/docs/html/ ./_build/html/doxygen/

github: html sphinxdoxygen
	-git branch -D gh-pages
	-git push origin --delete gh-pages
	ghp-import -n -b gh-pages -m "Update documentation" ./_build/html
	git checkout gh-pages
	git push --set-upstream origin gh-pages
	git checkout ${BRANCH}

all: doxygen github
