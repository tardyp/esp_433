import requests

s = requests.Session()

def read(r):
    return int(s.get("http://192.168.2.241/r?r={}".format(r)).content)

def write(r, v):
    s.get("http://192.168.2.241/w?r={}&v={}".format(r, v)).content

# for i in range(47):
#     print(hex(i), hex(read(0xC0 + i)))

print(s.post("http://192.168.2.241/send", data="P"*100, headers={'Content-Type': '433/data'}).content)