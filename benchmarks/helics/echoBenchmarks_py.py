# -*- coding: utf-8 -*-
"""
Created on Mon May 11 09:59:05 2020

Performs echoBenchmark of the current HELICS Version.  It provides some
performacne results of HELICS for a single core and multiple cores for a
variety of core types.

The command line arguments for the function can be found in the code
following the lines following the "if __name__ == '__main__':" line
at the end of this file.

@author: barn553
"""

import helics as h
import sys
import sysconfig
import os
import argparse
import pprint
import string
import random
import logging
from threading import Thread, Barrier, Timer
import json
import cpuinfo
import multiprocessing
import platform
import datetime
import time
import timeit
import psutil

# Setting up logger
logger = logging.getLogger(__name__)

# Setting up pretty printer
pp = pprint.PrettyPrinter(indent=4)


class EchoHub_c:
    def __init__(self):
        self.finalTime = 0.1
        self.vFed = None
        self.pubs = []
        self.subs = []
        self.cnt_ = 10
        self.initialized = False
        self.readyToRun = False
        logging.info("created the echo hub")

    def call_on_ready(self, parties):
        """This function creates the barrier for running the tests with
        multiple threads.

        Args:
            parties (int) - The number of barriers to create.  In this
            case, it is the number of federates plus 1.

        Returns:
            brr (barrier object) - The barrier for the test.
        """
        brr = Barrier(parties)
        logging.info("echo hub - created the barrier object")
        return brr

    def create_value_federate(self, coreName):
        """This function creates a value federate.

        Args:
            coreName (str) - The name of the core for creating the
            value federate.

        Returns:
            vFed (helics federate object) - The value federate.
        """
        name = "echohub Test--T"
        fi = h.helicsCreateFederateInfo()
        h.helicsFederateInfoSetCoreName(fi, coreName)
        global vFed
        vFed = h.helicsCreateValueFederate(name, fi)
        logging.info("echo hub - created the value federate")
        return vFed

    def initialize(self, vFed, cnt):
        """This function prepares the data for running the test.

        Args:
            vFed (helics federate object) - The value federate.

            cnt (int) - An initial number.  In this case, it is
            the number of federates.

        Returns:
            (null)
        """
        logging.info("echo hub - preparing the data for the run")
        self.vFed = vFed
        self.cnt_ = cnt
        i = 0
        while i < self.cnt_:
            leafname = "leafrx_{}".format(i)
            self.pubs.append(
                h.helicsFederateRegisterGlobalPublication(
                    self.vFed, leafname, h.helics_data_type_string, ""
                )
            )
            leafname2 = "leafsend_{}".format(i)
            self.subs.append(h.helicsFederateRegisterSubscription(self.vFed, leafname2, ""))
            i += 1
        self.initialized = True
        logging.info("echo hub - the data is prepared for the run")

    def make_ready(self, vFed):
        """This function assert that the test is ready to execute.

        Args:
            vFed (helics federate object) - The value federate

        Returns:
            (null)
        """
        logging.info("echo hub - making sure the test is ready to run")
        self.vFed = vFed
        if self.initialized is not True:
            logging.debug("the test has not been initialized for echo hub")
            raise Exception("Must initialize first")
        sys.stdout.flush()
        h.helicsFederateEnterExecutingModeAsync(self.vFed)
        while h.helicsFederateIsAsyncOperationCompleted(self.vFed) == 0:
            pass
        sys.stdout.flush()
        h.helicsFederateEnterExecutingModeComplete(self.vFed)
        sys.stdout.flush()
        self.readyToRun = True
        logging.info("echo hub - the test is ready to run")

    def _main_loop(self, vFed):
        """The main loop for running the HELICS functions.

        Args:
            vFed (helics federate object) - The value federate.

        Returns:
            (null)
        """
        self.vFed = vFed
        buffer = chr(256)
        cTime = h.helics_time_zero
        logging.info("echo hub - starting the helics functions")
        while cTime <= self.finalTime:
            i = 0
            for c in range(0, self.cnt_):
                if h.helicsInputIsUpdated(self.subs[i]):
                    actLen = 0
                    h.helicsInputGetString(self.subs[i], buffer, 256, actLen)
                    h.helicsPublicationPublishRaw(self.pub[i], buffer, actLen)
            h.helicsFederateRequestTimeAsync(self.vFed, self.finalTime + 0.05)
            while h.helicsFederateIsAsyncOperationCompleted(self.vFed) == 0:
                pass
            cTime = h.helicsFederateRequestTimeComplete(self.vFed)
        h.helicsFederateFinalizeAsync(self.vFed)
        while h.helicsFederateIsAsyncOperationCompleted(self.vFed) == 0:
            pass
        h.helicsFederateFinalizeComplete(self.vFed)
        logging.info("echo hub - the helics functions have been completed")

    def run(self, parties, vFed):
        """This function executes all the above functions.  This function
        is what we are benchmarking to evaluate its performance.

        Args:
            parties (int) - The number of barriers to create for the threads.
            In this case, it is the number of federates plus 1.

            vFed (helics federate object) - The value federate.

        Returns:
            (null)
        """
        logging.info("echo hub - starting the execution of the helics functions")
        self.vFed = vFed
        self.parties = parties
        if not self.readyToRun:
            self.make_ready(self.vFed)
        sys.stdout.flush()
        self.call_on_ready(self.parties)
        sys.stdout.flush()
        self._main_loop(self.vFed)
        logging.info("echo hub - finished the execution of the helics functions")

    def __del__(self):
        h.helicsFederateFree(self.vFed)
        logging.info("echo hub - the test is done -> information is cleared")


