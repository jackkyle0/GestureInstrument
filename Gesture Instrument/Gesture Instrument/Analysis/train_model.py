import pandas as pd
import numpy as np
import os
import torch
import torch.nn as nn
import torch.optim as optim
from torch.utils.data import DataLoader, TensorDataset
from sklearn.model_selection import train_test_split
import onnx

DATA_ROOT = "./Recordings/"
MODEL_FILE = "gesture_model.onnx"

def get_label_from_filepath(filepath):
    path_lower = filepath.lower()
    if "gentle" in path_lower: return 0
    elif "aggressive" in path_lower: return 1
    return None 

def process_file(filepath):
    try:
        df = pd.read_csv(filepath)
        if len(df) < 10: return None
        
        label = get_label_from_filepath(filepath)
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
        
        # Only keep frames where at at least one hand is moving!
        active_movement = (features['l_speed'] > 2.0) | (features['r_speed'] > 2.0)
        features = features[active_movement]
        
        if features.empty: return None

        features['Target'] = label
        return features
    except Exception as e:
        return None

print("LOADING TRAINING PATTERNS...")
all_data = []
for root, _, files in os.walk(DATA_ROOT):
    for file in files:
        if file.endswith(".csv"):
            processed = process_file(os.path.join(root, file))
            if processed is not None:
                all_data.append(processed)

if not all_data:
    print("No training data found. Run record_data.py first")
    print("No training data found.")
    exit()

dataset = pd.concat(all_data)
print(f"Loaded {len(all_data)} files. Total Frames: {len(dataset)}")

X = dataset.drop(columns=['Target']).values
y = dataset['Target'].values

gentle_count = np.sum(y == 0)
aggro_count = np.sum(y == 1)
print(f"Gentle Frames: {gentle_count} | Aggressive Frames: {aggro_count}")

X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)

X_train_t = torch.tensor(X_train, dtype=torch.float32)
y_train_t = torch.tensor(y_train, dtype=torch.long)
X_test_t = torch.tensor(X_test, dtype=torch.float32)
y_test_t = torch.tensor(y_test, dtype=torch.long)

train_dataset = TensorDataset(X_train_t, y_train_t)
train_loader = DataLoader(train_dataset, batch_size=64, shuffle=True, drop_last=True)

class GestureNN(nn.Module):
    def __init__(self):
        super(GestureNN, self).__init__()
        self.network = nn.Sequential(
            nn.Linear(4, 24),  
            nn.ReLU(),
            nn.Linear(24, 12),
            nn.ReLU(),
            nn.Linear(12, 2)
        )

    def forward(self, x):
        return self.network(x)

model = GestureNN()

class_counts = np.bincount(y_train)
weights = len(y_train) / (2.0 * class_counts)
class_weights = torch.tensor(weights, dtype=torch.float32)

criterion = nn.CrossEntropyLoss(weight=class_weights)
optimizer = optim.Adam(model.parameters(), lr=0.001) 

print("\nTRAINING NEURAL NETWORK...")
epochs = 30
for epoch in range(epochs):
    model.train()
    total_loss = 0
    for batch_X, batch_y in train_loader:
        optimizer.zero_grad()
        predictions = model(batch_X)
        loss = criterion(predictions, batch_y)
        loss.backward()
        optimizer.step()
        total_loss += loss.item()
    
    if (epoch+1) % 5 == 0:
        print(f"Epoch {epoch+1}/{epochs} | Loss: {total_loss/len(train_loader):.4f}")

model.eval()
with torch.no_grad():
    test_preds = model(X_test_t)
    predicted_classes = torch.argmax(test_preds, dim=1)
    acc = (predicted_classes == y_test_t).float().mean().item()

print(f"\nMODEL ACCURACY: {acc*100:.2f}%")

print(f"Exporting to {MODEL_FILE}...")
dummy_input = torch.randn(1, 4)

torch.onnx.export(
    model, 
    dummy_input, 
    MODEL_FILE, 
    export_params=True, 
    input_names=['input'], 
    output_names=['output'],
    dynamic_axes={'input': {0: 'batch_size'}, 'output': {0: 'batch_size'}}
)

print("Packaging...")
try:
    onnx_model = onnx.load(MODEL_FILE, load_external_data=True)
    onnx.save_model(onnx_model, "gesture_model_packaged.onnx", save_as_external_data=False)
    print("SUCCESS!")
except Exception as e:
    print(f"Packaging failed: {e}")