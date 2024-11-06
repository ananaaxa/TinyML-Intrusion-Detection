from flask import Flask, request, jsonify

app = Flask(_name_)

# Placeholder for the global model weights
global_model_weights = [0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0]

@app.route('/update_model', methods=['POST'])
def update_model():
    data = request.get_json()
    print("Received local update:", data)
    # Logic for aggregating updates could be added here
    return jsonify({"message": "Data received successfully"}), 200

@app.route('/get_global_model', methods=['GET'])
def get_global_model():
    # Send the global model weights
    return jsonify({"model_weights": global_model_weights}), 200

if _name_ == '_main_':
    app.run(host='0.0.0.0', port=5000)