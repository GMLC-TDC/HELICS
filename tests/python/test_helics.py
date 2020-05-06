def test_import():
    import helics

    print("Imported module {}".format(helics))


def test_version():
    import helics as h

    print(h.helicsGetVersion())
