#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include "mqtt/async_client.h"

const std::string SERVER_ADDRESS("tcp://localhost:1883");
const std::string CLIENT_ID("server-controller");
const std::string TOPIC_COMMAND("robot/command");

enum State {
    WAITING,
    MOVING_FORWARD,
    TURNING,
    STOPPED
};

State current_state = WAITING;

void sendCommand(mqtt::async_client& client, const std::string& cmd) {
    mqtt::message_ptr pubmsg = mqtt::make_message(TOPIC_COMMAND, cmd);
    pubmsg->set_qos(1);
    client.publish(pubmsg)->wait();
    std::cout << "Command sent: " << cmd << std::endl;
}

void processStateMachine(mqtt::async_client& client, double distance_to_target) {
    switch (current_state) {
        case WAITING:
            std::cout << "State: WAITING" << std::endl;
            // Ждем команду от пользователя
            break;
        case MOVING_FORWARD:
            std::cout << "State: MOVING_FORWARD" << std::endl;
            if (distance_to_target < 0.5) { // Меньше 50 см до цели
                sendCommand(client, "STOP");
                current_state = WAITING;
            } else {
                sendCommand(client, "FORWARD");
            }
            break;
        case TURNING:
            std::cout << "State: TURNING" << std::endl;
            sendCommand(client, "TURN");
            std::this_thread::sleep_for(std::chrono::seconds(2));
            current_state = MOVING_FORWARD;
            break;
        case STOPPED:
            std::cout << "State: STOPPED" << std::endl;
            sendCommand(client, "STOP");
            current_state = WAITING;
            break;
    }
}

int main() {
    mqtt::async_client client(SERVER_ADDRESS, CLIENT_ID);
    mqtt::connect_options connOpts;
    connOpts.set_clean_session(true);

    try {
        client.connect(connOpts)->wait();
        std::cout << "Connected to MQTT broker." << std::endl;
    }
    catch (const mqtt::exception& exc) {
        std::cerr << "MQTT connection failed: " << exc.what() << std::endl;
        return 1;
    }

    while (true) {
        std::string user_input;
        std::cout << "Enter command (go, stop, turn): ";
        std::cin >> user_input;

        if (user_input == "go") {
            current_state = MOVING_FORWARD;
        } else if (user_input == "stop") {
            current_state = STOPPED;
        } else if (user_input == "turn") {
            current_state = TURNING;
        }

        double distance_to_target = 1.0; // эмуляция расстояния
        processStateMachine(client, distance_to_target);
    }

    client.disconnect()->wait();
    return 0;
}
