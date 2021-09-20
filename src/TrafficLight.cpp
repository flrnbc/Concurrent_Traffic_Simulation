#include <chrono>
#include <iostream>
#include <mutex>
#include <random>
#include <stdlib.h>
#include <thread>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait()
    // to wait for and receive new messages and pull them from the queue using move semantics.
    // The received object should then be returned by the receive function.
    std::unique_lock<std::mutex> lck(_mtxQueue);
    _cond.wait(lck, [this] { return !_queue.empty(); });

    T msg = std::move(_queue.back());
    _queue.pop_back();

    return msg; // not copied because of return value optimization
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex>
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> lck(_mtxQueue);

    _queue.push_back(std::move(msg));
    _cond.notify_one();
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop
    // runs and repeatedly calls the receive function on the message queue.
    // Once it receives TrafficLightPhase::green, the method returns.
    while (true) {
        TrafficLightPhase t;
        t = this->_msgQueue.receive();

        if (t == TrafficLightPhase::green) {
            return;
        }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class.
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));

}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles
    // and toggles the current phase of the traffic light between red and green and sends an update method
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds.
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles.
  
  	// for random floats in [4.0, 6.0)
  	std::random_device rd;
    std::mt19937 eng(rd());
    std::uniform_real_distribution<> distribution(4.0, 6.0);
  
	// get (first) start time and cycle duration
    auto start = std::chrono::high_resolution_clock::now();
  	auto cycleDuration = distribution(eng)*1000; // multiply to get milliseconds
    
    while (true) {
        auto end = std::chrono::high_resolution_clock::now();
        auto time_span = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

      	
        if (time_span.count() - cycleDuration >= 0) {
            if (this->getCurrentPhase() == TrafficLightPhase::red){
                this->setCurrentPhase(TrafficLightPhase::green);
            } else {
                this->setCurrentPhase(TrafficLightPhase::red);
            }
            _msgQueue.send(std::move(this->getCurrentPhase()));
          	// update start time and cycle duration
          	start = std::chrono::high_resolution_clock::now();
          	cycleDuration = distribution(eng)*1000;
          	// sleep 
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}
