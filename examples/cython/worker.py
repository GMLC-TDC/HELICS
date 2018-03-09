# Copyright (C) 2017, Battelle Memorial Institute
# All rights reserved.
# This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

try:
    import click
except ImportError:
    print("Unable to import click, using mock click instead. Try 'pip install click' for better output on terminal")

    class click(object):

        @classmethod
        def style(string, **kwargs):
            return string

        def echo(string):
            print(string)


from helics import PyValueFederate, PyFederateInfo

import time


def worker(name, fType):

    fi = PyFederateInfo(2)

    fi.setName(name)

    color = 'red'
    s = click.style("Creating a Python Value Federate object", fg=color, bold=True)
    click.echo(s)

    if fType == 'pub':
        vf = PyValueFederate(fi=fi, publications=['topic1'])
    else:
        vf = PyValueFederate(fi=fi, subscriptions=['topic1'])

    color = 'yellow'
    s = click.style('Current state = {}'.format(vf.getState()), fg=color, bold=True)
    click.echo(s)

    color = 'blue'
    s = click.style("Waiting one second", fg=color, bold=True)
    click.echo(s)
    time.sleep(1)

    vf.enterExecutionState()
    color = 'yellow'
    s = click.style('Current state = {}'.format(vf.getState()), fg=color, bold=True)
    click.echo(s)

    color = 'blue'
    s = click.style("Waiting one second", fg=color, bold=True)
    click.echo(s)
    time.sleep(1)

    if fType == 'pub':
        value = 10
        color = 'yellow'
        s = click.style("Sent value = {}".format(value), fg=color, bold=True)
        click.echo(s)

        vf.send(str(value), 'topic1')

    if fType == 'sub':
        value = vf.recv('topic1')
        color = 'yellow'
        s = click.style("Received value = {}".format(value), fg=color, bold=True)
        click.echo(s)

    t = vf.requestTime(1.0)
    print('Current Time = {}'.format(t))

    color = 'blue'
    s = click.style("Waiting one second", fg=color, bold=True)
    click.echo(s)
    time.sleep(1)

    if fType == 'pub':
        value = 10
        color = 'yellow'
        s = click.style("Sent value = {}".format(value), fg=color, bold=True)
        click.echo(s)

        vf.send(str(value), 'topic1')

    if fType == 'sub':
        value = vf.recv('topic1')
        color = 'yellow'
        s = click.style("Received value = {}".format(value), fg=color, bold=True)
        click.echo(s)

    t = vf.requestTime(2.0)
    print('Current Time = {}'.format(t))

    color = 'blue'
    s = click.style("Waiting one second", fg=color, bold=True)
    click.echo(s)
    time.sleep(1)

    if fType == 'pub':
        value = 10
        color = 'yellow'
        s = click.style("Sent value = {}".format(value), fg=color, bold=True)
        click.echo(s)

        vf.send(str(value), 'topic1')

    if fType == 'sub':
        value = vf.recv('topic1')
        color = 'yellow'
        s = click.style("Received value = {}".format(value), fg=color, bold=True)
        click.echo(s)

    t = vf.requestTime(3.0)
    print('Current Time = {}'.format(t))

    color = 'blue'
    s = click.style("Waiting one second", fg=color, bold=True)
    click.echo(s)
    time.sleep(1)
    # t = vf.requestTime(1.0)
    # print('Current Time = {}'.format(t))

    s = click.style("Finishing simulation", fg=color, bold=True)
    click.echo(s)

    vf.finalize()

