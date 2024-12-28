# import json
# import requests

# data = 
# url ='http://0.0.0.0:8000/predict/'
# data = json.dumps(data)
# response = requests.post(url, data)
# print(response.json())

# import requests
# import json

# # URL Firebase Realtime Database
# FIREBASE_URL = "https://porjectptc-default-rtdb.asia-southeast1.firebasedatabase.app"
# API_KEY = "AIzaSyCsPcnfpFLBcQkL65EkUR5d1AgSyPdq6RE"

# # URL Docker Model API
# PREDICTION_URL = "http://localhost:8000/predict/"  # Endpoint untuk model prediksi

# # Fungsi untuk mengambil data dari Firebase
# def fetch_firebase_data(path):
#     try:
#         response = requests.get(f"{FIREBASE_URL}{path}?auth={API_KEY}")
#         if response.status_code == 200:
#             return response.json()
#         else:
#             print(f"Failed to fetch data from {path}: {response.status_code}, {response.text}")
#             return None
#     except requests.exceptions.RequestException as e:
#         print(f"Request to Firebase failed: {e}")
#         return None

# # Fungsi untuk mempersiapkan data untuk prediksi
# def prepare_data(firebase_data):
#     if not firebase_data:
#         return None

#     # Contoh transformasi data
#     try:
#         fill_level = firebase_data.get("Fullness", 0)
#         duration_to_full = firebase_data.get("DurationToFull", 0)
#         crowded_location = 1 if fill_level > 80 else 0  # Misalnya, lokasi ramai jika tingkat penuh > 80%
#         weekly_frequency = 3  # Contoh default frekuensi pengambilan

#         return {
#             "fill_level": fill_level,
#             "duration_to_full": int(round(duration_to_full / 3600)),  # Konversi dari detik ke jam
#             "crowded_location": crowded_location,
#             "weekly_frequency": weekly_frequency
#         }
#     except Exception as e:
#         print(f"Error preparing data: {e}")
#         return None

# # Fungsi untuk mengirim data ke model prediksi
# def send_to_prediction_api(data):
#     print(data)
#     if not data:
#         print("No data to send to prediction API.")
#         return None

#     try:
#         headers = {"Content-Type": "application/json"}
#         response = requests.post(PREDICTION_URL, data=json.dumps(data), headers=headers)

#         if response.status_code == 200:
#             return response.json()
#         else:
#             print(f"Prediction API error: {response.status_code}, {response.text}")
#             return None
#     except requests.exceptions.RequestException as e:
#         print(f"Request to Prediction API failed: {e}")
#         return None

# # Ambil data dari Firebase untuk TITIK1 dan TITIK2
# titik1_data = fetch_firebase_data("/TITIK1.json")
# titik2_data = fetch_firebase_data("/TITIK2.json")

# # Persiapkan data untuk prediksi
# titik1_input = prepare_data(titik1_data)
# titik2_input = prepare_data(titik2_data)

# # Kirim data ke API prediksi
# if titik1_input:
#     print("Sending TITIK1 data for prediction...")
#     titik1_prediction = send_to_prediction_api(titik1_input)
#     print("Prediction result for TITIK1:", titik1_prediction)

# if titik2_input:
#     print("Sending TITIK2 data for prediction...")
#     titik2_prediction = send_to_prediction_api(titik2_input)
#     print("Prediction result for TITIK2:", titik2_prediction)

from fastapi import FastAPI, HTTPException
from pydantic import BaseModel
import requests
import json

from fastapi.middleware.cors import CORSMiddleware

app = FastAPI()

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],  # Ganti dengan domain tertentu jika diperlukan
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"], 
)

# URL Docker Model API
PREDICTION_URL = "http://localhost:8000/predict/"  # Endpoint model logistic regression

class PredictionRequest(BaseModel):
    fill_level: int
    duration_to_full: int
    crowded_location: int
    weekly_frequency: int

@app.post("/predict/")
def predict(request: PredictionRequest):
    try:
        # Kirim data ke API model
        headers = {"Content-Type": "application/json"}
        response = requests.post(PREDICTION_URL, data=request.json(), headers=headers)

        if response.status_code == 200:
            return response.json()  # Kembalikan hasil prediksi
        else:
            raise HTTPException(status_code=response.status_code, detail=response.text)
    except requests.exceptions.RequestException as e:
        raise HTTPException(status_code=500, detail=str(e))
