#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <tflm_esp32.h>
#include <eloquent_tinyml.h>
#include "NN_model.tflite.h"

// Wi-Fi Credentials
const char* ssid = "AnanyasiPhone";
const char* password = "ananyaisdumb";

// Server URL and Endpoints
const char* serverBaseUrl = "https://tinyml-intrusion-detection-gcht.onrender.com";
const char* updateModelEndpoint = "/update_model";
const char* getGlobalModelEndpoint = "/get_global_model";

WiFiClientSecure client;  // Secure client for HTTPS

// TensorFlow Lite Model Config
#define NUMBER_OF_INPUTS 5
#define NUMBER_OF_OUTPUTS 1
#define TENSOR_ARENA_SIZE 2 * 1024
Eloquent::TF::Sequential<4, TENSOR_ARENA_SIZE> tf;

// Model weights for federated learning
float localWeights[10] = {0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0};
float globalWeights[10];

// Function Declarations for FreeRTOS Tasks
void predictionTask(void *parameter);
void modelUpdateTask(void *parameter);

void setup() {
    Serial.begin(115200);
    delay(3000);
    Serial.println("__TENSORFLOW INTRUSION DETECTION__");

    // Connect to Wi-Fi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to Wi-Fi...");
    }
    Serial.println("Connected to Wi-Fi.");

    // Load and Initialize Model
    tf.setNumInputs(NUMBER_OF_INPUTS);
    tf.setNumOutputs(NUMBER_OF_OUTPUTS);
    tf.resolver.AddFullyConnected();
    tf.resolver.AddSoftmax();
    if (!tf.begin(NN_model).isOk()) {
        Serial.println(tf.exception.toString());
        while (true);  // Halt if model loading fails
    }
    Serial.println("Model loaded successfully!");

    // Set client to insecure mode to bypass SSL certificate validation (use cautiously)
    client.setInsecure();

    // Create FreeRTOS tasks with increased stack size for ModelUpdateTask
    xTaskCreate(
        predictionTask,   // Task function
        "PredictionTask", // Name of the task
        4096,             // Stack size (in bytes)
        NULL,             // Task input parameter
        1,                // Priority
        NULL              // Task handle
    );

    xTaskCreate(
        modelUpdateTask,  // Task function
        "ModelUpdateTask", // Name of the task
        8192,              // Increased stack size to handle HTTP and JSON processing
        NULL,              // Task input parameter
        1,                 // Priority
        NULL               // Task handle
    );
}

// Task for making predictions
void predictionTask(void *parameter) {
    float input[NUMBER_OF_INPUTS] = {512.0, 1.0, 3.0, 2.0, 1.5};
    while (true) {
        if (tf.predict(input).isOk()) {
            float prediction = tf.output(0);
            float intrusion_threshold = 0.7;

            if (prediction > intrusion_threshold) {
                Serial.println("Intrusion detected!");
            } else {
                Serial.println("No intrusion detected.");
            }
        } else {
            Serial.println("Error in prediction");
        }

        vTaskDelay(5000 / portTICK_PERIOD_MS);  // Wait 5 seconds between predictions
    }
}

// Task for sending and receiving model updates
void modelUpdateTask(void *parameter) {
    while (true) {
        // Send local model update
        sendLocalUpdate(localWeights, 10);

        // Receive global model update
        receiveGlobalModel(globalWeights, 10);

        vTaskDelay(10000 / portTICK_PERIOD_MS);  // Wait 10 seconds before the next update
    }
}

// Send Local Model Update to Server
void sendLocalUpdate(float* localWeights, int size) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        String url = String(serverBaseUrl) + updateModelEndpoint;
        http.begin(client, url);
        http.addHeader("Content-Type", "application/json");

        StaticJsonDocument<500> doc;
        doc["client_id"] = "esp32_1";
        JsonArray weights = doc.createNestedArray("model_update");

        for (int i = 0; i < size; i++) {
            weights.add(localWeights[i]);
        }

        String jsonMessage;
        serializeJson(doc, jsonMessage);
        int httpResponseCode = http.POST(jsonMessage);

        if (httpResponseCode > 0) {
            String response = http.getString();
            Serial.println("Server Response:");
            Serial.println(response);
        } else {
            Serial.print("Error on sending POST: ");
            Serial.println(httpResponseCode);
        }
        http.end();
    } else {
        Serial.println("WiFi not connected.");
    }
}

// Receive Global Model Update from Server
void receiveGlobalModel(float* globalWeights, int size) {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        String url = String(serverBaseUrl) + getGlobalModelEndpoint;
        http.begin(client, url);

        int httpResponseCode = http.GET();

        if (httpResponseCode > 0) {
            String payload = http.getString();
            Serial.println("Received Global Model:");
            Serial.println(payload);

            StaticJsonDocument<500> doc;
            DeserializationError error = deserializeJson(doc, payload);

            if (!error) {
                JsonArray weights = doc["model_weights"];
                for (int i = 0; i < size && i < weights.size(); i++) {
                    globalWeights[i] = weights[i];
                }
                Serial.println("Global weights updated successfully.");
            } else {
                Serial.println("Error parsing JSON response");
            }
        } else {
            Serial.print("Error on receiving GET: ");
            Serial.println(httpResponseCode);
        }
        http.end();
    } else {
        Serial.println("WiFi not connected.");
    }
}

void loop() {
    // Empty loop, as tasks are managed by FreeRTOS
}
