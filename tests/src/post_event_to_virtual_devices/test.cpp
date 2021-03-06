#define CATCH_CONFIG_RUNNER
#include "../../vendor/catch/catch.hpp"

#include "manipulator/details/post_event_to_virtual_devices.hpp"
#include "manipulator/manipulator_factory.hpp"
#include "manipulator/manipulator_manager.hpp"
#include "manipulator/manipulator_managers_connector.hpp"
#include "thread_utility.hpp"

#define ENQUEUE_EVENT(QUEUE, DEVICE_ID, TIME_STAMP, EVENT, EVENT_TYPE, ORIGINAL_EVENT) \
  QUEUE.emplace_back_event(krbn::device_id(DEVICE_ID),                                 \
                           TIME_STAMP,                                                 \
                           EVENT,                                                      \
                           krbn::event_type::EVENT_TYPE,                               \
                           ORIGINAL_EVENT);

#define ENQUEUE_LAZY_EVENT(QUEUE, DEVICE_ID, TIME_STAMP, EVENT, EVENT_TYPE, ORIGINAL_EVENT) \
  QUEUE.emplace_back_event(krbn::device_id(DEVICE_ID),                                      \
                           TIME_STAMP,                                                      \
                           EVENT,                                                           \
                           krbn::event_type::EVENT_TYPE,                                    \
                           ORIGINAL_EVENT,                                                  \
                           true);

#define ENQUEUE_KEYBOARD_EVENT(EVENTS, KEY_CODE, VALUE, TIME_STAMP)                            \
  {                                                                                            \
    pqrs::karabiner_virtual_hid_device::hid_event_service::keyboard_event keyboard_event;      \
    keyboard_event.usage_page = *(krbn::types::get_usage_page(krbn::key_code::KEY_CODE));      \
    keyboard_event.usage = *(krbn::types::get_usage(krbn::key_code::KEY_CODE));                \
    keyboard_event.value = VALUE;                                                              \
    EVENTS.push_back(post_event_to_virtual_devices::queue::event(keyboard_event, TIME_STAMP)); \
  }

#define PUSH_BACK_LAZY_QUEUED_EVENT(VECTOR, DEVICE_ID, TIME_STAMP, EVENT, EVENT_TYPE, ORIGINAL_EVENT) \
  VECTOR.push_back(krbn::event_queue::queued_event(krbn::device_id(DEVICE_ID),                        \
                                                   TIME_STAMP,                                        \
                                                   EVENT,                                             \
                                                   krbn::event_type::EVENT_TYPE,                      \
                                                   ORIGINAL_EVENT,                                    \
                                                   true))

using krbn::manipulator::details::post_event_to_virtual_devices;

namespace {
krbn::event_queue::queued_event::event a_event(krbn::key_code::a);
krbn::event_queue::queued_event::event escape_event(krbn::key_code::escape);
krbn::event_queue::queued_event::event fn_event(krbn::key_code::fn);
krbn::event_queue::queued_event::event left_control_event(krbn::key_code::left_control);
krbn::event_queue::queued_event::event left_option_event(krbn::key_code::left_option);
krbn::event_queue::queued_event::event left_shift_event(krbn::key_code::left_shift);
krbn::event_queue::queued_event::event p_event(krbn::key_code::p);
krbn::event_queue::queued_event::event page_up_event(krbn::key_code::page_up);
krbn::event_queue::queued_event::event return_or_enter_event(krbn::key_code::return_or_enter);
krbn::event_queue::queued_event::event right_shift_event(krbn::key_code::right_shift);
krbn::event_queue::queued_event::event spacebar_event(krbn::key_code::spacebar);
krbn::event_queue::queued_event::event tab_event(krbn::key_code::tab);
krbn::event_queue::queued_event::event up_arrow_event(krbn::key_code::up_arrow);

krbn::event_queue::queued_event::event button1_event(krbn::pointing_button::button1);
krbn::event_queue::queued_event::event pointing_x_m10_event(krbn::event_queue::queued_event::event::type::pointing_x, -10);
krbn::event_queue::queued_event::event pointing_y_10_event(krbn::event_queue::queued_event::event::type::pointing_y, 10);
krbn::event_queue::queued_event::event device_ungrabbed_event(krbn::event_queue::queued_event::event::type::device_ungrabbed, 1);

uint64_t modifier_wait = krbn::time_utility::nano_to_absolute(NSEC_PER_MSEC);
} // namespace

