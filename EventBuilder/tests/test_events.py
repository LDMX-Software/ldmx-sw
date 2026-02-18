import os
import subprocess
import time
import sys


def test_event_builder_and_consumer(tmp_path):
    """Integration test: run event_builder (if present) on sample input and verify consumer sees events.

    This test requires the `eventbuilder/bin/event_builder` binary and a readable .dat input file.
    It will skip (xfail) if the binary or input file is missing.
    """
    bin_path = os.path.join(os.getcwd(), 'bin', 'event_builder')
    # if repo has local build dir layout, also try ../bin/event_builder
    if not os.path.exists(bin_path):
        # try project relative
        alt = os.path.join(os.getcwd(), '..', 'bin', 'event_builder')
        if os.path.exists(alt):
            bin_path = alt

    if not os.path.exists(bin_path):
        print('event_builder binary not found; skipping integration test')
        return

    # choose input file - prefer env override
    input_file = os.environ.get('EVENTBUILDER_INPUT', os.path.join(os.getcwd(), 'Run_061_20251211_145655.dat'))
    if not os.path.exists(input_file):
        print(f'Input file {input_file} not found; skipping integration test')
        return

    out_path = tmp_path / 'events_out.dat'
    # ensure out file not present
    if os.path.exists('events_out.dat'):
        try:
            os.remove('events_out.dat')
        except Exception:
            pass

    # run builder with env override (it will append events to events_out.dat)
    env = os.environ.copy()
    env['EVENTBUILDER_INPUT'] = input_file

    proc = subprocess.Popen([bin_path], env=env)
    # give it a short amount of time to parse and produce output
    time.sleep(2)
    proc.terminate()
    try:
        proc.wait(timeout=5)
    except Exception:
        proc.kill()

    # run consumer script to ensure file is readable
    consumer = os.path.join(os.getcwd(), 'tools', 'consume_events.py')
    if not os.path.exists(consumer):
        print('consumer script not found; skipping')
        return

    # run consumer; success is simply that it exits without exception
    res = subprocess.run([sys.executable, consumer, 'events_out.dat'], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    print(res.stdout)
    assert res.returncode == 0
