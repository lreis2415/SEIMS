# coding=utf-8
import json
import os
import re
import shlex
import subprocess
import sys
import io
from pathlib import Path

if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

from http.server import HTTPServer, BaseHTTPRequestHandler
from calibration.config import CaliConfig
from configparser import ConfigParser


# https://blog.csdn.net/Datapad/article/details/117284675

class SEIMSServer(BaseHTTPRequestHandler):
    def do_GET(self):
        data = {"Method:": self.command,
                "Path:": self.path}
        print(data)
        self.send_response(200)
        self.send_header('Content-type', 'application/json')
        self.end_headers()
        self.wfile.write(json.dumps(data).encode())

    def do_POST(self):

        data = {
            "Method:": self.command,
            "Path:": self.path,
            "Post Data": 'req',
            "code": 200,
            "data": {
                'nse': 0.5,
                'cali_dir': 'aaa',
            },
        }
        self.send_response(200)
        self.send_header('Content-type', 'application/json')
        self.end_headers()
        self.wfile.write(json.dumps(data).encode())
        return

        length = int(self.headers['Content-Length'])
        req_raw = self.rfile.read(length)
        req = str(req_raw.decode('utf-8'))
        print(req)
        # command = r"python -m scoop -n 4 calibration/main_nsga2.py -ini C:/src/SEIMS/data/youwuzhen/model_configs_conceptual-completed/calibration.ini"
        command = shlex.split(req)
        process = subprocess.Popen(
            command,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            shell=True,
            cwd=Path(os.path.abspath(os.path.dirname(__file__))).parent.as_posix(),
        )
        out, err = process.communicate()
        return_code = process.returncode
        # parse ini
        ini_path = Path(command[-1])
        parser = ConfigParser()
        parser.read(ini_path)
        cfg = CaliConfig(parser, method='nsga2')
        logbook = Path(cfg.opt.out_dir, 'logbook.txt')
        with io.open(logbook.as_posix(), 'r', encoding='utf-8') as f:
            lines = f.readlines()
            last_line = lines[-1]
            max_str = last_line.split('\t')[4]
            max_str = re.sub(r'\[|\]', '', max_str)
            nse = float(max_str.split()[0])

        data = {
            "Method:": self.command,
            "Path:": self.path,
            "Post Data": req,
            "code": 200,
            "data": {
                'nse': nse,
                'cali_dir': cfg.opt.out_dir,
            },
        }
        self.send_response(200)
        self.send_header('Content-type', 'application/json')
        self.end_headers()
        self.wfile.write(json.dumps(data).encode())


def start_server():
    host = ('localhost', 8415)
    server = HTTPServer(host, SEIMSServer)
    print("Starting server, listen at: %s:%s" % host)
    server.serve_forever()


def cmd_test():
    # command = [
    #     'python',
    #     '-m scoop',
    #     '-n 4',
    #     'calibration/main_nsga2.py',
    #     '-ini', r'C:/src/SEIMS/data/youwuzhen/model_configs_conceptual-completed/calibration.ini',
    # ]
    command = r"python -m scoop -n 4 calibration/main_nsga2.py -ini C:/src/SEIMS/data/youwuzhen/model_configs_conceptual-completed/calibration.ini"
    command = shlex.split(command)
    # command = [
    #     'python',
    #     '-m scoop',
    #     'calibration/main_nsga2.py',
    #     '-ini', r'C:/src/SEIMS/data/youwuzhen/model_configs_conceptual-completed/calibration.ini',
    # ]
    # command = [
    #     'python',
    #     'calibration/main_nsga2.py',
    #     '-ini', r'C:/src/SEIMS/data/youwuzhen/model_configs_conceptual-completed/calibration.ini',
    # ]
    process = subprocess.Popen(
        # executable=sys.executable,
        args=command,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        cwd=r'C:\src\SEIMS\seims',
        shell=True,
        # env={'PYTHONPATH': r'C:\src\SEIMS\seims'},

    )

    out, err = process.communicate()
    print 'output:'
    print out
    print 'error:'
    print err


if __name__ == '__main__':
    start_server()
    print("start server success...")
    # cmd_test()
