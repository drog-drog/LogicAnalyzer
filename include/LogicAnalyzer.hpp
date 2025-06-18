#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

class LogicAnalyzer{
public:
    static LogicAnalyzer* instance_init(struct device* serial_dev, struct gpio_dt_spec* pins, size_t size);

    /* Default big 5 */
    LogicAnalyzer(LogicAnalyzer&) noexcept = default;
    LogicAnalyzer(LogicAnalyzer&&) noexcept = default;
    LogicAnalyzer& operator=(LogicAnalyzer&) noexcept = default;
    LogicAnalyzer& operator=(LogicAnalyzer&&) noexcept = default;
    virtual ~LogicAnalyzer() = default;

    /* Setters */
    void set_pins(struct gpio_dt_spec* pins, size_t size);
    void set_serial_dev(struct device* serial_dev);

    int start_capture();
    int stop_capture();
    
    /* Callback handlers */
    void sync_serial(struct uart_event *evt);
    void start_output();
    
private:
    /* Analyzer sources for output */
    struct gpio_dt_spec* _gpio_pins;
    size_t _gpio_pins_count;
    struct device* _serial_dev;
    struct k_mutex _serial_sync;
    
    /* Output thread */
    struct k_thread _output_thread;
    
    
    /* Ctor */
    LogicAnalyzer(struct device* serial_dev, struct gpio_dt_spec* pins, size_t size);
    
    static void logic_analyzer_output(void* _gpio_pins, void* size, void* none);
    
    static void serial_cb(const struct device *dev, struct uart_event *evt, void *user_data);
};
