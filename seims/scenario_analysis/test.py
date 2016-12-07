
from pymongo import MongoClient

# cmdStr = "%s %s %d %d %s %d %d" % (model_Exe, model_Workdir, threadsNum, layeringMethod, HOSTNAME, PORT, 1)
# process = Popen(cmdStr, shell=True, stdout=PIPE)
# while(process.stdout.readline() != ""):
#     # line = process.stdout.readline().split("\n")
#     # if(line[0] != ""):
#     #     print line[0]
#     continue
# process.wait()
# if process.returncode == 0:
#     print "OK"

client = MongoClient('192.168.6.55', 27017)
db = client['BMP_Scenario_dianbu2_30m_longterm']
collection = db.BMP_SCENARIOS
# for i in range(10, 500):
#     collection.remove({'ID': i})
for i in collection.find({"ID":{"$gt":100}}):
    #print i["ID"]
    collection.remove({"ID":i["ID"]})