TEST_CASE("generic") {
  {
    krbn::manipulator::manipulator_manager manipulator_manager;

    auto manipulator = std::make_shared<post_event_to_virtual_devices>();
    manipulator_manager.push_back_manipulator(std::shared_ptr<krbn::manipulator::details::base>(manipulator));

    krbn::event_queue input_event_queue;
    krbn::event_queue output_event_queue;

    krbn::manipulator::manipulator_managers_connector connector;
    connector.emplace_back_connection(manipulator_manager,
                                      input_event_queue,
                                      output_event_queue);

    uint64_t time_stamp = 0;
    uint64_t interval = krbn::time_utility::nano_to_absolute(NSEC_PER_SEC);

    ENQUEUE_EVENT(input_event_queue, 1, time_stamp += interval, left_shift_event, key_down, left_shift_event);
    ENQUEUE_EVENT(input_event_queue, 1, time_stamp += interval, left_shift_event, key_up, left_shift_event);
    ENQUEUE_EVENT(input_event_queue, 2, time_stamp += interval, spacebar_event, key_down, spacebar_event);
    ENQUEUE_EVENT(input_event_queue, 1, time_stamp += interval, spacebar_event, key_up, spacebar_event);
    ENQUEUE_EVENT(input_event_queue, 1, time_stamp += interval, escape_event, key_down, escape_event);
    ENQUEUE_EVENT(input_event_queue, 1, time_stamp += interval, escape_event, key_up, escape_event);
    ENQUEUE_EVENT(input_event_queue, 1, time_stamp += interval, tab_event, key_down, tab_event);
    ENQUEUE_EVENT(input_event_queue, 1, time_stamp += interval, button1_event, key_down, button1_event);
    ENQUEUE_EVENT(input_event_queue, 1, time_stamp += interval, pointing_x_m10_event, key_down, pointing_x_m10_event);
    ENQUEUE_EVENT(input_event_queue, 1, time_stamp += interval, button1_event, key_up, button1_event);
    ENQUEUE_EVENT(input_event_queue, 1, time_stamp += interval, pointing_y_10_event, key_down, pointing_y_10_event);

    connector.manipulate(time_stamp += interval);

    {
      std::vector<post_event_to_virtual_devices::queue::event> expected;
      time_stamp = 0;
      ENQUEUE_KEYBOARD_EVENT(expected, left_shift, 1, time_stamp += interval);
      ENQUEUE_KEYBOARD_EVENT(expected, left_shift, 0, time_stamp += interval);
      ENQUEUE_KEYBOARD_EVENT(expected, spacebar, 1, time_stamp += interval);
      ENQUEUE_KEYBOARD_EVENT(expected, spacebar, 0, time_stamp += interval);
      ENQUEUE_KEYBOARD_EVENT(expected, escape, 1, time_stamp += interval);
      ENQUEUE_KEYBOARD_EVENT(expected, escape, 0, time_stamp += interval);
      ENQUEUE_KEYBOARD_EVENT(expected, tab, 1, time_stamp += interval);

      {
        pqrs::karabiner_virtual_hid_device::hid_report::pointing_input pointing_input;
        pointing_input.buttons[0] = 0x1;
        expected.push_back(post_event_to_virtual_devices::queue::event(pointing_input, time_stamp += interval));
      }
      {
        pqrs::karabiner_virtual_hid_device::hid_report::pointing_input pointing_input;
        pointing_input.buttons[0] = 0x1;
        pointing_input.x = -10;
        expected.push_back(post_event_to_virtual_devices::queue::event(pointing_input, time_stamp += interval));
      }
      {
        pqrs::karabiner_virtual_hid_device::hid_report::pointing_input pointing_input;
        expected.push_back(post_event_to_virtual_devices::queue::event(pointing_input, time_stamp += interval));
      }
      {
        pqrs::karabiner_virtual_hid_device::hid_report::pointing_input pointing_input;
        pointing_input.y = 10;
        expected.push_back(post_event_to_virtual_devices::queue::event(pointing_input, time_stamp += interval));
      }

      REQUIRE(manipulator->get_queue().get_events() == expected);
    }

    {
      std::vector<std::pair<krbn::device_id, krbn::key_code>> expected({
          std::make_pair(krbn::device_id(1), krbn::key_code::tab),
      });
      REQUIRE(manipulator->get_key_event_dispatcher().get_pressed_keys() == expected);
    }
  }
}