class EchoLeaf_c:
    def __init__(self):
        self.vFed = None
        self.pub = None
        self.sub = None
        self.index_ = 0
        self.initialized = False
        self.readyToRun = False
        logging.info("created the echo leaf")

    def call_on_ready(self, parties):
        """This function creates the barrier for running the tests with
        multiple threads.

        Args:
            parties (int) - The number of barriers to create.  In this
            case, it is the number of federates plus 1.

        Returns:
            brr (barrier object) - The barrier for the test.
        """
        brr = Barrier(parties)
        logging.info("echo leaf - created the barrier object")
        return brr

    def create_value_federate(self, coreName, index):
        """This function creates a value federate.

        Args:
            coreName (str) - The name of the core for creating the
            value federate.

            index (int) - The number that indicates which value federate
            is created and used during the test.

        Returns:
            vFed (helics federate object) - The value federate.
        """
        name = "echoleaf_{} Test--T".format(index)
        fi = h.helicsCreateFederateInfo()
        h.helicsFederateInfoSetCoreName(fi, coreName)
        global vFed
        vFed = h.helicsCreateValueFederate(name, fi)
        logging.info("echo leaf - created the value federate")
        return vFed

    def initialize(self, vFed, index):
        """This function prepares the data for running the test.

        Args:
            vFed (helics federate object) - The value federate.

            index (int) - An identifying number for the name of the leaf.

        Returns:
            (null)
        """
        logging.info("echo leaf - preparing the data for the run")
        self.vFed = vFed
        self.index_ = index
        leafname = "leafsend_{}".format(index)
        self.pub = h.helicsFederateRegisterGlobalPublication(
            self.vFed, leafname, h.helics_data_type_string, ""
        )
        leafname2 = "leafrx_{}".format(index)
        self.sub = h.helicsFederateRegisterSubscription(self.vFed, leafname2, "")
        self.initialized = True
        logging.info("echo leaf - the data is prepared for the run")

    def make_ready(self, vFed):
        """This function assert that the test is ready to execute.

        Args:
            vFed (helics federate object) - The value federate

        Returns:
            (null)
        """
        self.vFed = vFed
        logging.info("echo leaf - making sure the test is ready to run")
        if self.initialized is False:
            raise Exception("must initizialize first")
        sys.stdout.flush()
        h.helicsFederateEnterExecutingModeAsync(self.vFed)
        while h.helicsFederateIsAsyncOperationCompleted(self.vFed) == 0:
            pass
        sys.stdout.flush()
        h.helicsFederateEnterExecutingModeComplete(self.vFed)
        sys.stdout.flush()
        self.readyToRun = True
        logging.info("echo leaf - the test is ready to run")

    def _main_loop(self, vFed):
        """The main loop for running the HELICS functions.

        Args:
            vFed (helics federate object) - The value federate.

        Returns:
            (null)
        """
        cnt = 0
        txstring = "{:<100000}{:<100}".format(self.index_, "1")
        tbuffer = chr(256)
        itr = 5000
        self.vFed = vFed
        logging.info("echo leaf - starting the helics functions")
        while cnt <= itr + 1:
            h.helicsFederateRequestTimeAsync(self.vFed, 1.0)
            while h.helicsFederateIsAsyncOperationCompleted(self.vFed) == 0:
                pass
            h.helicsFederateRequestTimeComplete(self.vFed)
            if cnt <= itr:
                h.helicsPublicationPublishString(self.pub, txstring)
            if h.helicsInputIsUpdated(self.sub):
                actLen = 0
                h.helicsInputGetString(self.sub, tbuffer, 256, actLen)
                if str(tbuffer) != txstring:
                    logging.error("incorrect string\n")
                    break
            cnt += 1
        h.helicsFederateFinalizeAsync(self.vFed)
        while h.helicsFederateIsAsyncOperationCompleted(self.vFed) == 0:
            pass
        h.helicsFederateFinalizeComplete(self.vFed)
        logging.info("echo leaf - the helics functions have been completed")

    def run(self, parties, vFed):
        """This function executes all the above functions.  This function
        is what we are benchmarking to evaluate its performance.

        Args:
            parties (int) - The number of barriers to create for the threads.
            In this case, it is the number of federates plus 1.

            vFed (helics federate object) - The value federate.

        Returns:
            (null)
        """
        logging.info("echo leaf - starting the execution of the helics functions")
        self.vFed = vFed
        self.parties = parties
        if not self.readyToRun:
            self.make_ready(self.vFed)
        sys.stdout.flush()
        self.call_on_ready(self.parties)
        sys.stdout.flush()
        self._main_loop(self.vFed)
        logging.info("echo leaf - finished the execution of the helics functions")

    def __del__(self):
        h.helicsFederateFree(self.vFed)
        logging.info("echo leaf - the test is done -> information is cleared")


