import pandas as pd
import numpy as np
import joblib
import os
from sklearn.model_selection import train_test_split
from sklearn.ensemble import RandomForestClassifier
from sklearn.metrics import accuracy_score

DATA_ROOT = "./Recordings/"
MODEL_FILE = "gesture_model.pkl"

INTENT_MAP = {
    0: ["Gentle"], 
    1: ["Aggressive"]
}


def get_label_from_filename(filename):
    for label, keywords in INTENT_MAP.items():
        if any(k in filename for k in keywords):
            return label
    return None 

def process_file(filepath):
    try:
        df = pd.read_csv(filepath)
        if len(df) < 10: return None
        
        label = get_label_from_filename(os.path.basename(filepath))
        if label is None: return None

 
        features = pd.DataFrame()
        
        for hand, start_col in [('L', 1), ('R', 5)]: 
            coords = df.iloc[:, start_col:start_col+3]
            
            speed = np.sqrt(coords.diff().pow(2).sum(axis=1))
            
            jitter = speed.diff().abs()
            
            prefix = hand.lower() 
            features[f'{prefix}_speed'] = speed
            features[f'{prefix}_jitter'] = jitter
        
        features = features.dropna()
        features['Target'] = label
        return features

    except Exception as e:
        print(f"Error processing {filepath}: {e}")
        return None

print("TRAINING PATTERNS")
all_data = []

for root, _, files in os.walk(DATA_ROOT):
    for file in files:
        if file.endswith(".csv"):
            processed = process_file(os.path.join(root, file))
            if processed is not None:
                all_data.append(processed)

if not all_data:
    print("No training data found. Run record_data.py first")
    exit()

dataset = pd.concat(all_data)
print(f"Loaded {len(all_data)} files. Training Rows: {len(dataset)}")

X = dataset.drop(columns=['Target'])
y = dataset['Target']

X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)

model = RandomForestClassifier(n_estimators=200, max_depth=20, random_state=42)
model.fit(X_train, y_train)

acc = accuracy_score(y_test, model.predict(X_test))
print(f"\nMODEL ACCURACY: {acc*100:.2f}%")
print(f"Saved to {MODEL_FILE}")

joblib.dump(model, MODEL_FILE)