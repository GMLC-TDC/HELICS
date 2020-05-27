# -*- coding: utf-8 -*-
"""
Created on Mon May 11 09:59:05 2020

@author: barn553
"""

import helics as h
import sys
import os
# import numpy as np
import argparse
import logging
import json
from threading import Thread, Barrier, Timer
import cpuinfo
import multiprocessing
import platform
# import psutil
import pytest_benchmark as bench
# Setting up logger
logger = logging.getLogger(__name__)

class EchoHub_c:
    
    def __init__(self):
        print('Created the EchoHub class')
        # finalTime = h.helics_data_type_time
        self.finalTime = 0.1
        self.vFed = None
        self.pubs = []
        self.subs = []
        self.cnt_ = 10
        self.initialized = False
        self.readyToRun = False
    
    def callOnReady(self, parties):
        print('####### created the barrier object')
        brr = Barrier(parties)
        return brr
    
    def create_value_federate(self, coreName):
        name = 'echohub Test--AA'
        print('####### creating the Federate info...')
        fi = h.helicsCreateFederateInfo()
        print('Setting the core name')
        h.helicsFederateInfoSetCoreName(fi, coreName)
        print('Creating the value federate...')
        global vFed
        vFed = h.helicsCreateValueFederate(name, fi)
        return vFed
    
    def initialize(self, vFed, cnt):
        print('####### initialized the EchoHub test')
        self.vFed = vFed
        print('federate valid:', h.helicsFederateIsValid(self.vFed))
        self.cnt_ = cnt
        i = 0
        while i < self.cnt_:
            leafname = 'leafrx_{}'.format(i)
            print('appending the publications')
            self.pubs.append(h.helicsFederateRegisterGlobalPublication(
                self.vFed, leafname, h.helics_data_type_string, 
                ""))
            leafname2 = 'leafsend_{}'.format(i)
            print('appending the subscriptions')
            self.subs.append(h.helicsFederateRegisterSubscription(
                self.vFed, leafname2, ""))
            i += 1
        self.initialized = True
        
    def makeReady(self, vFed):
        self.vFed = vFed
        if self.initialized is False:
            print('must initialize first')
        print('federate valid:', h.helicsFederateIsValid(self.vFed))
        print('####### starting the HELICS execution')
        print('hub - before enter execution async')
        h.helicsFederateEnterExecutingModeAsync(self.vFed)
        h.helicsFederateEnterExecutingMode(self.vFed)
        h.helicsFederateEnterExecutingModeComplete(self.vFed)
        print('hub - after enter execution complete')
        print('federate valid:', h.helicsFederateIsValid(self.vFed))
        self.readyToRun = True
    
    def mainLoop(self, vFed):
        print('###### starting the main loop')
        self.vFed = vFed
        print('federate valid:', h.helicsFederateIsValid(self.vFed))
        buffer = chr(256)
        cTime = h.helics_data_type_time_zero()
        while cTime <= self.finalTime:
            i = 0
            for c in range(0, self.cnt_):
                print('checking to see if HELICS infor is updated')
                if h.helicsInputIsUpdated(self.subs[i]):
                    actLen = 0
                    print('getting the HELICS input info')
                    h.helicsInputGetString(self.subs[i], buffer, 256, 
                                           actLen)
                    print('getting the raw HELICS publication info')
                    h.helicsPublicationPublishRaw(self.pub[i], buffer, 
                                                  actLen)
            print('getting the federate time')
            print('hub - before request time async')
            h.helicsFederateRequestTimeAsync(self.vFed, self.finalTime + 0.05)
            cTime = h.helicsFederateRequestTime(
                self.vFed, self.finalTime + 0.05)
            h.helicsFederateRequestTimeComplete(self.vFed)
            print('hub - after request time complete')
        print('federate valid:', h.helicsFederateIsValid(self.vFed))
        print('finalizing the federate stuff')
        print('hub - before finalize async')
        h.helicsFederateFinalizeAsync(self.vFed)
        h.helicsFederateFinalize(self.vFed)
        h.helicsFederateFinalizeComplete(self.vFed)
        print('hub - after finalize complete')
        print('federate valid:', h.helicsFederateIsValid(self.vFed))
    
    def run(self, parties, vFed):
        print('###### starting the run function')
        self.vFed = vFed
        print('federate valid:', h.helicsFederateIsValid(self.vFed))
        self.parties = parties
        if self.readyToRun is False:
            self.makeReady(self.vFed)
        self.callOnReady(self.parties)
        self.mainLoop(self.vFed)
    
    def __del__(self):
        print('Inside the __del__ method of EchoHub_c class.')
        print('federate valid:', h.helicsFederateIsValid(self.vFed))
        h.helicsFederateFree(self.vFed)
        
            
