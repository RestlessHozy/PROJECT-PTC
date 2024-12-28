from fastapi import FastAPI, HTTPException
from pydantic import BaseModel
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import JSONResponse
import joblib
import numpy as np
import logging

# Konfigurasi logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

# Inisialisasi aplikasi FastAPI
app = FastAPI()

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],  # Ganti dengan domain tertentu jika diperlukan
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# Muat model yang telah disimpan
try:
    model = joblib.load("projectptc/model.joblib")
    logger.info("Model loaded successfully.")
except Exception as e:
    logger.error(f"Failed to load the model: {e}")
    model = None

# Schema untuk input data
class ModelInput(BaseModel):
    fill_level: int
    duration_to_full: int
    crowded_location: int
    weekly_frequency: int


@app.get("/")
def root():
    return {"message": "Model API is running. Use /predict endpoint for predictions."}


@app.post("/predict/")
def predict(input_data: ModelInput):
    if model is None:
        raise HTTPException(status_code=500, detail="Model not loaded.")
    
    try:
        # Siapkan input sesuai format model
        features = np.array([
            input_data.fill_level,
            input_data.duration_to_full,
            input_data.crowded_location,
            input_data.weekly_frequency,
        ]).reshape(1, -1)

        # Debugging input
        logger.info(f"Received input for prediction: {features}")

        # Prediksi dengan model
        prediction = model.predict(features)
        recommendation = "pick" if prediction[0] == 1 else "do_not_pick"

        # Debugging hasil prediksi
        logger.info(f"Prediction result: {prediction[0]}, Recommendation: {recommendation}")

        return {"recommendation": recommendation}

    except Exception as e:
        logger.error(f"Prediction failed: {e}")
        raise HTTPException(status_code=400, detail=f"Prediction failed: {e}")