def timer():
    logging.info("starting the timer")


def BMecho_singleCore(federates):
    """This function performs the echo test.

    Args:
        federates (int) - The number of federates to create for the single
        core echo test.

    Returns:
        (null)
    """
    logging.info("starting the single core test")
    t = Timer(1, timer)
    t.cancel()
    feds = [f for f in range(0, federates)]
    wcore = h.helicsCreateCore("inproc", None, "--autobroker --federates={}".format((federates)))
    hub = EchoHub_c()
    hub_vFed = hub.create_value_federate(h.helicsCoreGetIdentifier(wcore))
    hub.initialize(hub_vFed, federates)
    leafs = [EchoLeaf_c() for f in range(0, federates)]
    i = 0
    leaf_vFeds = []
    logging.info("preparing the federates")
    for f in feds:
        leaf_vFed = leafs[f].create_value_federate(h.helicsCoreGetIdentifier(wcore), i)
        leafs[f].initialize(leaf_vFed, i)
        leaf_vFeds.append(leaf_vFed)
        i += 1
    threads = []
    i = 0
    logging.info("creating the threads")
    for l, f in zip(leaf_vFeds, feds):
        x = Thread(target=leafs[f].run, name=leafs[f], args=(len(feds) + 1, l))
        threads.append(x)
        x.start()
        i += 1
    time.sleep(0.1)
    hub.make_ready(hub_vFed)
    logging.info("executing the echo hub")
    t.start()
    hub.run(len(feds) + 1, hub_vFed)
    t.cancel()
    logging.info("joining the threads")
    for thrd in threads:
        thrd.join()
    h.helicsCoreFree(wcore)
    h.helicsCleanupLibrary()
    logging.info("finished the single core test")