class EchoLeaf_c:
    
    def __init__(self):
        print('Created the EchoLeaf class')
        self.vFed = None
        self.pub = None
        self.sub = None
        self.index_ = 0
        self.initialized = False
        self.readyToRun = False
        
    def callOnReady(self, parties):
        print('####### created the barrier object')
        brr = Barrier(parties)
        return brr
    
    def create_value_federate(self, coreName, index):
        print('####### creating federate info')
        name = 'echoleaf_{} Test--AA'.format(index)
        fi = h.helicsCreateFederateInfo()
        print('setting core name')
        h.helicsFederateInfoSetCoreName(fi, coreName)
        print('creating value federate info')
        global vFed
        vFed = h.helicsCreateValueFederate(name, fi)
        return vFed
    
    def initialize(self, vFed, index):
        print('####### initialized the EchoLeaf test')
        self.vFed = vFed
        print('federate valid:', h.helicsFederateIsValid(self.vFed))
        self.index_ = index
        leafname = 'leafsend_{}'.format(index)
        print('creating publication info')
        self.pub = h.helicsFederateRegisterGlobalPublication(
            self.vFed, leafname, h.helics_data_type_string, 
            "")
        leafname2 = 'leafrx_{}'.format(index)
        print('creating subscription info')
        self.sub = h.helicsFederateRegisterSubscription(
            self.vFed, leafname2, "")

        self.initialized = True
        # print(self.vFed)
    
    def makeReady(self, vFed):
        self.vFed = vFed
        print('federate valid:', h.helicsFederateIsValid(self.vFed))
        if self.initialized is False:
            print('must initizialize first')
        print('federate valid:', h.helicsFederateIsValid(self.vFed))
        print('starting the HELICS execution')
        print('leaf - before enter execution async')
        h.helicsFederateEnterExecutingModeAsync(self.vFed)
        h.helicsFederateEnterExecutingMode(self.vFed)
        h.helicsFederateEnterExecutingModeComplete(self.vFed)
        print('leaf - after enter execution complete')
        print('federate valid:', h.helicsFederateIsValid(self.vFed))
        self.readyToRun = True
    
    def mainLoop(self, vFed):
        print('######## starting the main loop')
        cnt = 0
        txstring = '{:<100000}{:<100}'.format(self.index_, '1')
        tbuffer = chr(256)
        itr = 5000
        self.vFed = vFed
        print('federate valid:', h.helicsFederateIsValid(self.vFed))
        while cnt <= itr + 1:
            print('requesting the next time')
            print('federate valid:', h.helicsFederateIsValid(self.vFed))
            print('leaf - before request time async')
            h.helicsFederateRequestTimeAsync(self.vFed, 1.0)
            h.helicsFederateRequestTime(self.vFed, 1.0)
            h.helicsFederateRequestTimeComplete(self.vFed)
            print('leaf - after request time complete')
            # h.helicsFederateRequestNextStep(self.vFed)
            print('federate valid:', h.helicsFederateIsValid(self.vFed))
            if cnt <= itr:
                print('getting the publication string')
                h.helicsPublicationPublishString(
                    self.pub, txstring)
            print('checking to se if HELICS infor is updated')
            if h.helicsInputIsUpdated(self.sub):
                actLen = 0
                print('getting the HELICS string input')
                h.helicsInputGetString(
                    self.sub, tbuffer, 256, actLen)
                if str(tbuffer) != txstring:
                    logging.error("incorrect string\n")
                    break
            cnt += 1
        print('finalizing the federate stuff')
        print('federate valid:', h.helicsFederateIsValid(self.vFed))
        print('leaf - before finalize async')
        h.helicsFederateFinalizeAsync(self.vFed)
        h.helicsFederateFinalize(self.vFed)
        h.helicsFederateFinalizeComplete(self.vFed)
        print('leaf - after finalize complete')
        print('federate valid:', h.helicsFederateIsValid(self.vFed))
        
    def run(self, parties, vFed):
        print('####### starting the run function')
        self.vFed = vFed
        print('federate valid:', h.helicsFederateIsValid(self.vFed))
        self.parties = parties
        if self.readyToRun is False:
           self.makeReady(self.vFed)
        self.callOnReady(self.parties)
        self.mainLoop(self.vFed)
    
    def __del__(self):
        print('Inside the __del__ method of the EchoLeaf_c class')
        print('federate valid:', h.helicsFederateIsValid(self.vFed))
        h.helicsFederateFree(self.vFed)
        

