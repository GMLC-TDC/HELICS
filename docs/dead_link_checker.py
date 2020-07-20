#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on <day of week> <mon> <date> <HH:MM:ss> <year>

Description of code

@author: hard312 (Trevor Hardy)
"""

import argparse
import logging
import pprint
import os
import sys
from scrapy.spiders import CrawlSpider, Rule
from scrapy.linkextractors import LinkExtractor
from scrapy.selector import Selector
from scrapy.item import Item, Field

# Setting up logging
logger = logging.getLogger(__name__)

# Setting up pretty printing, mostly for debugging.
pp = pprint.PrettyPrinter(indent=4, )


def _auto_run(args):
    """This function executes when the script is called as a stand-alone
    executable.

    A more complete description of this code can be found in the
    docstring at the beginning of this file.

    Args:
        '-a' or '--auto_run_dir' - Path of the auto_run folder
        that contains examples of the files used in this script. Used
        for development purposes as well as models/examples/documentation
        of the format and contents expected in said files

    Returns:
        (none)
    """


if __name__ == '__main__':
    # TDH: This slightly complex mess allows lower importance messages
    # to be sent to the log file and ERROR messages to additionally
    # be sent to the console as well. Thus, when bad things happen
    # the user will get an error message in both places which,
    # hopefully, will aid in trouble-shooting.
    fileHandle = logging.FileHandler("dsot_solar.log", mode='w')
    fileHandle.setLevel(logging.DEBUG)
    streamHandle = logging.StreamHandler(sys.stdout)
    streamHandle.setLevel(logging.ERROR)
    logging.basicConfig(level=logging.INFO,
                        handlers=[fileHandle, streamHandle])

    parser = argparse.ArgumentParser(description='xxxxxxxx')
    # TDH: Have to do a little bit of work to generate a good default
    # path for the auto_run folder (where the development test data is
    # held.
    script_path = os.path.dirname(os.path.realpath(__file__))
    auto_run_dir = os.path.join(script_path, 'auto_run')
    parser.add_argument('-a',
                        '--auto_run_dir',
                        nargs='?',
                        default=auto_run_dir)
    args = parser.parse_args()
    _auto_run(args)