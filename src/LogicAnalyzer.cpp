#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/logging/log.h>

#include "LogicAnalyzer.hpp"

LOG_MODULE_REGISTER(Logic_Analyzer);

constexpr int THREAD_SIZE = 2048;
constexpr int OUTPUT_STR_SIZE = 13;
constexpr uint8_t ANSII_CLEAR_CONSOLE[] = "\x1B[2J";
constexpr uint8_t ANSII_CONSOLE_BEGINNING[] = "\x1B[H";

__nocache char pin_stat[OUTPUT_STR_SIZE] = { 0 };

K_THREAD_STACK_DEFINE(output_thread_stack, THREAD_SIZE);

LogicAnalyzer* LogicAnalyzer::instance_init(struct device* serial_dev, struct gpio_dt_spec* pins, size_t size){
    static LogicAnalyzer instance(serial_dev, pins, size);
    return &instance;
}

LogicAnalyzer::LogicAnalyzer(struct device* serial_dev, struct gpio_dt_spec* pins, size_t size){
    this->_serial_dev = serial_dev;
    this->_gpio_pins = pins;
    this->_gpio_pins_count = size;

    /* Configure pins for input */
    for(size_t i = 0; i < _gpio_pins_count;  i++){
        gpio_pin_configure_dt(&this->_gpio_pins[i], GPIO_INPUT);
    }

    /* Initiate resources for synchronic UART */
    uart_callback_set(this->_serial_dev, LogicAnalyzer::serial_cb, this);
    k_mutex_init(&this->_serial_sync);
}

void LogicAnalyzer::set_pins(struct gpio_dt_spec* pins, size_t size){
    this->_gpio_pins = pins;
    this->_gpio_pins_count = size;
}

void LogicAnalyzer::set_serial_dev(struct device* serial_dev){
    this->_serial_dev = serial_dev;
}

int LogicAnalyzer::start_capture(){
    LOG_INF("Starting Logic-Analyzer thread...\n");

    /* Starting the thread */
    k_thread_create(&this->_output_thread, &output_thread_stack[0], THREAD_SIZE, LogicAnalyzer::logic_analyzer_output, 
        this, NULL, NULL, CONFIG_LOGIC_ANALYZER_PRIO, K_USER, K_NO_WAIT);
    
    k_thread_start(&this->_output_thread);
    return 0;
}

void LogicAnalyzer::serial_cb(const struct device *dev, struct uart_event *evt, void *user_data){
    LogicAnalyzer* instance = reinterpret_cast<LogicAnalyzer*>(user_data);
    instance->sync_serial(evt);
}

void LogicAnalyzer::sync_serial(struct uart_event *evt){
    switch (evt->type) {
    case UART_TX_DONE:
        k_mutex_unlock(&this->_serial_sync);
        break;
    case UART_RX_RDY:
        break;
    case UART_RX_DISABLED:
        break;
    case UART_RX_BUF_REQUEST:
        break;
    case UART_RX_BUF_RELEASED:
        break;
    case UART_RX_STOPPED:
        break;
    default:
        break;
    }
}

void LogicAnalyzer::logic_analyzer_output(void* logic_instance, void* none, void* null){
    LogicAnalyzer* instance = reinterpret_cast<LogicAnalyzer*>(logic_instance);
    instance->start_output(); 
}

void LogicAnalyzer::start_output(){
    int val = 0;
    int err = 0;
    size_t i = 0;

    while (true){
        /* Clean the entire console */
        k_mutex_lock(&this->_serial_sync, K_FOREVER);
        err = uart_tx(this->_serial_dev, ANSII_CLEAR_CONSOLE, strlen(reinterpret_cast<char*>(const_cast<uint8_t*>(ANSII_CLEAR_CONSOLE))), SYS_FOREVER_US);
        if (err < 0){
            LOG_ERR("Could not transmit ansii code of line beginning, %d\n", err);
            k_yield();
        }
        k_msleep(10);

        /* Print the values */
        for (i = 0; i < this->_gpio_pins_count; i++){
            val = gpio_pin_get_dt(&this->_gpio_pins[i]);
            LOG_INF("Read Pin %d, it's value is  %d\n", i + 1, val);

            sprintf(pin_stat, "Pin %d: %d\n\r", i + 1, val);
            
            k_mutex_lock(&this->_serial_sync, K_FOREVER);
            err = uart_tx(this->_serial_dev, reinterpret_cast<uint8_t*>(pin_stat), strlen(pin_stat), SYS_FOREVER_US);
            if (err < 0){
                LOG_ERR("Could not transmit pin state, %d\n", err);
                k_yield();
            }
            k_msleep(10);
        }

        /* Get to the console beginning */
        k_mutex_lock(&this->_serial_sync, K_FOREVER);
        err = uart_tx(this->_serial_dev, ANSII_CONSOLE_BEGINNING, strlen(reinterpret_cast<char*>(const_cast<uint8_t*>(ANSII_CONSOLE_BEGINNING))), SYS_FOREVER_US);
        if (err < 0){
            LOG_ERR("Could not transmit ansii code of line beginning, %d\n", err);
            k_yield();
        }
        k_msleep(10);

        /* Resets the pin stat */
        memset(pin_stat, 0, OUTPUT_STR_SIZE);
    }
}
