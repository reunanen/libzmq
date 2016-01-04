/*
    Copyright (c) 2007-2015 Contributors as noted in the AUTHORS file

    This file is part of libzmq, the ZeroMQ core engine in C++.

    libzmq is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License (LGPL) as published
    by the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    As a special exception, the Contributors give you permission to link
    this library with independent modules to produce an executable,
    regardless of the license terms of these independent modules, and to
    copy and distribute the resulting executable under terms of your choice,
    provided that you also meet, for each linked independent module, the
    terms and conditions of the license of that module. An independent
    module is a module which is not derived from or based on this library.
    If you modify this library, you must extend this exception to your
    version of the library.

    libzmq is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
    License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "testutil.hpp"
#include "../include/zmq_utils.h"

#if defined ZMQ_HAVE_WINDOWS
#include "windows.hpp"
#else
#include <unistd.h>
#endif

void _sleep (long timeout_)
{
#if defined ZMQ_HAVE_WINDOWS
    Sleep (timeout_ > 0 ? timeout_ : INFINITE);
#elif defined ZMQ_HAVE_ANDROID
    usleep (timeout_ * 1000);
#else
    usleep (timeout_ * 1000);
#endif
}

void handler (int timer_id, void* arg)
{
    *((bool *)arg) = true;
}

int main (void)
{
    setup_test_environment ();

    void* timers = zmq_timers_new ();
    assert (timers);

    bool timer_invoked = false;

    int timer_id = zmq_timers_add (timers, 100, handler, &timer_invoked);
    assert (timer_id);

    //  Timer should be invoked yet
    int rc = zmq_timers_execute (timers);
    assert (rc == 0);
    assert (!timer_invoked);

    //  Wait half the time and check again
    _sleep (zmq_timers_timeout (timers) / 2);
    rc = zmq_timers_execute (timers);
    assert (rc == 0);
    assert (!timer_invoked);

    // Wait until the end
    _sleep (zmq_timers_timeout (timers));
    rc = zmq_timers_execute (timers);
    assert (rc == 0);
    assert (timer_invoked);
    timer_invoked = false;

    //  Wait half the time and check again
    long timeout = zmq_timers_timeout (timers);
    _sleep (timeout / 2);
    rc = zmq_timers_execute (timers);
    assert (rc == 0);
    assert (!timer_invoked);

    // Reset timer and wait half of the time left
    rc = zmq_timers_reset (timers, timer_id);
    _sleep (timeout / 2);
    rc = zmq_timers_execute (timers);
    assert (rc == 0);
    assert (!timer_invoked);

    // Wait until the end
    _sleep (zmq_timers_timeout (timers));
    rc = zmq_timers_execute (timers);
    assert (rc == 0);
    assert (timer_invoked);
    timer_invoked = false;

    // reschedule
    zmq_timers_set_interval (timers, timer_id, 50);
    _sleep (51);
    rc = zmq_timers_execute (timers);
    assert (rc == 0);
    assert (timer_invoked);
    timer_invoked = false;

    // cancel timer
    timeout = zmq_timers_timeout (timers);
    zmq_timers_cancel (timers, timer_id);
    _sleep (timeout * 2);
    rc = zmq_timers_execute (timers);
    assert (rc == 0);
    assert (!timer_invoked);

    rc = zmq_timers_destroy (&timers);
    assert (rc == 0);

    return 0;
}