TEST_CASE("Same modifier twice from different devices") {
  {
    krbn::manipulator::manipulator_manager manipulator_manager;

    auto manipulator = std::make_shared<post_event_to_virtual_devices>();
    manipulator_manager.push_back_manipulator(std::shared_ptr<krbn::manipulator::details::base>(manipulator));

    krbn::event_queue input_event_queue;
    krbn::event_queue output_event_queue;

    krbn::manipulator::manipulator_managers_connector connector;
    connector.emplace_back_connection(manipulator_manager,
                                      input_event_queue,
                                      output_event_queue);

    uint64_t time_stamp = 0;
    uint64_t interval = krbn::time_utility::nano_to_absolute(NSEC_PER_SEC);

    ENQUEUE_EVENT(input_event_queue, 1, time_stamp += interval, left_shift_event, key_down, left_shift_event);
    ENQUEUE_EVENT(input_event_queue, 2, time_stamp += interval, left_shift_event, key_down, left_shift_event);
    ENQUEUE_EVENT(input_event_queue, 1, time_stamp += interval, left_shift_event, key_up, left_shift_event);

    connector.manipulate(time_stamp += interval);

    std::vector<post_event_to_virtual_devices::queue::event> expected;
    ENQUEUE_KEYBOARD_EVENT(expected, left_shift, 1, interval);

    REQUIRE(manipulator->get_queue().get_events() == expected);

    time_stamp = interval * 3;

    ENQUEUE_EVENT(input_event_queue, 2, time_stamp += interval, left_shift_event, key_up, left_shift_event);
    ENQUEUE_EVENT(input_event_queue, 1, time_stamp += interval, left_shift_event, key_down, left_shift_event);

    connector.manipulate(time_stamp += interval);

    ENQUEUE_KEYBOARD_EVENT(expected, left_shift, 0, interval * 4);
    ENQUEUE_KEYBOARD_EVENT(expected, left_shift, 1, interval * 5);

    REQUIRE(manipulator->get_queue().get_events() == expected);
  }
}

TEST_CASE("device_ungrabbed event") {
  {
    krbn::manipulator::manipulator_manager manipulator_manager;

    auto manipulator = std::make_shared<post_event_to_virtual_devices>();
    manipulator_manager.push_back_manipulator(std::shared_ptr<krbn::manipulator::details::base>(manipulator));

    krbn::event_queue input_event_queue;
    krbn::event_queue output_event_queue;

    krbn::manipulator::manipulator_managers_connector connector;
    connector.emplace_back_connection(manipulator_manager,
                                      input_event_queue,
                                      output_event_queue);

    uint64_t time_stamp = 0;
    uint64_t interval = krbn::time_utility::nano_to_absolute(NSEC_PER_SEC);

    ENQUEUE_EVENT(input_event_queue, 1, time_stamp += interval, tab_event, key_down, tab_event);
    ENQUEUE_EVENT(input_event_queue, 2, time_stamp += interval, escape_event, key_down, escape_event);
    ENQUEUE_EVENT(input_event_queue, 1, time_stamp += interval, spacebar_event, key_down, spacebar_event);
    ENQUEUE_EVENT(input_event_queue, 1, time_stamp += interval, tab_event, key_up, tab_event);
    ENQUEUE_EVENT(input_event_queue, 1, time_stamp += interval, left_shift_event, key_down, left_shift_event);
    ENQUEUE_EVENT(input_event_queue, 1, time_stamp += interval, return_or_enter_event, key_down, return_or_enter_event);
    ENQUEUE_EVENT(input_event_queue, 2, time_stamp += interval, a_event, key_down, a_event);
    ENQUEUE_EVENT(input_event_queue, 1, time_stamp += interval, a_event, key_down, a_event); // `key_code::a` is pressed until device_id(2) is ungrabbed.

    connector.manipulate(interval * 5);

    time_stamp = 0;

    std::vector<post_event_to_virtual_devices::queue::event> expected;
    ENQUEUE_KEYBOARD_EVENT(expected, tab, 1, time_stamp += interval);
    ENQUEUE_KEYBOARD_EVENT(expected, escape, 1, time_stamp += interval);
    ENQUEUE_KEYBOARD_EVENT(expected, spacebar, 1, time_stamp += interval);
    ENQUEUE_KEYBOARD_EVENT(expected, tab, 0, time_stamp += interval);
    ENQUEUE_KEYBOARD_EVENT(expected, left_shift, 1, time_stamp += interval);
    ENQUEUE_KEYBOARD_EVENT(expected, return_or_enter, 1, time_stamp += interval);
    ENQUEUE_KEYBOARD_EVENT(expected, a, 1, time_stamp += interval);

    REQUIRE(manipulator->get_queue().get_events() == expected);

    ENQUEUE_EVENT(input_event_queue, 1, interval * 6, device_ungrabbed_event, key_down, device_ungrabbed_event);
    ENQUEUE_EVENT(input_event_queue, 3, interval * 6, device_ungrabbed_event, key_down, device_ungrabbed_event);

    connector.manipulate(interval * 7);

    ENQUEUE_KEYBOARD_EVENT(expected, spacebar, 0, interval * 6);
    ENQUEUE_KEYBOARD_EVENT(expected, return_or_enter, 0, interval * 6);
    ENQUEUE_KEYBOARD_EVENT(expected, left_shift, 0, time_stamp + modifier_wait); // need to wait after 'kHIDUsage_KeyboardA key_down'

    REQUIRE(manipulator->get_queue().get_events() == expected);

    ENQUEUE_EVENT(input_event_queue, 2, interval * 8, device_ungrabbed_event, key_down, device_ungrabbed_event);

    connector.manipulate(interval * 9);

    ENQUEUE_KEYBOARD_EVENT(expected, escape, 0, interval * 8);
    ENQUEUE_KEYBOARD_EVENT(expected, a, 0, interval * 8);

    REQUIRE(manipulator->get_queue().get_events() == expected);
  }
}

