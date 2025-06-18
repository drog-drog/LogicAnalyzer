#include <zephyr/kernel.h>
#include <LogicAnalyzer.hpp>

#define GPIO_NODE DT_NODELABEL(logic_analyzer)
#define GPIO_COUNT DT_PROP_LEN(GPIO_NODE, gpios)
#define INIT_GPIO(idx, _) GPIO_DT_SPEC_GET_BY_IDX(GPIO_NODE, gpios, idx),
struct gpio_dt_spec gpio_list[GPIO_COUNT] = {
    LISTIFY(GPIO_COUNT, INIT_GPIO, ())
};

int main(){
    struct device* serial_dev = const_cast<struct device*>(DEVICE_DT_GET(DT_NODELABEL(usart2)));
    LogicAnalyzer* analyzer = NULL;

    analyzer = LogicAnalyzer::instance_init(serial_dev, gpio_list, GPIO_COUNT);
    analyzer->start_capture();

    return 0;
}
