
import subprocess


class ExternalTextProcessor(object):

    def __init__(self, cmd):
        self.cmd = cmd

    def process(self, input_text):

        proc = subprocess.Popen(self.cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        outs, errs = proc.communicate(input=bytes(input_text, encoding='utf-8'))

        return outs.decode('utf-8')
