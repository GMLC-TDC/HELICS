#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# HELICS documentation build configuration file, created by
# sphinx-quickstart on Wed Dec 13 12:07:08 2017.
#
# This file is execfile()d with the current directory set to its
# containing dir.
#
# Note that not all possible configuration values are present in this
# autogenerated file.
#
# All configuration values have a default; values that are commented out
# serve to show the default.

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
# import os
# import sys
# sys.path.insert(0, os.path.abspath('.'))

# -- General configuration ------------------------------------------------

# If your documentation needs a minimal Sphinx version, state it here.
#
# needs_sphinx = '1.0'

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
import os
import sphinx_rtd_theme
import shutil
import subprocess
import requests
import tempfile
import zipfile

current_directory = os.path.dirname(os.path.realpath(__file__))


def download_helics_images():
    print("Downloading doc images from GMLC-TDC/helics_doc_resources")
    with tempfile.TemporaryDirectory() as tmpdirname:
        url = "https://github.com/GMLC-TDC/helics_doc_resources/archive/refs/heads/main.zip"
        target_path = os.path.join(tmpdirname, "helics_docs_resources.zip")
        response = requests.get(url, stream=True)
        if response.status_code == 200:
            with open(target_path, "wb") as f:
                f.write(response.raw.read())
        with zipfile.ZipFile(target_path, "r") as zip_ref:
            zip_ref.extractall(tmpdirname)
        if os.path.exists("img"):
            shutil.rmtree("img")
        shutil.move(os.path.join(tmpdirname, "helics_doc_resources-main/user_guide"), "img")
    print("Doc images downloaded")


def which(program):
    def is_exe(fpath):
        return os.path.exists(fpath) and os.access(fpath, os.X_OK) and os.path.isfile(fpath)

    def ext_candidates(fpath):
        yield fpath
        for ext in os.environ.get("PATHEXT", "").split(os.pathsep):
            yield fpath + ext

    # fpath, fname = os.path.split(program)
    fpath = os.path.dirname(program)
    if fpath:
        if is_exe(program):
            return program
    else:
        for path in os.environ["PATH"].split(os.pathsep):
            exe_file = os.path.join(path, program)
            for candidate in ext_candidates(exe_file):
                if is_exe(candidate):
                    return candidate

    return None


read_the_docs_build = os.environ.get("READTHEDOCS", None) == "True"

# For RTD builds, only download the images once; for developers, always get the images
if not read_the_docs_build or not os.path.exists("img"):
    download_helics_images()

# For RTD builds run doxygen once for all output formats
if read_the_docs_build:
    dir_name = os.path.realpath(os.path.dirname(__file__))
    checkout_dir = os.path.realpath(os.path.join(dir_name, os.pardir))
    doxygen_build_dir = os.path.realpath(os.path.join(checkout_dir, "build-doxygen"))
    if not os.path.isdir(doxygen_build_dir):
        os.makedirs(os.path.join(doxygen_build_dir, "docs", "html"))
        subprocess.call(
            "cd {dir_name} && python ./scripts/render-doxyfile.py && doxygen Doxyfile;".format(
                dir_name=checkout_dir
            ),
            shell=True,
        )
    html_extra_path = [os.path.abspath(os.path.join(doxygen_build_dir, "docs", "html"))]

extensions = [
    "myst_parser",
    "sphinx.ext.autodoc",
    "sphinx.ext.doctest",
    "sphinx.ext.intersphinx",
    "sphinx.ext.coverage",
    "sphinx.ext.mathjax",
    "sphinx.ext.viewcode",
    "sphinx.ext.githubpages",
    "sphinx.ext.napoleon",
    "sphinxcontrib.rsvgconverter",
    "IPython.sphinxext.ipython_console_highlighting",
    "breathe",
    "sphinxcontrib.redoc",
    "sphinxcontrib.mermaid",
]

myst_enable_extensions = [
    "amsmath",
    "dollarmath",
]
myst_dmath_double_inline = True

myst_heading_anchors = 5

breathe_projects = {
    "helics": os.path.abspath(os.path.join(current_directory, "./../build-doxygen/docs/xml")),
}

breathe_default_project = "helics"

# Add any paths that contain templates here, relative to this directory.
templates_path = []


# The master toctree document.
master_doc = "index"

