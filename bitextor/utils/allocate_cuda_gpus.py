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

import os
import sys
import time
import random
import traceback
import logging

import bitextor.utils.mutex as mutex
from bitextor.utils.common import (
    debug_if_true,
    path_exists,
)

LOGGER = logging.getLogger("bitextor")

def get_available_cuda_devices(allocated_cuda_devices, max_devices=0, update_allocated_devices=True):
    # WARNING: call this function with mutex acquired

    available_cuda_devices = list(range(max_devices)) # All devices available by default -> we have to check which ones are not

    if "CUDA_VISIBLE_DEVICES" in os.environ:
        available_cuda_devices = list(map(lambda d: int(d), os.environ["CUDA_VISIBLE_DEVICES"].split(',')))

    if update_allocated_devices:
        allocated = dict({(d, 0) for d in available_cuda_devices})

        for gpu_token, cuda_device in allocated_cuda_devices.items():
            allocated[cuda_device] += 1

            if allocated[cuda_device] != 1:
                raise Exception("Bug: same device was allocated twice or more")

            try:
                idx = available_cuda_devices.index(cuda_device)

                del available_cuda_devices[idx]
            except ValueError as e:
                raise Exception("Different values of 'max_devices' have been provided") from e

    return available_cuda_devices

def clean_and_update_cuda_devices(persistent_storage, allocated_cuda_devices, gpu_ofile_token, debug=False):
    # WARNING: call this function with mutex acquired

    debug_if_true(f"{gpu_ofile_token}: allocated_cuda_devices: before cleaning: {str(allocated_cuda_devices)}", debug)
    remove = set()

    for gpu_token, cuda_device in allocated_cuda_devices.items():
        if path_exists(gpu_token):
            remove.add(gpu_token)

    for r in remove:
        del allocated_cuda_devices[r]

    persistent_storage.store("gpu", allocated_cuda_devices)

    debug_if_true(f"{gpu_ofile_token}: allocated_cuda_devices: after cleaning: {str(allocated_cuda_devices)}", debug)

    aux_allocated_cuda_devices = persistent_storage.fetch("gpu")

    if allocated_cuda_devices != aux_allocated_cuda_devices:
        raise Exception("Bug: the content should be the same but it's not: "
                        f"'{str(allocated_cuda_devices)}' vs '{str(aux_allocated_cuda_devices)}'")

    # The returned instance is already cleaned
    return allocated_cuda_devices

def allocate_cuda_visible_device(persistent_storage, gpu_ofile_token, max_devices=0, debug=False):
    _available_cuda_devices = get_available_cuda_devices(None, max_devices=max_devices, update_allocated_devices=False)

    if len(_available_cuda_devices) == 0:
        # GPU not allocated (i.e. not handled) or CPU execution
        return -1

    try:
        #debug_if_true(f"process stats: {os.getpid()} {os.getppid()} {os.getgid()} {os.getpgrp()} {os.getpgid(os.getppid())}", debug)
        debug_if_true(f"{gpu_ofile_token}: max_devices: {max_devices}", debug)

        mutex.acquire_mutex(persistent_storage, gpu_ofile_token, debug=debug)

        allocated_cuda_devices = persistent_storage.fetch("gpu")

        if gpu_ofile_token in allocated_cuda_devices:
            raise Exception("The same token can't allocate 2 devices")

        collision = True # Fake collision
        candidate_device = 0
        warning_message = False

        while collision:
            allocated_cuda_devices = clean_and_update_cuda_devices(persistent_storage, allocated_cuda_devices,
                                                                   gpu_ofile_token, debug=debug) # Release unused devices

            # Try to allocate a device
            available_cuda_devices = get_available_cuda_devices(allocated_cuda_devices, max_devices=max_devices)

            debug_if_true(f"{gpu_ofile_token}: available_cuda_devices: {str(available_cuda_devices)}", debug)

            # Check if we were able to allocate a candidate device
            if len(available_cuda_devices) == 0:
                LOGGER.warning("No device available (this shouldn't happen if the resources have been correctly configured): waiting...")

                if warning_message:
                    # Since we have acquired the mutex, this message should've been showed only once, but this is the second time...
                    raise Exception("Bug: mutex acquired but it seems that other process had access to the mutex")

                warning_message = True

            while len(available_cuda_devices) == 0:
                # Wait until some device is released
                time.sleep(10)

                allocated_cuda_devices = clean_and_update_cuda_devices(persistent_storage, allocated_cuda_devices,
                                                                       gpu_ofile_token, debug=debug) # Release unused devices
                available_cuda_devices = get_available_cuda_devices(allocated_cuda_devices, max_devices=max_devices)

            if warning_message:
                LOGGER.info("It seems that some device has been released")

            # We have our candidate device -> communicate our intention about allocate the resource
            candidate_device = available_cuda_devices[0]
            allocated_cuda_devices[gpu_ofile_token] = candidate_device

            persistent_storage.store("gpu", allocated_cuda_devices) # Mutex is acquired, so there shouldn't be any problem

            time.sleep(1 + random.random()) # Wait [1, 2)s before proceed in order to check if there were collisions
                                            #  allocating the same device (there shouldn't)

            # Check if there were collisions
            allocated_cuda_devices = persistent_storage.fetch("gpu")
            candidate_device_count = 0
            check_collision = False

            for gpu_token, cuda_device in allocated_cuda_devices.items():
                if cuda_device == candidate_device:
                    # It might be our previous allocate request or a collision

                    if gpu_token != gpu_ofile_token:
                        check_collision = True

                    candidate_device_count += 1

            if candidate_device_count > 1 or check_collision:
                # Collision -> deallocate device and wait random time in order to avoid further collisions and re-try
                LOGGER.warning("Collision detected while mutex acquired (likely a bug: please, report): trying to solve")

                del allocated_cuda_devices[gpu_ofile_token]

                # clean_and_update_cuda_devices() will update the result

                time.wait(random.randrange(2) + random.random()) # Wait [0, 2)
            else:
                # Device allocated successfully
                collision = False

        # Update allocated device
        clean_and_update_cuda_devices(persistent_storage, allocated_cuda_devices, gpu_ofile_token, debug=debug)

        debug_if_true(f"{gpu_ofile_token}: device: {candidate_device}", debug)

        # Release mutex
        mutex.release_mutex(persistent_storage, gpu_ofile_token, debug=debug)

        return candidate_device
    except Exception as e:
        debug_if_true(f"exception ({str(sys.exc_info()[0])}): {str(e)}", True)
        debug_if_true(traceback.format_exc(), True)

        raise

def allocate_cuda_devices_teardown(persistent_storage, ofile_token, debug=False):
    # WARNING: call only in teardown

    mutex.acquire_mutex(persistent_storage, ofile_token, debug=debug)

    # Touch all GPU token files
    allocated_cuda_devices = persistent_storage.fetch("gpu")

    for gpu_token, cuda_device in allocated_cuda_devices.items():
        if not path_exists(gpu_token):
            # Touch token file
            with open(gpu_token, 'w'):
                pass

    # Update persistent storage in order to avoid errors with other instances
    clean_and_update_cuda_devices(persistent_storage, allocated_cuda_devices, ofile_token, debug=debug)

    mutex.release_mutex(persistent_storage, ofile_token, debug=debug)
