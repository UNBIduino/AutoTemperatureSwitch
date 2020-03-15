import bluetooth
import time

def switch():
    name = "HC-06"
    passkey = "0000"
    addr = "XX:XX:XX:XX:XX:XX"
    port = 1
    sock = bluetooth.BluetoothSocket(bluetooth.RFCOMM)
    sock.connect((addr, port))
    print('Connected')
    sock.settimeout(5.0)

    sock.send("70.00,71.00")  # battery full
    mystr=""
    try:
        while True:
            r = sock.recv(255)
            strval:str=r.decode("utf-8")
            print(strval,end="")
            if not r:
                break
    except:
        pass

try:
    switch()
except:
    time.sleep(10)
    try:
        switch()
    except:
        time.sleep(10)
        try:
            switch()
        except:
            pass