TEST_CASE("wait around modifier") {
  {
    krbn::manipulator::manipulator_manager manipulator_manager;

    auto manipulator = std::make_shared<post_event_to_virtual_devices>();
    manipulator_manager.push_back_manipulator(std::shared_ptr<krbn::manipulator::details::base>(manipulator));

    krbn::event_queue input_event_queue;
    krbn::event_queue output_event_queue;

    krbn::manipulator::manipulator_managers_connector connector;
    connector.emplace_back_connection(manipulator_manager,
                                      input_event_queue,
                                      output_event_queue);

    uint64_t time_stamp = 0;
    uint64_t interval = krbn::time_utility::nano_to_absolute(NSEC_PER_SEC);

    ENQUEUE_EVENT(input_event_queue, 1, time_stamp += interval, tab_event, key_down, tab_event);
    // left_shift_event, escape_event (key_down)
    ENQUEUE_EVENT(input_event_queue, 1, time_stamp += interval, left_shift_event, key_down, left_shift_event);
    ENQUEUE_EVENT(input_event_queue, 1, time_stamp + 1, escape_event, key_down, escape_event);
    // left_shift_event, escape_event (key_up)
    ENQUEUE_EVENT(input_event_queue, 1, time_stamp += interval, escape_event, key_up, escape_event);
    ENQUEUE_EVENT(input_event_queue, 1, time_stamp + 1, left_shift_event, key_up, left_shift_event);
    // escape_event, left_shift_event (key_down)
    ENQUEUE_EVENT(input_event_queue, 1, time_stamp += interval, escape_event, key_down, escape_event);
    ENQUEUE_EVENT(input_event_queue, 1, time_stamp + 1, left_shift_event, key_down, left_shift_event);
    // escape_event, left_shift_event (key_up)
    ENQUEUE_EVENT(input_event_queue, 1, time_stamp += interval, left_shift_event, key_up, left_shift_event);
    ENQUEUE_EVENT(input_event_queue, 1, time_stamp + 1, escape_event, key_up, escape_event);

    connector.manipulate(time_stamp += interval);

    time_stamp = 0;

    std::vector<post_event_to_virtual_devices::queue::event> expected;
    ENQUEUE_KEYBOARD_EVENT(expected, tab, 1, time_stamp += interval);
    ENQUEUE_KEYBOARD_EVENT(expected, left_shift, 1, time_stamp += interval);
    ENQUEUE_KEYBOARD_EVENT(expected, escape, 1, time_stamp + modifier_wait);
    ENQUEUE_KEYBOARD_EVENT(expected, escape, 0, time_stamp += interval);
    ENQUEUE_KEYBOARD_EVENT(expected, left_shift, 0, time_stamp + modifier_wait);
    ENQUEUE_KEYBOARD_EVENT(expected, escape, 1, time_stamp += interval);
    ENQUEUE_KEYBOARD_EVENT(expected, left_shift, 1, time_stamp + modifier_wait);
    ENQUEUE_KEYBOARD_EVENT(expected, left_shift, 0, time_stamp += interval);
    ENQUEUE_KEYBOARD_EVENT(expected, escape, 0, time_stamp + modifier_wait);

    REQUIRE(manipulator->get_queue().get_events() == expected);
  }
}

