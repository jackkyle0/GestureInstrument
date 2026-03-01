import pandas as pd
from pythonosc import osc_server
from pythonosc.dispatcher import Dispatcher
import threading
import time
import os
import datetime

USER_ID = "User07"
SAVE_PATH = f"./Recordings/{USER_ID}/"

SESSIONS = [
    ("Gentle_Static_Neutral", 15),
    ("Gentle_Static_FlatHand", 5), 
    ("Gentle_Static_Fist", 5), 
    
    ("Gentle_Linear_X_Pure", 10), 
    ("Aggressive_Linear_X_Pure", 10), 
    ("Gentle_Linear_Y_Pure", 10), 
    ("Aggressive_Linear_Y_Pure", 10),
    ("Gentle_Linear_Z_Pure", 10),
    ("Aggressive_Linear_Z_Pure", 10),
     
    ("Style_Gentle", 20), 
    ("Style_Aggressive", 20),
    
    ("Gentle_Combo_XY_Circle", 10), 
    ("Aggressive_Combo_XY_Circle", 10), 
    ("Gentle_Combo_XZ_FloorCircle", 10), 
    ("Aggressive_Combo_XZ_FloorCircle", 10),
    ("Gentle_Combo_YZ_DiagonalThrusts", 10), 
    ("Aggressive_Combo_YZ_DiagonalThrusts", 10),
    ("Gentle_Combo_Spiral", 10),
    ("Aggressive_Combo_Spiral", 10),
    ("Aggressive_FastJitter", 10),
    
    ("Bounds_MaxReach", 15),
    ("Edge_CrossHands", 15), 
    ("Gentle_Edge_ExitField", 20),

    
]

if not os.path.exists(SAVE_PATH): os.makedirs(SAVE_PATH)

class DataRecorder:
    def __init__(self):
        self.data_buffer = []
        self.is_recording = False
        self.current_session = ""

    def handle_osc(self, address, *args):
        if self.is_recording:
            self.data_buffer.append([time.time()] + list(args))

    def start(self, name):
        self.data_buffer = []
        self.current_session = name
        self.is_recording = True

    def stop(self):
        self.is_recording = False
        if not self.data_buffer: return
        
        filename = f"{USER_ID}_{self.current_session}_{datetime.datetime.now().strftime('%H%M%S')}.csv"
        df = pd.DataFrame(self.data_buffer)
        df.to_csv(os.path.join(SAVE_PATH, filename), index=False)
        print(f"SAVED: {filename} ({len(df)} frames)")

recorder = DataRecorder()
dispatcher = Dispatcher()
dispatcher.map("/gesture/raw", recorder.handle_osc)



def run_server():
    server = osc_server.ThreadingOSCUDPServer(("127.0.0.1", 9000), dispatcher)
    server.serve_forever()

threading.Thread(target=run_server, daemon=True).start()

print(f"RECORDING DATA FOR: {USER_ID}")
print(f"Total Sessions: {len(SESSIONS)}")

for idx, (name, duration) in enumerate(SESSIONS, 1):
    print(f"\n[{idx}/{len(SESSIONS)}] NEXT: {name.upper()}")
    input("    Press ENTER to start 3s countdown...")
    
    for i in range(3, 0, -1):
        print(f"    Starting in {i}...", end="\r")
        time.sleep(1)
    
    recorder.start(name)
    print(f"RECORDING ({duration}s)...           ")
    
    for i in range(duration):
        progress = "=" * i + "-" * (duration - i)
        print(f"    [{progress}] {duration-i}s left", end="\r")
        time.sleep(1)
        
    recorder.stop()

print(f"\n\nTESTING COMPLETE FOR {USER_ID}.")