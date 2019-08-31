# Minimal makefile for Sphinx documentation
#

# You can set these variables from the command line.
SPHINXOPTS    =
SPHINXBUILD   = python -msphinx
SPHINXPROJ    = HELICS
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

rtddoxygen:
	cd ../; mkdir -p build-doxygen; rm -rf build-doxygen; mkdir -p build-doxygen; python ./scripts/render-doxyfile.py; doxygen Doxyfile; mkdir -p rtd-doxygen; cp -r ./build-doxygen/docs/html ./rtd-doxygen/doxygen;

doxygen:
	cd ../; mkdir -p build-doxygen; rm -rf build-doxygen; mkdir -p build-doxygen; cd build-doxygen; cmake -DCMAKE_INSTALL_PREFIX=./install/helics_install -DGENERATE_DOXYGEN_DOC=ON ../; make doc;

sphinxdoxygen: doxygen html
	-rm -rf ./_build/html/doxygen
	cp -r ../build-doxygen/docs/html/ ./_build/html/doxygen/

v1.3.1:
	git checkout kd/fix-v1.3.1-docstrings
	-rm -rf _build/html
	make sphinxdoxygen
	-rm -rf _build/v1.3.1
	mv ./_build/html/ ./_build/v1.3.1

v2.0.0:
	git checkout kd/documentation-update
	-rm -rf _build/html
	make sphinxdoxygen
	-rm -rf _build/v2.0.0
	mv ./_build/html/ ./_build/v2.0.0

alldocs:
	echo "Ensure that you've run the targets v1.3.1 and v2.0.0 manually."
	mkdir -p _build
	rm -rf _build/index.html
	echo "<link rel='canonical' href='https://helics.readthedocs.io/en/latest'/>" > _build/404.html
	echo "<script>" >> _build/404.html
	echo "if (window.location.href.indexOf('https://gmlc-tdc.github.io/HELICS') === 0) {" >> _build/404.html
	echo "url_link = window.location.href.replace('gmlc-tdc.github.io/HELICS', 'helics.readthedocs.io/en/latest')" >> _build/404.html
	echo "window.location.href = url_link;" >> _build/404.html
	echo "}" >> _build/404.html
	echo "</script>" >> _build/404.html

	echo "<link rel='canonical' href='https://helics.readthedocs.io/en/latest'/>" > _build/index.html
	echo "<script>" >> _build/index.html
	echo "if (window.location.href.indexOf('https://gmlc-tdc.github.io/HELICS') === 0) {" >> _build/index.html
	echo "url_link = window.location.href.replace('gmlc-tdc.github.io/HELICS', 'helics.readthedocs.io/en/latest')" >> _build/index.html
	echo "window.location.href = url_link;" >> _build/index.html
	echo "}" >> _build/index.html
	echo "</script>" >> _build/index.html
	-rm -rf _build/html
	mkdir -p ./_build/html
	cp -r _build/index.html _build/html/index.html
	cp -r _build/404.html _build/html/404.html

github: alldocs
	echo "Ensure you have built the docs correctly at the _build/html folder"
	-git branch -D gh-pages
	-git push origin --delete gh-pages
	ghp-import -n -b gh-pages -m "Update documentation" ./_build/html
	git checkout gh-pages
	git push --set-upstream origin gh-pages
	git checkout ${BRANCH}

all: doxygen github