TEST_CASE("lazy events") {
  {
    // ----------------------------------------
    // Collapse lazy events
    //   * left_shift:   key_down,key_up
    //   * left_control: key_up,key_down

    krbn::manipulator::manipulator_manager manipulator_manager;

    auto manipulator = std::make_shared<post_event_to_virtual_devices>();
    manipulator_manager.push_back_manipulator(std::shared_ptr<krbn::manipulator::details::base>(manipulator));

    krbn::event_queue input_event_queue;
    krbn::event_queue output_event_queue;

    krbn::manipulator::manipulator_managers_connector connector;
    connector.emplace_back_connection(manipulator_manager,
                                      input_event_queue,
                                      output_event_queue);

    uint64_t time_stamp = 0;
    uint64_t interval = krbn::time_utility::nano_to_absolute(NSEC_PER_SEC);

    ENQUEUE_LAZY_EVENT(input_event_queue, 1, time_stamp += interval, left_shift_event, key_down, left_shift_event);
    ENQUEUE_LAZY_EVENT(input_event_queue, 1, time_stamp += interval, left_control_event, key_down, left_control_event);
    ENQUEUE_LAZY_EVENT(input_event_queue, 1, time_stamp += interval, left_shift_event, key_up, left_shift_event);
    ENQUEUE_EVENT(input_event_queue, 1, time_stamp += interval, left_control_event, key_up, left_control_event);

    ENQUEUE_EVENT(input_event_queue, 1, time_stamp += interval, fn_event, key_down, fn_event);
    ENQUEUE_EVENT(input_event_queue, 1, time_stamp += interval, spacebar_event, key_down, spacebar_event);
    ENQUEUE_LAZY_EVENT(input_event_queue, 1, time_stamp += interval, fn_event, key_up, fn_event);
    ENQUEUE_LAZY_EVENT(input_event_queue, 1, time_stamp += interval, spacebar_event, key_up, spacebar_event);

    ENQUEUE_EVENT(input_event_queue, 1, time_stamp += interval, fn_event, key_down, fn_event);
    ENQUEUE_EVENT(input_event_queue, 1, time_stamp += interval, spacebar_event, key_down, spacebar_event);
    ENQUEUE_EVENT(input_event_queue, 1, time_stamp += interval, fn_event, key_up, fn_event);
    ENQUEUE_EVENT(input_event_queue, 1, time_stamp += interval, spacebar_event, key_up, spacebar_event);

    connector.manipulate(time_stamp += interval);

    // ----------------------------------------

    std::vector<post_event_to_virtual_devices::queue::event> expected;
    time_stamp = 0;

    time_stamp += interval;
    time_stamp += interval;
    time_stamp += interval;
    time_stamp += interval;

    ENQUEUE_KEYBOARD_EVENT(expected, fn, 1, time_stamp += interval);
    ENQUEUE_KEYBOARD_EVENT(expected, spacebar, 1, time_stamp += interval);
    time_stamp += interval;
    ENQUEUE_KEYBOARD_EVENT(expected, spacebar, 0, time_stamp += interval);

    time_stamp += interval;
    ENQUEUE_KEYBOARD_EVENT(expected, spacebar, 1, time_stamp += interval);
    ENQUEUE_KEYBOARD_EVENT(expected, fn, 0, time_stamp += interval);
    ENQUEUE_KEYBOARD_EVENT(expected, spacebar, 0, time_stamp += interval);

    REQUIRE(manipulator->get_queue().get_events() == expected);
  }
}

namespace {
class actual_examples_helper final {
public:
  actual_examples_helper(void) {
    // fn-up_arrow -> page_up
    // control-p -> up_arrow

    std::ifstream input("json/complex_modifications.json");
    auto json = nlohmann::json::parse(input);
    for (const auto& j : json) {
      auto m = krbn::manipulator::manipulator_factory::make_manipulator(j);
      modifications_manipulator_manager_.push_back_manipulator(m);
    }

    manipulator_ = std::make_shared<post_event_to_virtual_devices>();
    post_event_to_virtual_devices_manipulator_manager_.push_back_manipulator(std::shared_ptr<krbn::manipulator::details::base>(manipulator_));

    connector_.emplace_back_connection(modifications_manipulator_manager_,
                                       input_event_queue_,
                                       middle_event_queue_);
    connector_.emplace_back_connection(post_event_to_virtual_devices_manipulator_manager_,
                                       output_event_queue_);
  }

  krbn::event_queue& get_input_event_queue(void) {
    return input_event_queue_;
  }

  void manipulate(uint64_t time_stamp) {
    connector_.manipulate(time_stamp);
  }

  const std::vector<post_event_to_virtual_devices::queue::event> get_events(void) {
    return manipulator_->get_queue().get_events();
  }

private:
  krbn::manipulator::manipulator_manager modifications_manipulator_manager_;
  krbn::manipulator::manipulator_manager post_event_to_virtual_devices_manipulator_manager_;
  std::shared_ptr<post_event_to_virtual_devices> manipulator_;
  krbn::event_queue input_event_queue_;
  krbn::event_queue middle_event_queue_;
  krbn::event_queue output_event_queue_;
  krbn::manipulator::manipulator_managers_connector connector_;
};
} // namespace

