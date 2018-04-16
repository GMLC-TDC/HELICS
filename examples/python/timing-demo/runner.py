import libtmux


def main():
    session = libtmux.Server().list_sessions()[0]
    session.attached_window.split_window(vertical=False)

    for i, p in enumerate(session.attached_window.children):
        p.clear()
        p.send_keys("python ./timing-federate{}.py".format(i+1))
        p.enter()

if __name__ == "__main__":
    main()

