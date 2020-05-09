# Generating Documentation

The documentation requires Pandoc to convert from Markdown to RST.

You will need the following Python packages.

```bash
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

If you don't have Pandoc, you can install it using `conda`.

```bash
conda install pandoc
```

If you are unable to install `pandoc`, you may be able to generate some of the documentation if you install the following.

```bash
pip install recommonmark
```