# General information about the project.
project = "HELICS"
copyright = "2017-2021 Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC. See the top-level NOTICE for additional details. All rights reserved. SPDX-License-Identifier: BSD-3-Clause. Documentation source in https://github.com/GMLC-TDC/HELICS"
author = "Philip Top, Trevor Hardy, Ryan Mast, Dheepak Krishnamurthy, Andrew Fisher, Bryan Palmintier, Jason Fuller"

# The version info for the project you're documenting, acts as replacement for
# |version| and |release|, also used in various other places throughout the
# built documents.
#
# The short X.Y version.
version = ""
# The full version, including alpha/beta/rc tags.
release = ""

# The language for content autogenerated by Sphinx. Refer to documentation
# for a list of supported languages.
#
# This is also used if you do content translation via gettext catalogs.
# Usually you set "language" from the command line for these cases.
language = None

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This patterns also effect to html_static_path and html_extra_path
exclude_patterns = ["_build", "Thumbs.db", ".DS_Store"]

# The name of the Pygments (syntax highlighting) style to use.
pygments_style = "sphinx"

# If true, `todo` and `todoList` produce output, else they produce nothing.
todo_include_todos = False

# redoc config
redoc = [
    {
        "name": "HELICS REST API",
        "page": "references/api-reference/rest_queries_api",
        "spec-root": "swagger/",
        "spec": "reference/queries.yaml",
        "embed": False,
    }
]

# mermaid config
# default js file priority is 500; require.js used by nbsphinx causes issues
# when it gets loaded first
mermaid_js_priority = 499

# -- Options for HTML output ----------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
html_theme = "sphinx_rtd_theme"
html_theme_path = [sphinx_rtd_theme.get_html_theme_path()]

# Theme options are theme-specific and customize the look and feel of a theme
# further.  For a list of options available for each theme, see the
# documentation.
#
html_theme_options = {"navigation_depth": 4, "collapse_navigation": False}

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = ["_static"]

html_logo = "./logos/helics-logo-stacked-onWhite.png"
html_favicon = html_logo

# Custom sidebar templates, must be a dictionary that maps document names
# to template names.
#
# This is required for the alabaster theme
# refs: http://alabaster.readthedocs.io/en/latest/installation.html#sidebars
html_sidebars = {
    "**": [
        "about.html",
        "navigation.html",
        "relations.html",  # needs 'show_related': True theme option to display
        "searchbox.html",
        "donate.html",
    ]
}


# Adding last updated timestamp at the bottom of every page
html_last_updated_fmt = ""  # Empty string = '%b %d, %Y'


# -- Options for HTMLHelp output ------------------------------------------

# Output file base name for HTML help builder.
htmlhelp_basename = "HELICSdoc"

# -- Options for LaTeX output ---------------------------------------------

latex_elements = {
    # The paper size ('letterpaper' or 'a4paper').
    #
    # 'papersize': 'letterpaper',
    # The font size ('10pt', '11pt' or '12pt').
    #
    # 'pointsize': '10pt',
    # Additional stuff for the LaTeX preamble.
    #
    # 'preamble': '',
    # Latex figure (float) alignment
    #
    # 'figure_align': 'htbp',
}

# Grouping the document tree into LaTeX files. List of tuples
# (source start file, target name, title,
#  author, documentclass [howto, manual, or own class]).
latex_documents = [
    (
        master_doc,
        "HELICS.tex",
        "HELICS Documentation",
        "Philip Top, Trevor Hardy, Ryan Mast, Dheepak Krishnamurthy, Andrew Fisher, Bryan Palmintier, Jason Fuller",
        "manual",
    ),
]

# -- Options for manual page output ---------------------------------------

# One entry per manual page. List of tuples
# (source start file, name, description, authors, manual section).
man_pages = [(master_doc, "helics", "HELICS Documentation", [author], 1)]

# -- Options for Texinfo output -------------------------------------------

# Grouping the document tree into Texinfo files. List of tuples
# (source start file, target name, title, author,
#  dir menu entry, description, category)
texinfo_documents = [
    (
        master_doc,
        "HELICS",
        "HELICS Documentation",
        author,
        "HELICS",
        "Hierarchical Engine for Large-scale Infrastructure Co-Simulation",
        "Miscellaneous",
    ),
]


def setup(app):
    app.add_css_file("css/custom.css")  # may also be an URL