TEST_CASE("actual examples") {
  uint64_t time_stamp = 0;
  uint64_t interval = krbn::time_utility::nano_to_absolute(NSEC_PER_MSEC) * 2;
  std::vector<post_event_to_virtual_devices::queue::event> expected;

  {
    actual_examples_helper helper;

    time_stamp = 0;

    ENQUEUE_EVENT(helper.get_input_event_queue(), 1, time_stamp += interval, tab_event, key_down, tab_event);
    ENQUEUE_EVENT(helper.get_input_event_queue(), 1, time_stamp += interval, tab_event, key_up, tab_event);

    helper.manipulate(time_stamp += interval);

    time_stamp = 0;
    expected.clear();

    ENQUEUE_KEYBOARD_EVENT(expected, tab, 1, time_stamp += interval);
    ENQUEUE_KEYBOARD_EVENT(expected, tab, 0, time_stamp += interval);

    REQUIRE(helper.get_events() == expected);
  }

  // ----------------------------------------
  // fn-up_arrow

  {
    actual_examples_helper helper;

    time_stamp = 0;

    ENQUEUE_EVENT(helper.get_input_event_queue(), 1, time_stamp += interval, fn_event, key_down, fn_event);
    ENQUEUE_EVENT(helper.get_input_event_queue(), 1, time_stamp += interval, up_arrow_event, key_down, up_arrow_event);
    ENQUEUE_EVENT(helper.get_input_event_queue(), 1, time_stamp += interval, up_arrow_event, key_up, up_arrow_event);
    ENQUEUE_EVENT(helper.get_input_event_queue(), 1, time_stamp += interval, up_arrow_event, key_down, up_arrow_event);
    ENQUEUE_EVENT(helper.get_input_event_queue(), 1, time_stamp += interval, up_arrow_event, key_up, up_arrow_event);
    ENQUEUE_EVENT(helper.get_input_event_queue(), 1, time_stamp += interval, fn_event, key_up, fn_event);

    helper.manipulate(time_stamp += interval);

    time_stamp = 0;
    expected.clear();

    ENQUEUE_KEYBOARD_EVENT(expected, fn, 1, time_stamp += interval);

    time_stamp += 2; // lazy fn key_up, lazy fn key_down
    ENQUEUE_KEYBOARD_EVENT(expected, page_up, 1, time_stamp += interval);
    time_stamp += 2; // lazy fn key_up, lazy fn key_down

    ENQUEUE_KEYBOARD_EVENT(expected, page_up, 0, time_stamp += interval);

    time_stamp += 2; // lazy fn key_up, lazy key_down
    ENQUEUE_KEYBOARD_EVENT(expected, page_up, 1, time_stamp += interval);
    time_stamp += 2; // lazy fn key_up, lazy fn key_down

    ENQUEUE_KEYBOARD_EVENT(expected, page_up, 0, time_stamp += interval);
    ENQUEUE_KEYBOARD_EVENT(expected, fn, 0, time_stamp += interval);

    REQUIRE(helper.get_events() == expected);
  }

  // ----------------------------------------
  // fn-up_arrow (release fn before up_arrow)

  {
    actual_examples_helper helper;

    time_stamp = 0;

    ENQUEUE_EVENT(helper.get_input_event_queue(), 1, time_stamp += interval, fn_event, key_down, fn_event);
    ENQUEUE_EVENT(helper.get_input_event_queue(), 1, time_stamp += interval, up_arrow_event, key_down, up_arrow_event);
    ENQUEUE_EVENT(helper.get_input_event_queue(), 1, time_stamp += interval, up_arrow_event, key_up, up_arrow_event);
    ENQUEUE_EVENT(helper.get_input_event_queue(), 1, time_stamp += interval, up_arrow_event, key_down, up_arrow_event);
    ENQUEUE_EVENT(helper.get_input_event_queue(), 1, time_stamp += interval, fn_event, key_up, fn_event);
    ENQUEUE_EVENT(helper.get_input_event_queue(), 1, time_stamp += interval, up_arrow_event, key_up, up_arrow_event);

    helper.manipulate(time_stamp += interval);

    time_stamp = 0;
    expected.clear();

    ENQUEUE_KEYBOARD_EVENT(expected, fn, 1, time_stamp += interval);

    time_stamp += 2; // lazy fn key_up, lazy fn key_down
    ENQUEUE_KEYBOARD_EVENT(expected, page_up, 1, time_stamp += interval);
    time_stamp += 2; // lazy fn key_up, lazy fn key_down

    ENQUEUE_KEYBOARD_EVENT(expected, page_up, 0, time_stamp += interval);

    time_stamp += 2; // lazy fn key_up, lazy fn key_down
    ENQUEUE_KEYBOARD_EVENT(expected, page_up, 1, time_stamp += interval);
    time_stamp += 2; // lazy fn key_up, lazy fn key_down

    ENQUEUE_KEYBOARD_EVENT(expected, fn, 0, time_stamp += interval);
    ENQUEUE_KEYBOARD_EVENT(expected, page_up, 0, time_stamp += interval);

    REQUIRE(helper.get_events() == expected);
  }

  // ----------------------------------------
  // control-p

  {
    actual_examples_helper helper;

    time_stamp = 0;

    ENQUEUE_EVENT(helper.get_input_event_queue(), 1, time_stamp += interval, left_control_event, key_down, left_control_event);
    ENQUEUE_EVENT(helper.get_input_event_queue(), 1, time_stamp += interval, p_event, key_down, p_event);
    ENQUEUE_EVENT(helper.get_input_event_queue(), 1, time_stamp += interval, p_event, key_up, p_event);
    ENQUEUE_EVENT(helper.get_input_event_queue(), 1, time_stamp += interval, p_event, key_down, p_event);
    ENQUEUE_EVENT(helper.get_input_event_queue(), 1, time_stamp += interval, p_event, key_up, p_event);
    ENQUEUE_EVENT(helper.get_input_event_queue(), 1, time_stamp += interval, left_control_event, key_up, left_control_event);
    ENQUEUE_EVENT(helper.get_input_event_queue(), 1, time_stamp += interval, p_event, key_down, tab_event);
    ENQUEUE_EVENT(helper.get_input_event_queue(), 1, time_stamp += interval, p_event, key_up, tab_event);

    helper.manipulate(time_stamp += interval);

    time_stamp = 0;
    expected.clear();

    ENQUEUE_KEYBOARD_EVENT(expected, left_control, 1, time_stamp += interval);

    time_stamp += 1; // lazy left_control key_up
    ENQUEUE_KEYBOARD_EVENT(expected, left_control, 0, time_stamp += interval);
    ENQUEUE_KEYBOARD_EVENT(expected, up_arrow, 1, time_stamp + modifier_wait); // wait after modifier
    time_stamp += 1;                                                           // lazy left_control key_down

    ENQUEUE_KEYBOARD_EVENT(expected, up_arrow, 0, time_stamp += interval);

    time_stamp += 1; // lazy left_control key_up
    ENQUEUE_KEYBOARD_EVENT(expected, up_arrow, 1, time_stamp += interval);
    time_stamp += 1; // lazy left_control key_down

    ENQUEUE_KEYBOARD_EVENT(expected, up_arrow, 0, time_stamp += interval);

    time_stamp += interval; // left_control_event key_up

    ENQUEUE_KEYBOARD_EVENT(expected, p, 1, time_stamp += interval);
    ENQUEUE_KEYBOARD_EVENT(expected, p, 0, time_stamp += interval);

    REQUIRE(helper.get_events() == expected);
  }

  // ----------------------------------------
  // control-p, tab

  {
    actual_examples_helper helper;

    time_stamp = 0;

    ENQUEUE_EVENT(helper.get_input_event_queue(), 1, time_stamp += interval, left_control_event, key_down, left_control_event);
    ENQUEUE_EVENT(helper.get_input_event_queue(), 1, time_stamp += interval, p_event, key_down, p_event);
    ENQUEUE_EVENT(helper.get_input_event_queue(), 1, time_stamp += interval, p_event, key_up, p_event);
    ENQUEUE_EVENT(helper.get_input_event_queue(), 1, time_stamp += interval, p_event, key_down, p_event);
    ENQUEUE_EVENT(helper.get_input_event_queue(), 1, time_stamp += interval, p_event, key_up, p_event);
    ENQUEUE_EVENT(helper.get_input_event_queue(), 1, time_stamp += interval, tab_event, key_down, tab_event);
    ENQUEUE_EVENT(helper.get_input_event_queue(), 1, time_stamp += interval, tab_event, key_up, tab_event);
    ENQUEUE_EVENT(helper.get_input_event_queue(), 1, time_stamp += interval, left_control_event, key_up, left_control_event);

    helper.manipulate(time_stamp += interval);

    time_stamp = 0;
    expected.clear();

    ENQUEUE_KEYBOARD_EVENT(expected, left_control, 1, time_stamp += interval);

    time_stamp += 1; // lazy left_control key_up
    ENQUEUE_KEYBOARD_EVENT(expected, left_control, 0, time_stamp += interval);
    ENQUEUE_KEYBOARD_EVENT(expected, up_arrow, 1, time_stamp + modifier_wait); // wait after modifier
    time_stamp += 1;                                                           // lazy left_control key_down

    ENQUEUE_KEYBOARD_EVENT(expected, up_arrow, 0, time_stamp += interval);

    time_stamp += 1; // lazy left_control key_up
    ENQUEUE_KEYBOARD_EVENT(expected, up_arrow, 1, time_stamp += interval);
    time_stamp += 1; // lazy left_control key_down

    ENQUEUE_KEYBOARD_EVENT(expected, up_arrow, 0, time_stamp += interval);

    ENQUEUE_KEYBOARD_EVENT(expected, left_control, 1, time_stamp += interval);
    ENQUEUE_KEYBOARD_EVENT(expected, tab, 1, time_stamp + modifier_wait);
    ENQUEUE_KEYBOARD_EVENT(expected, tab, 0, time_stamp += interval);
    ENQUEUE_KEYBOARD_EVENT(expected, left_control, 0, time_stamp += interval);

    REQUIRE(helper.get_events() == expected);
  }

  // ----------------------------------------
  // control-option-p

  {
    actual_examples_helper helper;

    time_stamp = 0;

    ENQUEUE_EVENT(helper.get_input_event_queue(), 1, time_stamp += interval, left_control_event, key_down, left_control_event);
    ENQUEUE_EVENT(helper.get_input_event_queue(), 1, time_stamp += interval, left_option_event, key_down, left_option_event);
    ENQUEUE_EVENT(helper.get_input_event_queue(), 1, time_stamp += interval, p_event, key_down, p_event);
    ENQUEUE_EVENT(helper.get_input_event_queue(), 1, time_stamp += interval, p_event, key_up, p_event);
    ENQUEUE_EVENT(helper.get_input_event_queue(), 1, time_stamp += interval, left_control_event, key_up, left_control_event);
    ENQUEUE_EVENT(helper.get_input_event_queue(), 1, time_stamp += interval, left_option_event, key_up, left_option_event);

    helper.manipulate(time_stamp += interval);

    time_stamp = 0;
    expected.clear();

    ENQUEUE_KEYBOARD_EVENT(expected, left_control, 1, time_stamp += interval);
    ENQUEUE_KEYBOARD_EVENT(expected, left_option, 1, time_stamp += interval);
    ENQUEUE_KEYBOARD_EVENT(expected, p, 1, time_stamp += interval);
    ENQUEUE_KEYBOARD_EVENT(expected, p, 0, time_stamp += interval);
    ENQUEUE_KEYBOARD_EVENT(expected, left_control, 0, time_stamp += interval);
    ENQUEUE_KEYBOARD_EVENT(expected, left_option, 0, time_stamp += interval);

    REQUIRE(helper.get_events() == expected);
  }

  // ----------------------------------------
  // control-shift-p

  {
    actual_examples_helper helper;

    time_stamp = 0;

    ENQUEUE_EVENT(helper.get_input_event_queue(), 1, time_stamp += interval, left_control_event, key_down, left_control_event);
    ENQUEUE_EVENT(helper.get_input_event_queue(), 1, time_stamp += interval, left_shift_event, key_down, left_shift_event);
    ENQUEUE_EVENT(helper.get_input_event_queue(), 1, time_stamp += interval, p_event, key_down, p_event);
    ENQUEUE_EVENT(helper.get_input_event_queue(), 1, time_stamp += interval, p_event, key_up, p_event);
    ENQUEUE_EVENT(helper.get_input_event_queue(), 1, time_stamp += interval, p_event, key_down, p_event);
    ENQUEUE_EVENT(helper.get_input_event_queue(), 1, time_stamp += interval, p_event, key_up, p_event);
    ENQUEUE_EVENT(helper.get_input_event_queue(), 1, time_stamp += interval, left_control_event, key_up, left_control_event);
    ENQUEUE_EVENT(helper.get_input_event_queue(), 1, time_stamp += interval, p_event, key_down, tab_event);
    ENQUEUE_EVENT(helper.get_input_event_queue(), 1, time_stamp += interval, p_event, key_up, tab_event);

    helper.manipulate(time_stamp += interval);

    time_stamp = 0;
    expected.clear();

    ENQUEUE_KEYBOARD_EVENT(expected, left_control, 1, time_stamp += interval);
    ENQUEUE_KEYBOARD_EVENT(expected, left_shift, 1, time_stamp += interval);

    time_stamp += 1; // lazy left_control key_up
    ENQUEUE_KEYBOARD_EVENT(expected, left_control, 0, time_stamp += interval);
    ENQUEUE_KEYBOARD_EVENT(expected, up_arrow, 1, time_stamp + modifier_wait); // wait after modifier
    time_stamp += 1;                                                           // lazy left_control key_down

    ENQUEUE_KEYBOARD_EVENT(expected, up_arrow, 0, time_stamp += interval);

    time_stamp += 1; // lazy left_control key_up
    ENQUEUE_KEYBOARD_EVENT(expected, up_arrow, 1, time_stamp += interval);
    time_stamp += 1; // lazy left_control key_down

    ENQUEUE_KEYBOARD_EVENT(expected, up_arrow, 0, time_stamp += interval);

    time_stamp += interval; // left_control_event key_up

    ENQUEUE_KEYBOARD_EVENT(expected, p, 1, time_stamp += interval);
    ENQUEUE_KEYBOARD_EVENT(expected, p, 0, time_stamp += interval);

    REQUIRE(helper.get_events() == expected);
  }
}

int main(int argc, char* const argv[]) {
  krbn::thread_utility::register_main_thread();
  return Catch::Session().run(argc, argv);
}