def BMecho_multiCore(cTypeString, federates):
    """This function performs the multicore test for a specific core
    type.

    Args:
        cTypeString (str) - Specific core type, e.g. inproc

        federates (int) - The number of federates to create for the echo
        multicore test.

    Returns:
        (null)
    """
    logging.info("starting the multicore test for {}".format(cTypeString))
    t = Timer(1, timer)
    t.cancel()
    if h.helicsIsCoreTypeAvailable(cTypeString) == h.helics_false:
        t.start()
    feds = [f for f in range(0, federates)]
    initString = "--log-level=no_print --federates={}".format(federates)
    broker = h.helicsCreateBroker(cTypeString, "brokerf", initString)
    wcore = h.helicsCreateCore(cTypeString, "", "--federates=1 --log_level=no_print")
    hub = EchoHub_c()
    hub_vFed = hub.create_value_federate(h.helicsCoreGetIdentifier(wcore))
    hub.initialize(hub_vFed, federates)
    leafs = [EchoLeaf_c() for f in range(0, federates)]
    cores = []
    i = 0
    leaf_vFeds = []
    logging.info("preparing the federates")
    for f in feds:
        core = h.helicsCreateCore(cTypeString, None, "-f 1 --log_level=no_print")
        h.helicsCoreConnect(core)
        leaf_vFed = leafs[f].create_value_federate(h.helicsCoreGetIdentifier(core), i)
        leafs[f].initialize(leaf_vFed, i)
        leaf_vFeds.append(leaf_vFed)
        cores.append(core)
        i += 1
    threads = []
    i = 0
    logging.info("creating the threads")
    for l, f in zip(leaf_vFeds, feds):
        x = Thread(target=leafs[f].run, name=leafs[f], args=(len(feds) + 1, l))
        threads.append(x)
        x.start()
        i += 1
    time.sleep(0.1)
    hub.make_ready(hub_vFed)
    logging.info("executing the echo hub")
    t.start()
    hub.run(len(feds) + 1, hub_vFed)
    t.cancel()
    logging.info("joining the threads")
    for thrd in threads:
        thrd.join()
    h.helicsBrokerDisconnect(broker)
    h.helicsBrokerFree(broker)
    logging.info("clearing the cores")
    for cr in cores:
        h.helicsCoreFree(cr)
    cores.clear()
    h.helicsCoreFree(wcore)
    h.helicsCleanupLibrary()
    logging.info("finished the multicore test for {}".format(cTypeString))


def create_bm_dictionary(name, federate_count, core_type, real_time, cpu_time, threads):
    """This function creates a dictionary for a single benchmark
    run.

    Args:
        name (str) - The name of the benchmark, e.g. BMecho_singleCore
        federate_count (int) - The number of federates.

        core_type (str) - The name of the core type.

        real_time (float) - The human-interpreted time it takes to
        execute this script.

        cpu_time (float) - The time it takes a CPU to execute this script.

        threads (int) - The number of threads.

    Returns:
        bm_dict (dict) - A dictionary of the benchmark results.
    """
    if name == "BMecho_singleCore":
        bm_dict = {
            "name": "{}/{}/iterations:1/real_time".format(name, federate_count),
            "run_name": "{}/{}/iterations:1/real_time".format(name, federate_count),
            "run_type": "iteration",
            "repetitions": 1,
            "repetitions_index": 1,
            "threads": threads,
            "iterations": 1,
            "real_time": real_time,
            "cpu_time": cpu_time,
            "time_unit": "s",
        }
    else:
        bm_dict = {
            "name": "{}/{}Core/{}/real_time".format(name, core_type, federate_count),
            "run_name": "{}/{}Core/{}/real_time".format(name, core_type, federate_count),
            "run_type": "iteration",
            "repetitions": 1,
            "repetitions_index": 1,
            "threads": threads,
            "iterations": 1,
            "real_time": real_time,
            "cpu_time": cpu_time,
            "time_unit": "s",
        }
    return bm_dict


def wrapper(func, *args, **kwargs):
    """This is a wrapper function to be used for benchmarking.  It allows
    func to be passed directly to timeit.

    Args:
        func (function) - The function to be wrapped.

    Returns:
        wrapped (function) - The original function, but in a format that
        is ready to be passed to the timeit function call.
    """

    def wrapped():
        return func(*args, **kwargs)

    return wrapped


