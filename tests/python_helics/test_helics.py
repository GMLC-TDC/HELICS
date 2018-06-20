import helics as h
import pytest as pt

def test_import():
    import helics
    print("Imported module {}".format(helics))

def test_version():
    print(h.helicsGetVersion())

