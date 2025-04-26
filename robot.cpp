#include <iostream>
#include <string>
#include <thread>
#include <wiringPi.h>
#include "mqtt/async_client.h"

const std::string SERVER_ADDRESS("tcp://192.168.1.100:1883");
const std::string CLIENT_ID("robot-client");
const std::string TOPIC_COMMAND("robot/command");

// Пины моторов
const int MOTOR_LEFT_FORWARD = 0; // GPIO 17
const int MOTOR_RIGHT_FORWARD = 2; // GPIO 27

void moveForward() {
    digitalWrite(MOTOR_LEFT_FORWARD, HIGH);
    digitalWrite(MOTOR_RIGHT_FORWARD, HIGH);
    std::cout << "Moving forward" << std::endl;
}

void turnRight() {
    digitalWrite(MOTOR_LEFT_FORWARD, HIGH);
    digitalWrite(MOTOR_RIGHT_FORWARD, LOW);
    std::cout << "Turning right" << std::endl;
}

void stopMoving() {
    digitalWrite(MOTOR_LEFT_FORWARD, LOW);
    digitalWrite(MOTOR_RIGHT_FORWARD, LOW);
    std::cout << "Stopped" << std::endl;
}

class callback : public virtual mqtt::callback {
public:
    void message_arrived(mqtt::const_message_ptr msg) override {
        std::string payload = msg->to_string();
        std::cout << "Received command: " << payload << std::endl;

        if (payload == "FORWARD") {
            moveForward();
        } else if (payload == "TURN") {
            turnRight();
        } else if (payload == "STOP") {
            stopMoving();
        }
    }
};

int main() {
    wiringPiSetup();
    pinMode(MOTOR_LEFT_FORWARD, OUTPUT);
    pinMode(MOTOR_RIGHT_FORWARD, OUTPUT);

    mqtt::async_client client(SERVER_ADDRESS, CLIENT_ID);
    mqtt::connect_options connOpts;
    connOpts.set_clean_session(true);

    callback cb;
    client.set_callback(cb);

    try {
        client.connect(connOpts)->wait();
        client.start_consuming();
        client.subscribe(TOPIC_COMMAND, 1)->wait();
        std::cout << "Connected to MQTT broker and subscribed." << std::endl;

        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        client.stop_consuming();
        client.disconnect()->wait();
    }
    catch (const mqtt::exception& exc) {
        std::cerr << "MQTT error: " << exc.what() << std::endl;
        return 1;
    }

    return 0;
}