def create_output_file(benchmark, output_path, filename):
    helics_version = h.helicsGetVersion()
    zmq_version = h.getZMQVersion()
    compiler = platform.python_compiler()
    build_flags = platform.python_build()
    processor = platform.processor()
    
    string = 'HELICS BENCHMARK {}\n'.format(benchmark)
    string += '------------HELICS BUILD INFO -------------\n'
    string += 'HELICS VERSION: {}\n'.foramt(helics_version)
    string += 'ZMQ VERSION: {}\n'.format(zmq_version)
    string += 'COMPILER INFO: {}\n'.format(compiler)
    string += 'BUILD FLAGS: {}\n'.format(build_flags)
    string += '------------PROCESSOR INFO ----------------\n'
    string += 'HOST PROCESSOR TYPE: {}\n'.format(processor)
    string += 'CPU MODEL: {}\n'.format(
        cpuinfo.get_cpu_info().get('brand_raw'))
    string += 'NUM CPUS: {}\n'.format(multiprocessing.cpu_count())
    string += '-------------------------------------------'
    
    with open('{}\{}.json'.format(output_path, filename), 'r') as input_file:
        data = json.loads(input_file)
    string += data
    
    with open('{}\{}.txt'.format(output_path, filename), 'w') as output_file:
        output_file.write(string)


def timer():
    print('Starting timer')
        

def BMecho_singleCore(federates):
    print('starting to run the single core test')
    t = Timer(1, timer)
    t.cancel()
    feds = [f for f in range(0, federates)]
    print('feds: ', feds)
    wcore = h.helicsCreateCore(
        'inproc',
        None,
        '--autobroker --federates={}'.format((federates)))
    hub = EchoHub_c()
    hub_vFed = hub.create_value_federate(h.helicsCoreGetIdentifier(wcore))
    print('federate valid:', h.helicsFederateIsValid(hub_vFed))
    print('federate state:', h.helicsFederateGetState(hub_vFed))
    hub.initialize(hub_vFed, federates)
    leafs = [EchoLeaf_c() for f in range(0, federates)]
    i = 0
    leaf_vFeds = []
    for f in feds:
        leaf_vFed = leafs[f].create_value_federate(
            h.helicsCoreGetIdentifier(wcore), i)
        leafs[f].initialize(leaf_vFed, i)
        leaf_vFeds.append(leaf_vFed)
        i += 1
    print('creating the thread')
    threads = []
    i = 0
    for l, f in zip(leaf_vFeds, feds):
        x = Thread(
            target=leafs[f].run, 
            name=leafs[i], 
            args=(len(feds)+1, l))
        threads.append(x)
        x.start()
        i += 1
    hub.makeReady(hub_vFed)
    brr = hub.callOnReady(len(feds)+1)
    brr.wait()
    t.start()
    hub.run(hub_vFed)
    t.cancel()
    for thrd in threads:
        thrd.join()
    h.helicsCoreFree(wcore)
    h.helicsCleanupLibrary()