def create_output_file(benchmark, output_path, filename, date, bm_dicts):
    """This function creates the output file, which contains some basic
    information, along with the benchmark results.

    Args:
        benchmark (str) - The name of the benchmark, e.g. echoBenchmark

        output_path (str) - The location to send the results.

        filename (str) - The name of the results file.

        date (datetime object) - The date and time of the benchmark run.

        bm_dicts (list) - The list of benchmark results.

    Returns:
        (null)
    """
    helics_version = h.helicsGetVersion()
    cpu_freq = psutil.cpu_freq()
    # zmq_version = h.getZMQVersion()
    s, v, c = platform.system(), platform.version(), platform.python_compiler()
    compiler = "{}-{}:{}".format(s, v, c)
    build_flags_dict = sysconfig.get_config_vars()
    build_flags = (
        str(build_flags_dict.get("base")) + "\\" + "py{}".format(build_flags_dict.get("py_version"))
    )
    machine = platform.machine()
    # NOTE: To get the host name, do platform.node()
    # Creating the header string
    string = "HELICS_BENCHMARK: {}\n".format(benchmark)
    string += "------------HELICS BUILD INFO -------------\n"
    string += "HELICS VERSION: {}\n".format(helics_version)
    # string += 'ZMQ VERSION: {}\n'.format(zmq_version)
    string += "COMPILER INFO: {}\n".format(compiler)
    string += "BUILD FLAGS: {}\n".format(build_flags)
    string += "------------PROCESSOR INFO ----------------\n"
    string += "HOST PROCESSOR TYPE: {}\n".format(machine)
    string += "CPU MODEL: {}\n".format(cpuinfo.get_cpu_info().get("brand"))
    string += "NUM CPU: {}\n".format(multiprocessing.cpu_count())
    string += "-------------------------------------------\n"
    bm_dict = {
        "context": {
            "date": date,
            "host_name": platform.node(),
            "executable": sys.executable,
            "num_cpus": multiprocessing.cpu_count(),
            "mhz_per_cpu": cpu_freq.max,
            "cpu_scaling_enabled": False,
            "caches": [],
            "load_avg": [],
            "library_build_type": "release",
        },
        "benchmarks": bm_dicts,
    }
    bm_dict = json.dumps(bm_dict, indent=2)
    string += str(bm_dict)
    # Combing the header string with the benchmark dictionary
    with open("{}\\{}.txt".format(output_path, filename), "w") as output_file:
        output_file.write(string)


