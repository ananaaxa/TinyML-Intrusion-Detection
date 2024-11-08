from flask import Flask, request, jsonify

app = Flask(_name_)

# Placeholder for the global model weights
global_model_weights = [0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0]
local_updates = []  # List to store updates from different clients

@app.route('/update_model', methods=['POST'])
def update_model():
    data = request.get_json()
    local_weights = data.get("model_weights")
    
    if local_weights:
        local_updates.append(local_weights)
        print("Received local update:", local_weights)
        
        # If enough local updates have been received, update the global model
        if len(local_updates) >= 3:  # Example threshold; adjust as needed
            update_global_model()
            local_updates.clear()  # Clear updates after aggregation
            
        return jsonify({"message": "Local model weights received"}), 200
    else:
        return jsonify({"error": "No model weights provided"}), 400

@app.route('/get_global_model', methods=['GET'])
def get_global_model():
    # Send the global model weights
    return jsonify({"model_weights": global_model_weights}), 200

def update_global_model():
    # Aggregate local updates by averaging
    global global_model_weights
    
    # Calculate the average of each weight
    num_updates = len(local_updates)
    aggregated_weights = [sum(weights[i] for weights in local_updates) / num_updates for i in range(len(global_model_weights))]
    
    global_model_weights = aggregated_weights
    print("Global model updated:", global_model_weights)

if _name_ == '_main_':
    app.run(host='0.0.0.0', port=5000)