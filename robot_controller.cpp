// robot_controller.cpp

#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <wiringPi.h>
#include "mqtt/async_client.h"

using namespace std;

// Пины моторов (проверь под свою плату!)
const int MOTOR_LEFT_FORWARD = 0;
const int MOTOR_LEFT_BACKWARD = 1;
const int MOTOR_RIGHT_FORWARD = 2;
const int MOTOR_RIGHT_BACKWARD = 3;

// MQTT настройки
const string ADDRESS("tcp://localhost:1883");
const string CLIENT_ID("robot_client");
const string TOPIC_CONTROL("robot/control");
const int QOS = 1;

enum State {
    WAITING,
    MOVING_TO_TARGET,
    TURNING,
    GOING_STRAIGHT,
    ARRIVED
};

State current_state = WAITING;

// Функции управления моторами
void stopMotors() {
    digitalWrite(MOTOR_LEFT_FORWARD, LOW);
    digitalWrite(MOTOR_LEFT_BACKWARD, LOW);
    digitalWrite(MOTOR_RIGHT_FORWARD, LOW);
    digitalWrite(MOTOR_RIGHT_BACKWARD, LOW);
}

void moveForward() {
    digitalWrite(MOTOR_LEFT_FORWARD, HIGH);
    digitalWrite(MOTOR_LEFT_BACKWARD, LOW);
    digitalWrite(MOTOR_RIGHT_FORWARD, HIGH);
    digitalWrite(MOTOR_RIGHT_BACKWARD, LOW);
}

void turnLeft() {
    digitalWrite(MOTOR_LEFT_FORWARD, LOW);
    digitalWrite(MOTOR_LEFT_BACKWARD, HIGH);
    digitalWrite(MOTOR_RIGHT_FORWARD, HIGH);
    digitalWrite(MOTOR_RIGHT_BACKWARD, LOW);
}

void turnRight() {
    digitalWrite(MOTOR_LEFT_FORWARD, HIGH);
    digitalWrite(MOTOR_LEFT_BACKWARD, LOW);
    digitalWrite(MOTOR_RIGHT_FORWARD, LOW);
    digitalWrite(MOTOR_RIGHT_BACKWARD, HIGH);
}

// Обработчик сообщений MQTT
void on_message_arrived(string topic, string payload) {
    cout << "Получено сообщение: " << payload << endl;

    if (payload == "move_forward") {
        moveForward();
        current_state = GOING_STRAIGHT;
    }
    else if (payload == "turn_left") {
        turnLeft();
        current_state = TURNING;
    }
    else if (payload == "turn_right") {
        turnRight();
        current_state = TURNING;
    }
    else if (payload == "stop") {
        stopMotors();
        current_state = WAITING;
    }
}

// Класс слушателя сообщений
class callback : public virtual mqtt::callback {
public:
    void message_arrived(mqtt::const_message_ptr msg) override {
        on_message_arrived(msg->get_topic(), msg->to_string());
    }
};

int main() {
    wiringPiSetup();
    pinMode(MOTOR_LEFT_FORWARD, OUTPUT);
    pinMode(MOTOR_LEFT_BACKWARD, OUTPUT);
    pinMode(MOTOR_RIGHT_FORWARD, OUTPUT);
    pinMode(MOTOR_RIGHT_BACKWARD, OUTPUT);

    mqtt::async_client client(ADDRESS, CLIENT_ID);
    callback cb;
    client.set_callback(cb);

    mqtt::connect_options connOpts;
    connOpts.set_keep_alive_interval(20);
    connOpts.set_clean_session(true);

    try {
        cout << "Подключение к MQTT брокеру..." << endl;
        client.connect(connOpts)->wait();
        client.subscribe(TOPIC_CONTROL, QOS);

        cout << "Робот готов к приему команд!" << endl;

        // Основной цикл работы робота
        while (true) {
            switch (current_state) {
                case WAITING:
                    // Ждём команду
                    this_thread::sleep_for(chrono::milliseconds(100));
                    break;

                case MOVING_TO_TARGET:
                    moveForward();
                    break;

                case TURNING:
                    // Поворот уже инициирован командой
                    break;

                case GOING_STRAIGHT:
                    // Движемся вперёд
                    break;

                case ARRIVED:
                    stopMotors();
                    current_state = WAITING;
                    break;
            }
            this_thread::sleep_for(chrono::milliseconds(100));
        }

        client.disconnect()->wait();
    }
    catch (const mqtt::exception& exc) {
        cerr << "Ошибка: " << exc.what() << endl;
        return 1;
    }

    return 0;
}