def _auto_run(args):
    """This function runs this script as a stand-alone executable.

    Args:
        '-p' or '--power' - An integaer, including 0, used to represent
        how many federates should be created, e.g. 2**p where p equals 0, 1,
        2, etc.

        '-o' or '--output_path' - The path to send the benchmark results.

    Returns:
        (null)
    """
    logging.info("starting the echoBenchmark run")
    benchmarks = []
    assert isinstance(args.power, int)
    for i in range(0, args.power):
        single = wrapper(BMecho_singleCore, 2 ** i)
        single_real_time_start = time.time()
        BMecho_singleCore(2 ** i)
        single_real_time_stop = time.time()
        single_cpu_time = timeit.timeit(stmt=single, number=1)
        single_real_time = single_real_time_stop - single_real_time_start
        single_dict = create_bm_dictionary(
            "BMecho_singleCore", 2 ** i, "singleCore", single_real_time, single_cpu_time, 1
        )
        inproc = wrapper(BMecho_multiCore, "inproc", 2 ** i)
        inproc_real_time_start = time.time()
        BMecho_multiCore("inproc", 2 ** i)
        inproc_real_time_stop = time.time()
        inproc_cpu_time = timeit.timeit(stmt=inproc, number=1)
        inproc_real_time = inproc_real_time_stop - inproc_real_time_start
        inproc_dict = create_bm_dictionary(
            "BMecho_multiCore", 2 ** i, "inproc", inproc_real_time, inproc_cpu_time, 1
        )
        zmq = wrapper(BMecho_multiCore, "zmq", 2 ** i)
        zmq_real_time_start = time.time()
        BMecho_multiCore("zmq", 2 ** i)
        zmq_real_time_stop = time.time()
        zmq_cpu_time = timeit.timeit(stmt=zmq, number=1)
        zmq_real_time = zmq_real_time_stop - zmq_real_time_start
        zmq_dict = create_bm_dictionary(
            "BMecho_multiCore", 2 ** i, "zmq", zmq_real_time, zmq_cpu_time, 1
        )
        zmqss = wrapper(BMecho_multiCore, "zmqss", 2 ** i)
        zmqss_real_time_start = time.time()
        BMecho_multiCore("zmqss", 2 ** i)
        zmqss_real_time_stop = time.time()
        zmqss_cpu_time = timeit.timeit(stmt=zmqss, number=1)
        zmqss_real_time = zmqss_real_time_stop - zmqss_real_time_start
        zmqss_dict = create_bm_dictionary(
            "BMecho_multiCore", 2 ** i, "zmqss", zmqss_real_time, zmqss_cpu_time, 1
        )
        udp = wrapper(BMecho_multiCore, "udp", 2 ** i)
        udp_real_time_start = time.time()
        BMecho_multiCore("udp", 2 ** i)
        udp_real_time_stop = time.time()
        udp_cpu_time = timeit.timeit(stmt=udp, number=1)
        udp_real_time = udp_real_time_stop - udp_real_time_start
        udp_dict = create_bm_dictionary(
            "BMecho_multiCore", 2 ** i, "udp", udp_real_time, udp_cpu_time, 1
        )
        # NOTE: The following core types take way too long to complete.
        # This indicates there is an issue that cannot be fixed by Python,
        # but within HELICS for these core types to work.  When these core
        # types finally work, uncomment these lines and update the script
        # to match the above lines and run this code to include their
        # results.
        # ipc = wrapper(BMecho_multiCore, 'ipc', 1)
        # print('ipc core before timeit')
        # ipc_b = timeit.timeit(stmt=ipc, number=1)
        # print('ipc core after timeit')
        # ipc_dict = create_bm_dictionary(
        #     'BMecho_multiCore', 2**i, 'ipc', ipc_b, 1)
        # tcp = wrapper(BMecho_multiCore, 'tcp', 1)
        # tcp_b = timeit.timeit(stmt=tcp, number=1)
        # tcp_dict = create_bm_dictionary(
        #     'BMecho_multiCore', 2**i, 'tcp', tcp_b, 1)
        # tcpss = wrapper(BMecho_multiCore, 'tcpss', 1)
        # tcpss_b = timeit.timeit(stmt=tcpss, number=1)
        # tcpss_dict = create_bm_dictionary(
        #     'BMecho_multiCore', 2**i, 'tcpss', tcpss_b, 1)
        benchmarks.append([single_dict, inproc_dict, zmq_dict, zmqss_dict, udp_dict])
    # Simplifying the benchmarks list before adding it to the output file
    benchmarks = [val for sublist in benchmarks for val in sublist]
    # Getting the current date and time of the run
    date_time = "{}".format(datetime.datetime.now())
    run_id = "".join(random.choices(string.ascii_uppercase + string.digits, k=5))
    create_output_file(
        "echoBenchmark",
        args.output_path,
        "bm_echo_pyResults{}_{}".format(datetime.date.today(), str(run_id)),
        date_time,
        benchmarks,
    )
    logging.info("finished the echoBenchmark run")


if __name__ == "__main__":
    fileHandle = logging.FileHandler("echoBenchmarks.log", mode="w")
    fileHandle.setLevel(logging.DEBUG)
    streamHandle = logging.StreamHandler(sys.stdout)
    streamHandle.setLevel(logging.ERROR)
    logging.basicConfig(level=logging.INFO, handlers=[fileHandle, streamHandle])

    parser = argparse.ArgumentParser(description="Produce benchmark results.")
    script_path = os.path.dirname(os.path.realpath(__file__))
    # print(script_path)
    head, tail = os.path.split(script_path)
    parser.add_argument("-p", "--power", nargs="?", default=2)
    parser.add_argument("-o", "--output_path", nargs="?", default=os.path.join(head))
    args = parser.parse_args()
    _auto_run(args)
