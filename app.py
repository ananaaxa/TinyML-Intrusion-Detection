from flask import Flask, request, jsonify

app = Flask(__name__)

@app.route('/update_model', methods=['POST'])
def update_model():
    data = request.get_json()
    print(data)  # Print the incoming data for inspection
    # Here you can add logic to process and aggregate the incoming updates
    return jsonify({"message": "Data received successfully"}), 200

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
