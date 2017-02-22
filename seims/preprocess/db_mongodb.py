import sys

from pymongo import MongoClient
from pymongo.errors import ConnectionFailure


class ConnectMongoDB(object):
    """Connect to MongoDB, and close when finished."""

    def __init__(self, ip, port):
        try:
            self.conn = MongoClient(ip, port)
        except ConnectionFailure:
            sys.stderr.write("Could not connect to MongoDB: %s" % ConnectionFailure.message)
            sys.exit(1)

    def get_conn(self):
        return self.conn

    def close(self):
        self.conn.close()
