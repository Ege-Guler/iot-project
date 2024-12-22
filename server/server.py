from flask import Flask, request, jsonify
import joblib
import pandas as pd

app = Flask(__name__)

# Load model and preprocessors
model_path = 'weather_xgb_model.pkl'
scaler_path = 'scaler.pkl'
label_encoder_path = 'label_encoder.pkl'
features_path = 'features.pkl'

try:
    model = joblib.load(model_path)
    scaler = joblib.load(scaler_path)
    label_encoder = joblib.load(label_encoder_path)
    feature_names = joblib.load(features_path)
except Exception as e:
    print(f"Error loading model or preprocessors: {e}")

@app.route('/')
def home():
    return "Weather Prediction API is running!"

@app.route('/predict', methods=['POST'])
def predict_weather():
    try:
        # Get JSON input
        data = request.json

        # Extract inputs
        temp = float(data['temperature']) + 273.15  # Convert to Kelvin
        humidity = float(data['humidity'])
        hour = int(data['hour'])
        day = int(data['day'])
        month = int(data['month'])

        # Prepare input for prediction
        input_data = pd.DataFrame(columns=feature_names)
        input_data.loc[0] = [temp] * 5 + [humidity] * 5 + [0] + [1 if temp <= 276.15 else 0] + [hour, day, month]

        # Scale input
        input_scaled = scaler.transform(input_data)

        # Predict
        prediction = model.predict(input_scaled)[0]
        predicted_label = label_encoder.inverse_transform([prediction])[0]

        # Return prediction
        return jsonify({
            'prediction': predicted_label,
            'next_hour': hour + 1
        })

    except Exception as e:
        return jsonify({'error': str(e)})

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)
