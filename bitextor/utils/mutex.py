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

# Global mutex mechanism through persistent files

import time
import random

from bitextor.utils.common import debug_if_true

def acquire_mutex(persistent_dict, ofile_token, debug=False):
    _mutex = ''

    while not _mutex:
        # Try to get the mutex
        while True:
            _mutex = persistent_dict.fetch("mutex")

            if not _mutex:
                # We have the mutex (not sure yet)
                persistent_dict.store("mutex", ofile_token)

                break
            elif _mutex == ofile_token:
                raise Exception("Mutex token impersonation: have you provided the same token across "
                                f"instances (if not, it might be a bug)? Token: {ofile_token}")
            else:
                # Wait [1, 2) until we can get the mutex
                time.sleep(1 + random.random())

                debug_if_true(f"{ofile_token}: waiting to acquire the mutex", debug)

        # Wait [1, 2) and check if we still keep the mutex
        time.sleep(1 + random.random())

        debug_if_true(f"{ofile_token}: candidate to acquire the mutex", debug)

        _mutex = persistent_dict.fetch("mutex")

        if _mutex != ofile_token:
            # We lost the mutex -> re-try after wait
            _mutex = ""

            debug_if_true(f"{ofile_token}: did not acquire the mutex", debug)

            time.sleep(1 + random.random()) # Wait [1, 2)
        else:
            debug_if_true(f"{ofile_token}: acquired the mutex", debug)

    return True

def release_mutex(persistent_dict, ofile_token, debug=False):
    time.sleep(1 + random.random()) # Wait [1, 2) in order to be more robust

    _mutex = persistent_dict.fetch("mutex")

    if _mutex != ofile_token:
        raise Exception("Bug: releasing mutex which hadn't been acquired or concurrency bug")

    persistent_dict.store("mutex", '')

    debug_if_true(f"{ofile_token}: released the mutex", debug)
