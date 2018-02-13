Generating Documentation
========================

You will need the following Python packages.

``` {.sourceCode .bash}
pip install sphinx
pip install ghp-import
pip install breathe
pip install sphinx_rtd_theme
pip install nbsphinx
pip install sphinxcontrib-pandoc-markdown
```

You will also need doxygen.

You can then type `make doxygen html` to create the documentation
locally.
