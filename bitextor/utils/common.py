#  This file is part of Bitextor.
#
#  Bitextor is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  Bitextor is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with Bitextor.  If not, see <https://www.gnu.org/licenses/>.

try:
    import lzma
except ImportError:
    from backports import lzma

import gzip
from contextlib import contextmanager

import subprocess
import sys
import os
import requests
import shlex
import time
import random
import traceback

from pytools.persistent_dict import PersistentDict, NoSuchEntryError

class ExternalTextProcessor(object):

    def __init__(self, cmd, raise_exception=True, return_debug_data=False):
        self.raise_exception = raise_exception
        self.return_debug_data = return_debug_data

        if isinstance(cmd, str):
            # Split the command as bash does
            #  This adds support to quotes (e.g. "echo 'hi, bye'" -> ["echo", "hi, bye"] -> "hi, bye")
            #  Without quotes support, if quotes are added, the command might be, and likely will be, incorrect
            #   (e.g. "echo 'hi, bye'" -> ["echo", "'hi,", "bye'"] -> "'hi, bye'")
            self.cmd = shlex.split(cmd)
        else:
            self.cmd = cmd

    def process(self, input_text):
        proc = subprocess.Popen(self.cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        outs, errs = proc.communicate(input=bytes(input_text, encoding='utf-8'))
        output = outs.decode('utf-8')
        error_output = errs.decode('utf-8')
        returncode = proc.returncode

        if self.raise_exception and returncode != 0:
            raise Exception(f"External tool exited with the non-zero code {returncode}: {error_output.strip()}")

        if self.return_debug_data:
            return output, error_output, returncode

        return output


@contextmanager
def open_xz_or_gzip_or_plain(file_path, mode='rt'):
    f = None
    try:
        if file_path[-3:] == ".gz":
            f = gzip.open(file_path, mode)
        elif file_path[-3:] == ".xz":
            f = lzma.open(file_path, mode)
        else:
            f = open(file_path, mode)
        yield f

    except Exception:
        raise Exception("Error occurred while loading a file!")

    finally:
        if f:
            f.close()


@contextmanager
def dummy_open():
    yield None


def build_mappings(file_path_from, file_path_to, column=None, dem='\t'):
    mapping = {}

    def next_or_next_in_column(handler):
        if not column:
            return next(handler, None)

        text = next(handler, None)
        if text:
            return text.split(dem)[column]

        return text

    with open_xz_or_gzip_or_plain(file_path_from) as f_from, open_xz_or_gzip_or_plain(file_path_to) as f_to:
        line_from = next_or_next_in_column(f_from)
        line_to = next_or_next_in_column(f_to)

        while line_from and line_to:
            line_from = line_from.strip()
            mapping[line_from] = line_to.strip()

            line_from = next_or_next_in_column(f_from)
            line_to = next_or_next_in_column(f_to)

    return mapping


def check_lengths(file_path_from, file_path_to, throw=True):
    f1_lines = 0
    f2_lines = 0
    with open_xz_or_gzip_or_plain(file_path_from) as f:
        for _ in f:
            f1_lines += 1

    with open_xz_or_gzip_or_plain(file_path_to) as f:
        for _ in f:
            f2_lines += 1

    if throw and f1_lines != f2_lines:
        raise Exception("Files must have the same number of lines!\
                            {0}: {1}, {2}: {3}".format(file_path_from, f1_lines, file_path_to, f2_lines))

    return f1_lines == f2_lines


def check_connection(url):
    connection_error = False
    connection = None

    for check in range(2):
        try:
            connection = requests.get(url, timeout=15)
        except requests.exceptions.ConnectTimeout:
            if check:
                connection_error = True
            else:
                url = f"https{url[4:]}"
        except BaseException as e:
            if check:
                connection_error = True
                sys.stderr.write(f"WARNING: error connecting to {url}\n")
                sys.stderr.write(str(e) + "\n")

    return connection_error, url


_duration_suffix = {"s": 1, "m": 60, "h": 3600, "d": 86400, "w": 604800}
# expected duration: integer + one-letter suffix
# allowed suffixes: s (seconds), m (minutes), h (hours), d (days), w (weeks)
def duration_to_seconds(value):
    suffix = value[-1]
    seconds = int(value[:-1]) * _duration_suffix[suffix]
    return seconds


def return_dict_value_if_key(d, k, else_value, pos_value=None, only_check_key=False, apply_function=None):
    condition = k in d if only_check_key else k in d and d[k]

    if pos_value:
        result = pos_value if condition else else_value
    else:
        result = d[k] if condition else else_value

    if apply_function:
        result = apply_function(result)

    return result


def print_alternatively_lines(input_file="-", blocks=2):
    input_fd = sys.stdin if input_file == "-" else open(input_file)
    lines = []

    for line in input_fd:
        lines.append(line.strip())

    offset = len(lines) // blocks

    if len(lines) % blocks != 0:
        raise Exception(f"Provided lines mod blocks did not pass: {len(lines)} mod {blocks} != 0")

    for idx in range(len(lines)):
        if idx >= offset:
            break

        for i in range(blocks):
            print(lines[idx + offset * i])

    if input_file == "-":
        input_fd.close()


def get_all_idxs_from_list(l, element):
    idxs = []
    find_idx = 0

    while find_idx < len(l):
        try:
            idxs.append(l.index(element, find_idx))

            find_idx = idxs[-1] + 1
        except ValueError:
            find_idx = len(l)

    return idxs


def get_snakemake_execution_mark(tmp_path):
    mark_path = f"{tmp_path}/mark_first_execution"

    return mark_path


def is_first_snakemake_execution(tmp_path, create_mark=False):
    mark_path = get_snakemake_execution_mark(tmp_path)

    if path_exists(mark_path):
        return False

    # Create mark?
    if create_mark:
        with open(mark_path, "w"):
            pass

    return True


def path_exists(path, expand=True, f=os.path.isfile):
    _path = path

    if expand:
        _path = os.path.expanduser(path)

    return f(_path)

def debug_if_true(msg, debug, flush=True):
    if debug:
        sys.stderr.write(f"--- DEBUG [{time.time()}]: {msg}\n")

        if flush:
            sys.stderr.flush()

#_allocated_cuda_devices_storage = PersistentDict(f"gpu_{os.getpgrp()}") # thread-safe -> it handles concurrency
_allocated_cuda_devices_storage = PersistentDict(f"gpu") # Global dictionary (shared in all processes)
                                                         # thread-safe -> it handles concurrency

def initialize_persistent_dict():
    # WARNING: call only once! Remember that Snakemake resets the WHOLE workflow a few times...

    err = False
    d = {"gpu": {},
         "mutex": ''}

    for k, v in d.items():
        try:
            _allocated_cuda_devices_storage.fetch(k)

            sys.stderr.write("WARNING: the dictionary is expected to be empty if this is the only instance running: "
                             f"found key '{k}'\n")

            err = True
        except NoSuchEntryError:
            _allocated_cuda_devices_storage.store(k, v)

    if err:
        sys.stderr.write(f"WARNING: there were values in the dictionary: if you're not running parallel instances, "
                         "this is not expected: waiting 20s before proceed")
        time.sleep(20)

def allocate_cuda_devices_teardown(debug=False):
    # WARNING: call only in teardown

    # TODO touch all output files and update in order to avoid error with other instances

    pass

# TODO fix when max_devices is 0
def allocate_cuda_visible_device(gpu_ofile_token, max_devices=0, debug=False):
    try:
        #debug_if_true(f"process stats: {os.getpid()} {os.getppid()} {os.getgid()} {os.getpgrp()} {os.getpgid(os.getppid())}", debug)
        debug_if_true(f"{gpu_ofile_token}: max_devices: {max_devices}", debug)

        def get_available_cuda_devices(_allocated_cuda_devices):
            # WARNING: call this function with mutex acquired

            available_cuda_devices = list(range(max_devices)) # All devices available by default -> we have to check which ones are not

            if "CUDA_VISIBLE_DEVICES" in os.environ:
                available_cuda_devices = list(map(lambda d: int(d), os.environ["CUDA_VISIBLE_DEVICES"].split(',')))

            allocated = dict({(d, 0) for d in available_cuda_devices})

            for gpu_token, cuda_device in _allocated_cuda_devices.items():
                allocated[cuda_device] += 1

                if allocated[cuda_device] != 1:
                    raise Exception("Bug: same device was allocated twice or more")

                try:
                    idx = available_cuda_devices.index(cuda_device)

                    del available_cuda_devices[idx]
                except ValueError as e:
                    raise Exception("Different values of 'max_devices' have been provided") from e

            return available_cuda_devices

        def clean_and_update_cuda_devices(_allocated_cuda_devices):
            # WARNING: call this function with mutex acquired

            debug_if_true(f"{gpu_ofile_token}: _allocated_cuda_devices: before cleaning: {str(_allocated_cuda_devices)}", debug)
            remove = set()

            for gpu_token, cuda_device in _allocated_cuda_devices.items():
                if path_exists(gpu_token):
                    remove.add(gpu_token)

            for r in remove:
                del _allocated_cuda_devices[r]

            _allocated_cuda_devices_storage.store("gpu", _allocated_cuda_devices)

            debug_if_true(f"{gpu_ofile_token}: _allocated_cuda_devices: after cleaning: {str(_allocated_cuda_devices)}", debug)

            _aux_allocated_cuda_devices = _allocated_cuda_devices_storage.fetch("gpu")

            if _allocated_cuda_devices != _aux_allocated_cuda_devices:
                raise Exception(f"Bug: the content should be the same but it's not: '{str(_allocated_cuda_devices)}' vs '{str(_aux_allocated_cuda_devices)}'")

            # The returned instance is already cleaned
            return _allocated_cuda_devices

        def acquire_mutex():
            _mutex = ''

            while not _mutex:
                # Try to get the mutex
                while True:
                    _mutex = _allocated_cuda_devices_storage.fetch("mutex")

                    if not _mutex:
                        # We have the mutex (not sure yet)
                        _allocated_cuda_devices_storage.store("mutex", gpu_ofile_token)

                        break
                    elif _mutex == gpu_ofile_token:
                        raise Exception("Bug: impersonation?")
                    else:
                        # Wait [1, 2) until we can get the mutex
                        time.sleep(1 + random.random())

                        debug_if_true(f"{gpu_ofile_token}: waiting to acquire the mutex", debug)

                # Wait [1, 2) and check if we still keep the mutex
                time.sleep(1 + random.random())

                debug_if_true(f"{gpu_ofile_token}: candidate to acquire the mutex", debug)

                _mutex = _allocated_cuda_devices_storage.fetch("mutex")

                if _mutex != gpu_ofile_token:
                    # We lost the mutex -> re-try after wait
                    _mutex = ""

                    debug_if_true(f"{gpu_ofile_token}: did not acquire the mutex", debug)

                    time.sleep(1 + random.random()) # Wait [1, 2)
                else:
                    debug_if_true(f"{gpu_ofile_token}: acquired the mutex", debug)

            return True

        def release_mutex():
            time.sleep(1 + random.random()) # Wait [1, 2) in order to be more robust

            _mutex = _allocated_cuda_devices_storage.fetch("mutex")

            if _mutex != gpu_ofile_token:
                raise Exception("Bug: releasing mutex which hadn't been acquired or concurrency bug")

            _allocated_cuda_devices_storage.store("mutex", '')

            debug_if_true(f"{gpu_ofile_token}: released the mutex", debug)

        acquire_mutex()

        _allocated_cuda_devices = _allocated_cuda_devices_storage.fetch("gpu")

        if gpu_ofile_token in _allocated_cuda_devices:
            raise Exception("The same token can't allocate 2 devices")

        collision = True # Fake collision
        candidate_device = 0
        warning_message = False

        while collision:
            _allocated_cuda_devices = clean_and_update_cuda_devices(_allocated_cuda_devices) # Release unused devices

            # Try to allocate a device
            available_cuda_devices = get_available_cuda_devices(_allocated_cuda_devices)

            debug_if_true(f"{gpu_ofile_token}: available_cuda_devices: {str(available_cuda_devices)}", debug)

            # Check if we were able to allocate a candidate device
            if len(available_cuda_devices) == 0:
                sys.stderr.write("WARNING: no device available (this shouldn't happen if the resources have been correctly configured): waiting...\n")

                if warning_message:
                    # Since we have acquired the mutex, this message should've been showed only once, but this is the second time...
                    raise Exception("Bug: mutex acquired but it seems that other process had access to the mutex")

                warning_message = True

            while len(available_cuda_devices) == 0:
                # Wait until some device is released
                time.sleep(10)

                _allocated_cuda_devices = clean_and_update_cuda_devices(_allocated_cuda_devices) # Release unused devices
                available_cuda_devices = get_available_cuda_devices(_allocated_cuda_devices)

            if warning_message:
                sys.stderr.write("INFO: it seems that some device has been released\n")

            # We have our candidate device -> communicate our intention about allocate the resource
            candidate_device = available_cuda_devices[0]
            _allocated_cuda_devices[gpu_ofile_token] = candidate_device

            _allocated_cuda_devices_storage.store("gpu", _allocated_cuda_devices) # Mutex is acquired, so there shouldn't be any problem

            time.sleep(1 + random.random()) # Wait [1, 2)s before proceed in order to check if there were collisions allocating the same device (there shouldn't)

            # Check if there were collisions
            _allocated_cuda_devices = _allocated_cuda_devices_storage.fetch("gpu")
            candidate_device_count = 0
            check_collision = False

            for gpu_token, cuda_device in _allocated_cuda_devices.items():
                if cuda_device == candidate_device:
                    # It might be our previous allocate request or a collision

                    if gpu_token != gpu_ofile_token:
                        check_collision = True

                    candidate_device_count += 1

            if candidate_device_count > 1 or check_collision:
                # Collision -> deallocate device and wait random time in order to avoid further collisions and re-try
                sys.stderr.write("WARNING: collision detected while mutex acquired (likely a bug: please, report): trying to solve\n")

                del _allocated_cuda_devices[gpu_ofile_token]

                # clean_and_update_cuda_devices() will update the result

                time.wait(random.randrange(2) + random.random()) # Wait [0, 2)
            else:
                # Device allocated successfully
                collision = False

        # Update allocated device
        clean_and_update_cuda_devices(_allocated_cuda_devices)

        debug_if_true(f"{gpu_ofile_token}: device: {candidate_device}", debug)

        # Release mutex
        release_mutex()

        return candidate_device
    except Exception as e:
        debug_if_true(f"exception ({str(sys.exc_info()[0])}): {str(e)}", True)
        debug_if_true(traceback.format_exc(), True)

        raise
