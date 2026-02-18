import pandas as pd
import numpy as np
import joblib
import warnings
import os
import sys
from pythonosc import osc_server, udp_client
from pythonosc.dispatcher import Dispatcher

def resource_path(relative_path):
    try:
        base_path = sys._MEIPASS
    except Exception:
        base_path = os.path.abspath(".")
    return os.path.join(base_path, relative_path)

MODEL_PATH = resource_path("gesture_model.pkl")
OSC_IN = 9000      
OSC_OUT = 9001     

FRAME_SKIP = 2
SMOOTH_WINDOW = 8

warnings.filterwarnings("ignore")
try:
    model = joblib.load(MODEL_PATH)
    sys.stdout.write(f"MODEL LOADED: {MODEL_PATH}\n")
except Exception as e:
    sys.stdout.write(f"ERROR: Model not found. {e}\n")
    sys.exit(1)

client = udp_client.SimpleUDPClient("127.0.0.1", OSC_OUT)
prev_frame = None
history = []
frame_count = 0

def handle_data(addr, *args):
    global prev_frame, history, frame_count
    
    frame_count += 1
    if len(args) != 8: return
    
    if frame_count % FRAME_SKIP != 0: return 

    curr = np.array(args)
    if prev_frame is not None:
        try:
            l_coords, r_coords = curr[0:3], curr[4:7]
            pl_coords, pr_coords = prev_frame[0:3], prev_frame[4:7]
            
            l_speed = np.linalg.norm(l_coords - pl_coords)
            r_speed = np.linalg.norm(r_coords - pr_coords)

            threshold = 0.005 
            if l_speed < threshold: l_speed = 0.0
            if r_speed < threshold: r_speed = 0.0

            l_jitter = abs(l_speed - getattr(handle_data, 'last_ls', 0))
            r_jitter = abs(r_speed - getattr(handle_data, 'last_rs', 0))
            handle_data.last_ls, handle_data.last_rs = l_speed, r_speed

            features = pd.DataFrame([[l_speed, l_jitter, r_speed, r_jitter]], 
                                    columns=['l_speed', 'l_jitter', 'r_speed', 'r_jitter'])
            
            probs = model.predict_proba(features)[0] 
            
            raw_pred = 1 if probs[1] > 0.5 else 0
            history.append(raw_pred)
            if len(history) > SMOOTH_WINDOW: history.pop(0)
            
            avg = sum(history) / len(history)
            final_style = 1 if avg > 0.5 else 0
            confidence = probs[final_style] * 100 

            client.send_message("/ai/style", float(final_style))
            client.send_message("/ai/confidence", float(confidence))
            
            lbl = "AGGRESSIVE" if final_style == 1 else "GENTLE"
            sys.stdout.write(f"\r {lbl:10} | {confidence:.0f}%   ")
            sys.stdout.flush()

        except Exception:
            pass

    prev_frame = curr

dispatcher = Dispatcher()
dispatcher.map("/gesture/raw", handle_data)
server = osc_server.ThreadingOSCUDPServer(("127.0.0.1", OSC_IN), dispatcher)
sys.stdout.write(f"🚀 AI ACTIVE (Port {OSC_IN})...\n")
server.serve_forever()