def BMecho_multiCore(cTypeString, federates):
    print('starting the multicore test.')
    t = Timer(1, timer)
    t.cancel()
    print('checking to see if the core is available')
    if h.helicsIsCoreTypeAvailable(cTypeString) == h.helics_false:
        t.start()
    feds = [f for f in range(0, federates)]
    print('feds: ', feds)
    initString = "--log-level=no_print --federates={}".format(federates)
    print('creating the broker')
    broker = h.helicsCreateBroker(
        cTypeString, "brokerb", initString, None)

    wcore = h.helicsCreateCore(
        cTypeString, "", "--federates=1 --log_level=no_print")
    hub = EchoHub_c()
    hub_vFed = hub.create_value_federate(h.helicsCoreGetIdentifier(wcore))
    print('federate valid:', h.helicsFederateIsValid(hub_vFed))
    print('helics state:', h.helicsFederateGetState(hub_vFed))
    hub.initialize(hub_vFed, federates)
    leafs = [EchoLeaf_c() for f in range(0, federates)]
    cores = [h.helics_cores() for f in range(0, federates)]
    i = 0
    leaf_vFeds = []
    for f in feds:
        print('creating the core')
        cores[f] = h.helicsCreateCore(
            cTypeString, None, "-f 1 --log_level=no_print")
        print('connecting the cores')
        h.helicsCoreConnect(cores[f], None)
        leaf_vFed = leafs[f].create_value_federate(
            h.helicsCoreGetIdentifier(cores[i]))
        leafs[f].initialize(leaf_vFed, i)
        leaf_vFeds.append(leaf_vFed)
        i += 1
    print('creating the thread list')
    threads = []
    i = 0
    for l, f in zip(leaf_vFeds, feds):
        x = Thread(
            target=leafs[f].run, 
            name=leafs[i], 
            args=(len(feds)+1, l))
        threads.append(x)
        x.start()
        i += 1
    hub.makeReady(hub_vFed)
    brr = hub.callOnReady(len(feds)+1)
    brr.wait()
    t.start()
    hub.run(hub_vFed)
    t.cancel()
    for thrd in threads:
        thrd.join()
    print('disconnecting the broker')
    h.helicsBrokerDisconnect(broker, None)
    h.helicsBrokerFree(broker)

    for cr in cores:
        h.helicsCoreFree(cr)
    cores.clear()
    print('clearing the core')
    h.helicsCoreFree(wcore)
    h.helicsCleanupLibrary()


def _auto_run(args):
    """This function runs this script as a stand-alone executable."""
    print('Starting this code and performing benchmarks')
    bench(BMecho_singleCore(1))
    # bench.json(BMecho_singleCore(1), 
    #            os.path.join(args.output_path, 'test1.json'))
    
    # bench(BMecho_multiCore('inproc', 1), iterations=1, rounds=1)
    # bench.json(BMecho_multiCore('inproc', 1), 
    #            os.path.join(args.output_path, 'test2.json'))
    
    # bench(BMecho_multiCore('zmq', 1), iterations=1, rounds=1)
    # bench.json(BMecho_multiCore('zmq', 1), 
    #            os.path.join(args.output_path, 'test3.json'))
    
    # bench(BMecho_multiCore('zmqss', 1), iterations=1, rounds=1)
    # bench.json(BMecho_multiCore('zmqss', 1), 
    #            os.path.join(args.output_path, 'test4.json'))        
    
    # bench(BMecho_multiCore('ipc', 1), iterations=1, rounds=1)
    # bench.json(BMecho_multiCore('ipc', 1), 
    #            os.path.join(args.output_path, 'test5.json'))
    
    # bench(BMecho_multiCore('tcp', 1), iterations=1, rounds=1)
    # bench.json(BMecho_multiCore('tcp', 1), 
    #            os.path.join(args.output_path, 'test6.json'))
    
    # bench(BMecho_multiCore('tcpss', 1), iterations=1, rounds=1)
    # bench.json(BMecho_multiCore('tcpss', 1), 
    #            os.path.join(args.output_path, 'test7.json'))
    
    # bench(BMecho_multiCore('udp', 1), iterations=1, rounds=1)
    # bench.json(BMecho_multiCore('udp', 1), 
    #            os.path.join(args.output_path, 'test8.json'))
    
    # print('saving the data')
    # for root, dirs, files in os.walk(os.path.join(args.output_path)):
    #     for file in files:
    #         if '.json' in file:
    #             create_output_file(
    #                 'echoBenchmark', 
    #                 os.path.join(root, file))

if __name__ == '__main__':
    fileHandle = logging.FileHandler("echoBenchmarks.log", mode='w')
    fileHandle.setLevel(logging.DEBUG)
    streamHandle = logging.StreamHandler(sys.stdout)
    streamHandle.setLevel(logging.ERROR)
    logging.basicConfig(level=logging.INFO,
                        handlers=[fileHandle, streamHandle])

    parser = argparse.ArgumentParser(
        description='Produce benchmark results.')
    script_path = os.path.dirname(os.path.realpath(__file__))
#    print(script_path)
    head, tail = os.path.split(script_path)
    parser.add_argument('-o', 
                        '--output_path', 
                        nargs='?', 
                        default=os.path.join(head))
    args = parser.parse_args()
    _auto_run(args)