#define CATCH_CONFIG_RUNNER
#include "../../vendor/catch/catch.hpp"

#include "manipulator/details/types.hpp"
#include "thread_utility.hpp"
#include <boost/optional/optional_io.hpp>

namespace {
krbn::modifier_flag_manager::active_modifier_flag left_command_1(krbn::modifier_flag_manager::active_modifier_flag::type::increase,
                                                                 krbn::modifier_flag::left_command,
                                                                 krbn::device_id(1));
krbn::modifier_flag_manager::active_modifier_flag left_control_1(krbn::modifier_flag_manager::active_modifier_flag::type::increase,
                                                                 krbn::modifier_flag::left_control,
                                                                 krbn::device_id(1));
krbn::modifier_flag_manager::active_modifier_flag left_option_1(krbn::modifier_flag_manager::active_modifier_flag::type::increase,
                                                                krbn::modifier_flag::left_option,
                                                                krbn::device_id(1));
krbn::modifier_flag_manager::active_modifier_flag left_shift_1(krbn::modifier_flag_manager::active_modifier_flag::type::increase,
                                                               krbn::modifier_flag::left_shift,
                                                               krbn::device_id(1));
krbn::modifier_flag_manager::active_modifier_flag right_shift_1(krbn::modifier_flag_manager::active_modifier_flag::type::increase,
                                                                krbn::modifier_flag::right_shift,
                                                                krbn::device_id(1));
} // namespace

TEST_CASE("event_definition.make_modifiers") {
  using krbn::manipulator::details::event_definition;

  {
    nlohmann::json json({"left_command", "left_shift", "fn", "any"});
    auto actual = event_definition::make_modifiers(json);
    auto expected = std::unordered_set<event_definition::modifier>({
        event_definition::modifier::left_command,
        event_definition::modifier::left_shift,
        event_definition::modifier::fn,
        event_definition::modifier::any,
    });
    REQUIRE(actual == expected);
  }

  {
    nlohmann::json json("left_command");
    auto actual = event_definition::make_modifiers(json);
    auto expected = std::unordered_set<event_definition::modifier>({
        event_definition::modifier::left_command,
    });
    REQUIRE(actual == expected);
  }

  {
    nlohmann::json json("unknown");
    auto actual = event_definition::make_modifiers(json);
    auto expected = std::unordered_set<event_definition::modifier>({});
    REQUIRE(actual == expected);
  }

  {
    nlohmann::json json(nullptr);
    auto actual = event_definition::make_modifiers(json);
    auto expected = std::unordered_set<event_definition::modifier>({});
    REQUIRE(actual == expected);
  }
}

TEST_CASE("event_definition.get_modifier") {
  using krbn::manipulator::details::event_definition;

  REQUIRE(event_definition::get_modifier(krbn::modifier_flag::zero) == event_definition::modifier::end_);
  REQUIRE(event_definition::get_modifier(krbn::modifier_flag::left_shift) == event_definition::modifier::left_shift);
}

TEST_CASE("event_definition.test_modifier") {
  using krbn::manipulator::details::event_definition;
  using krbn::manipulator::details::from_event_definition;

  {
    krbn::modifier_flag_manager modifier_flag_manager;
    modifier_flag_manager.push_back_active_modifier_flag(left_command_1);

    {
      auto actual = from_event_definition::test_modifier(modifier_flag_manager, event_definition::modifier::left_shift);
      auto expected = std::make_pair(false, krbn::modifier_flag::zero);
      REQUIRE(actual == expected);
    }
    {
      auto actual = from_event_definition::test_modifier(modifier_flag_manager, event_definition::modifier::left_command);
      auto expected = std::make_pair(true, krbn::modifier_flag::left_command);
      REQUIRE(actual == expected);
    }
    {
      auto actual = from_event_definition::test_modifier(modifier_flag_manager, event_definition::modifier::right_command);
      auto expected = std::make_pair(false, krbn::modifier_flag::zero);
      REQUIRE(actual == expected);
    }
    {
      auto actual = from_event_definition::test_modifier(modifier_flag_manager, event_definition::modifier::command);
      auto expected = std::make_pair(true, krbn::modifier_flag::left_command);
      REQUIRE(actual == expected);
    }
    {
      auto actual = from_event_definition::test_modifier(modifier_flag_manager, event_definition::modifier::shift);
      auto expected = std::make_pair(false, krbn::modifier_flag::zero);
      REQUIRE(actual == expected);
    }
  }
  {
    krbn::modifier_flag_manager modifier_flag_manager;
    modifier_flag_manager.push_back_active_modifier_flag(right_shift_1);

    {
      auto actual = from_event_definition::test_modifier(modifier_flag_manager, event_definition::modifier::left_shift);
      auto expected = std::make_pair(false, krbn::modifier_flag::zero);
      REQUIRE(actual == expected);
    }
    {
      auto actual = from_event_definition::test_modifier(modifier_flag_manager, event_definition::modifier::right_shift);
      auto expected = std::make_pair(true, krbn::modifier_flag::right_shift);
      REQUIRE(actual == expected);
    }
    {
      auto actual = from_event_definition::test_modifier(modifier_flag_manager, event_definition::modifier::shift);
      auto expected = std::make_pair(true, krbn::modifier_flag::right_shift);
      REQUIRE(actual == expected);
    }
  }
}

TEST_CASE("event_definition.test_modifiers") {
  using krbn::manipulator::details::event_definition;
  using krbn::manipulator::details::from_event_definition;

  // empty

  {
    std::unordered_set<event_definition::modifier> mandatory_modifiers;
    std::unordered_set<event_definition::modifier> optional_modifiers;
    from_event_definition event_definition(krbn::key_code::spacebar, mandatory_modifiers, optional_modifiers);

    REQUIRE(event_definition.get_key_code() == krbn::key_code::spacebar);
    REQUIRE(event_definition.get_mandatory_modifiers() == mandatory_modifiers);
    REQUIRE(event_definition.get_optional_modifiers() == optional_modifiers);

    {
      krbn::modifier_flag_manager modifier_flag_manager;
      REQUIRE(event_definition.test_modifiers(modifier_flag_manager) == std::unordered_set<krbn::modifier_flag>({}));
    }
    {
      krbn::modifier_flag_manager modifier_flag_manager;
      modifier_flag_manager.push_back_active_modifier_flag(left_shift_1);
      REQUIRE(event_definition.test_modifiers(modifier_flag_manager) == boost::none);
    }
  }

  // empty (modifier key)

  {
    std::unordered_set<event_definition::modifier> mandatory_modifiers;
    std::unordered_set<event_definition::modifier> optional_modifiers;
    from_event_definition event_definition(krbn::key_code::left_shift, mandatory_modifiers, optional_modifiers);

    REQUIRE(event_definition.get_key_code() == krbn::key_code::left_shift);
    REQUIRE(event_definition.get_mandatory_modifiers() == mandatory_modifiers);
    REQUIRE(event_definition.get_optional_modifiers() == optional_modifiers);

    {
      krbn::modifier_flag_manager modifier_flag_manager;
      REQUIRE(event_definition.test_modifiers(modifier_flag_manager) == std::unordered_set<krbn::modifier_flag>({}));
    }
    {
      krbn::modifier_flag_manager modifier_flag_manager;
      modifier_flag_manager.push_back_active_modifier_flag(left_shift_1);
      REQUIRE(event_definition.test_modifiers(modifier_flag_manager) == boost::none);
    }
  }

  // mandatory_modifiers any

  {
    std::unordered_set<event_definition::modifier> mandatory_modifiers({
        event_definition::modifier::any,
    });
    std::unordered_set<event_definition::modifier> optional_modifiers;
    from_event_definition event_definition(krbn::key_code::spacebar, mandatory_modifiers, optional_modifiers);

    REQUIRE(event_definition.get_key_code() == krbn::key_code::spacebar);
    REQUIRE(event_definition.get_mandatory_modifiers() == mandatory_modifiers);
    REQUIRE(event_definition.get_optional_modifiers() == optional_modifiers);

    {
      krbn::modifier_flag_manager modifier_flag_manager;
      REQUIRE(event_definition.test_modifiers(modifier_flag_manager) == std::unordered_set<krbn::modifier_flag>({}));
    }
    {
      krbn::modifier_flag_manager modifier_flag_manager;
      modifier_flag_manager.push_back_active_modifier_flag(left_shift_1);
      REQUIRE(event_definition.test_modifiers(modifier_flag_manager) == std::unordered_set<krbn::modifier_flag>({
                                                                            krbn::modifier_flag::left_shift,
                                                                        }));
    }
    {
      krbn::modifier_flag_manager modifier_flag_manager;
      modifier_flag_manager.push_back_active_modifier_flag(left_command_1);
      modifier_flag_manager.push_back_active_modifier_flag(left_shift_1);
      REQUIRE(event_definition.test_modifiers(modifier_flag_manager) == std::unordered_set<krbn::modifier_flag>({
                                                                            krbn::modifier_flag::left_command,
                                                                            krbn::modifier_flag::left_shift,
                                                                        }));
    }
  }

  // optional_modifiers any

  {
    std::unordered_set<event_definition::modifier> mandatory_modifiers;
    std::unordered_set<event_definition::modifier> optional_modifiers({
        event_definition::modifier::any,
    });
    from_event_definition event_definition(krbn::key_code::spacebar, mandatory_modifiers, optional_modifiers);

    REQUIRE(event_definition.get_key_code() == krbn::key_code::spacebar);
    REQUIRE(event_definition.get_mandatory_modifiers() == mandatory_modifiers);
    REQUIRE(event_definition.get_optional_modifiers() == optional_modifiers);

    {
      krbn::modifier_flag_manager modifier_flag_manager;
      REQUIRE(event_definition.test_modifiers(modifier_flag_manager) == std::unordered_set<krbn::modifier_flag>({}));
    }
    {
      krbn::modifier_flag_manager modifier_flag_manager;
      modifier_flag_manager.push_back_active_modifier_flag(left_shift_1);
      REQUIRE(event_definition.test_modifiers(modifier_flag_manager) == std::unordered_set<krbn::modifier_flag>({}));
    }
    {
      krbn::modifier_flag_manager modifier_flag_manager;
      modifier_flag_manager.push_back_active_modifier_flag(left_command_1);
      modifier_flag_manager.push_back_active_modifier_flag(left_shift_1);
      REQUIRE(event_definition.test_modifiers(modifier_flag_manager) == std::unordered_set<krbn::modifier_flag>({}));
    }
  }

  // mandatory_modifiers and optional_modifiers

  {
    std::unordered_set<event_definition::modifier> mandatory_modifiers({
        event_definition::modifier::control,
    });
    std::unordered_set<event_definition::modifier> optional_modifiers({
        event_definition::modifier::shift,
    });
    from_event_definition event_definition(krbn::key_code::p, mandatory_modifiers, optional_modifiers);

    REQUIRE(event_definition.get_key_code() == krbn::key_code::p);
    REQUIRE(event_definition.get_mandatory_modifiers() == mandatory_modifiers);
    REQUIRE(event_definition.get_optional_modifiers() == optional_modifiers);

    {
      krbn::modifier_flag_manager modifier_flag_manager;
      REQUIRE(event_definition.test_modifiers(modifier_flag_manager) == boost::none);
    }
    {
      krbn::modifier_flag_manager modifier_flag_manager;
      modifier_flag_manager.push_back_active_modifier_flag(left_control_1);
      REQUIRE(event_definition.test_modifiers(modifier_flag_manager) == std::unordered_set<krbn::modifier_flag>({
                                                                            krbn::modifier_flag::left_control,
                                                                        }));
    }
    {
      krbn::modifier_flag_manager modifier_flag_manager;
      modifier_flag_manager.push_back_active_modifier_flag(left_control_1);
      modifier_flag_manager.push_back_active_modifier_flag(left_shift_1);
      REQUIRE(event_definition.test_modifiers(modifier_flag_manager) == std::unordered_set<krbn::modifier_flag>({
                                                                            krbn::modifier_flag::left_control,
                                                                        }));
    }
    {
      krbn::modifier_flag_manager modifier_flag_manager;
      modifier_flag_manager.push_back_active_modifier_flag(left_control_1);
      modifier_flag_manager.push_back_active_modifier_flag(left_option_1);
      REQUIRE(event_definition.test_modifiers(modifier_flag_manager) == boost::none);
    }
  }

  // optional_modifiers any with mandatory_modifiers

  {
    std::unordered_set<event_definition::modifier> mandatory_modifiers({
        event_definition::modifier::left_shift,
    });
    std::unordered_set<event_definition::modifier> optional_modifiers({
        event_definition::modifier::any,
    });
    from_event_definition event_definition(krbn::key_code::spacebar, mandatory_modifiers, optional_modifiers);

    REQUIRE(event_definition.get_key_code() == krbn::key_code::spacebar);
    REQUIRE(event_definition.get_mandatory_modifiers() == mandatory_modifiers);
    REQUIRE(event_definition.get_optional_modifiers() == optional_modifiers);

    {
      krbn::modifier_flag_manager modifier_flag_manager;
      REQUIRE(event_definition.test_modifiers(modifier_flag_manager) == boost::none);
    }
    {
      krbn::modifier_flag_manager modifier_flag_manager;
      modifier_flag_manager.push_back_active_modifier_flag(left_shift_1);
      REQUIRE(event_definition.test_modifiers(modifier_flag_manager) == std::unordered_set<krbn::modifier_flag>({
                                                                            krbn::modifier_flag::left_shift,
                                                                        }));
    }
    {
      krbn::modifier_flag_manager modifier_flag_manager;
      modifier_flag_manager.push_back_active_modifier_flag(left_command_1);
      modifier_flag_manager.push_back_active_modifier_flag(left_shift_1);
      REQUIRE(event_definition.test_modifiers(modifier_flag_manager) == std::unordered_set<krbn::modifier_flag>({
                                                                            krbn::modifier_flag::left_shift,
                                                                        }));
    }
  }

  // mandatory_modifiers strict matching

  {
    std::unordered_set<event_definition::modifier> mandatory_modifiers({
        event_definition::modifier::left_shift,
    });
    std::unordered_set<event_definition::modifier> optional_modifiers;
    from_event_definition event_definition(krbn::key_code::spacebar, mandatory_modifiers, optional_modifiers);

    REQUIRE(event_definition.get_key_code() == krbn::key_code::spacebar);
    REQUIRE(event_definition.get_mandatory_modifiers() == mandatory_modifiers);
    REQUIRE(event_definition.get_optional_modifiers() == optional_modifiers);

    {
      krbn::modifier_flag_manager modifier_flag_manager;
      modifier_flag_manager.push_back_active_modifier_flag(left_command_1);
      REQUIRE(event_definition.test_modifiers(modifier_flag_manager) == boost::none);
    }
    {
      krbn::modifier_flag_manager modifier_flag_manager;
      modifier_flag_manager.push_back_active_modifier_flag(left_shift_1);
      REQUIRE(event_definition.test_modifiers(modifier_flag_manager) == std::unordered_set<krbn::modifier_flag>({
                                                                            krbn::modifier_flag::left_shift,
                                                                        }));
    }
    {
      krbn::modifier_flag_manager modifier_flag_manager;
      modifier_flag_manager.push_back_active_modifier_flag(left_shift_1);
      modifier_flag_manager.push_back_active_modifier_flag(right_shift_1);
      REQUIRE(event_definition.test_modifiers(modifier_flag_manager) == boost::none);
    }
  }

  // mandatory_modifiers (modifier::shift)

  {
    std::unordered_set<event_definition::modifier> mandatory_modifiers({
        event_definition::modifier::shift,
    });
    std::unordered_set<event_definition::modifier> optional_modifiers({
        event_definition::modifier::any,
    });
    from_event_definition event_definition(krbn::key_code::spacebar, mandatory_modifiers, optional_modifiers);

    REQUIRE(event_definition.get_key_code() == krbn::key_code::spacebar);
    REQUIRE(event_definition.get_mandatory_modifiers() == mandatory_modifiers);
    REQUIRE(event_definition.get_optional_modifiers() == optional_modifiers);

    {
      krbn::modifier_flag_manager modifier_flag_manager;
      modifier_flag_manager.push_back_active_modifier_flag(left_command_1);
      REQUIRE(event_definition.test_modifiers(modifier_flag_manager) == boost::none);
    }
    {
      krbn::modifier_flag_manager modifier_flag_manager;
      modifier_flag_manager.push_back_active_modifier_flag(left_shift_1);
      REQUIRE(event_definition.test_modifiers(modifier_flag_manager) == std::unordered_set<krbn::modifier_flag>({
                                                                            krbn::modifier_flag::left_shift,
                                                                        }));
    }
    {
      krbn::modifier_flag_manager modifier_flag_manager;
      modifier_flag_manager.push_back_active_modifier_flag(right_shift_1);
      REQUIRE(event_definition.test_modifiers(modifier_flag_manager) == std::unordered_set<krbn::modifier_flag>({
                                                                            krbn::modifier_flag::right_shift,
                                                                        }));
    }
    {
      krbn::modifier_flag_manager modifier_flag_manager;
      modifier_flag_manager.push_back_active_modifier_flag(left_shift_1);
      modifier_flag_manager.push_back_active_modifier_flag(right_shift_1);
      REQUIRE(event_definition.test_modifiers(modifier_flag_manager) == std::unordered_set<krbn::modifier_flag>({
                                                                            krbn::modifier_flag::left_shift,
                                                                        }));
    }
    {
      krbn::modifier_flag_manager modifier_flag_manager;
      modifier_flag_manager.push_back_active_modifier_flag(left_command_1);
      modifier_flag_manager.push_back_active_modifier_flag(left_shift_1);
      modifier_flag_manager.push_back_active_modifier_flag(right_shift_1);
      REQUIRE(event_definition.test_modifiers(modifier_flag_manager) == std::unordered_set<krbn::modifier_flag>({
                                                                            krbn::modifier_flag::left_shift,
                                                                        }));
    }
  }

  // mandatory_modifiers strict matching (modifier::shift)

  {
    std::unordered_set<event_definition::modifier> mandatory_modifiers({
        event_definition::modifier::shift,
    });
    std::unordered_set<event_definition::modifier> optional_modifiers;
    from_event_definition event_definition(krbn::key_code::spacebar, mandatory_modifiers, optional_modifiers);

    REQUIRE(event_definition.get_key_code() == krbn::key_code::spacebar);
    REQUIRE(event_definition.get_mandatory_modifiers() == mandatory_modifiers);
    REQUIRE(event_definition.get_optional_modifiers() == optional_modifiers);

    {
      krbn::modifier_flag_manager modifier_flag_manager;
      modifier_flag_manager.push_back_active_modifier_flag(left_command_1);
      REQUIRE(event_definition.test_modifiers(modifier_flag_manager) == boost::none);
    }
    {
      krbn::modifier_flag_manager modifier_flag_manager;
      modifier_flag_manager.push_back_active_modifier_flag(left_shift_1);
      REQUIRE(event_definition.test_modifiers(modifier_flag_manager) == std::unordered_set<krbn::modifier_flag>({
                                                                            krbn::modifier_flag::left_shift,
                                                                        }));
    }
    {
      krbn::modifier_flag_manager modifier_flag_manager;
      modifier_flag_manager.push_back_active_modifier_flag(right_shift_1);
      REQUIRE(event_definition.test_modifiers(modifier_flag_manager) == std::unordered_set<krbn::modifier_flag>({
                                                                            krbn::modifier_flag::right_shift,
                                                                        }));
    }
    {
      krbn::modifier_flag_manager modifier_flag_manager;
      modifier_flag_manager.push_back_active_modifier_flag(left_shift_1);
      modifier_flag_manager.push_back_active_modifier_flag(right_shift_1);
      REQUIRE(event_definition.test_modifiers(modifier_flag_manager) == std::unordered_set<krbn::modifier_flag>({
                                                                            krbn::modifier_flag::left_shift,
                                                                        }));
    }
    {
      krbn::modifier_flag_manager modifier_flag_manager;
      modifier_flag_manager.push_back_active_modifier_flag(left_command_1);
      modifier_flag_manager.push_back_active_modifier_flag(left_shift_1);
      modifier_flag_manager.push_back_active_modifier_flag(right_shift_1);
      REQUIRE(event_definition.test_modifiers(modifier_flag_manager) == boost::none);
    }
  }
}

TEST_CASE("manipulator.details.from_event_definition") {
  {
    nlohmann::json json;
    krbn::manipulator::details::from_event_definition event_definition(json);
    REQUIRE(event_definition.get_type() == krbn::manipulator::details::event_definition::type::none);
    REQUIRE(event_definition.get_mandatory_modifiers().size() == 0);
    REQUIRE(event_definition.get_optional_modifiers().size() == 0);
    REQUIRE(!(event_definition.to_event()));
  }
  {
    nlohmann::json json({
        {"key_code", "spacebar"},
        {"modifiers", {
                          {"mandatory", {
                                            "shift", "left_command",
                                        }},
                          {"optional", {
                                           "any",
                                       }},
                      }},
    });
    krbn::manipulator::details::from_event_definition event_definition(json);
    REQUIRE(event_definition.get_type() == krbn::manipulator::details::event_definition::type::key_code);
    REQUIRE(event_definition.get_key_code() == krbn::key_code::spacebar);
    REQUIRE(event_definition.get_pointing_button() == boost::none);
    REQUIRE(event_definition.get_mandatory_modifiers() == std::unordered_set<krbn::manipulator::details::event_definition::modifier>({
                                                              krbn::manipulator::details::event_definition::modifier::shift,
                                                              krbn::manipulator::details::event_definition::modifier::left_command,
                                                          }));
    REQUIRE(event_definition.get_optional_modifiers() == std::unordered_set<krbn::manipulator::details::event_definition::modifier>({
                                                             krbn::manipulator::details::event_definition::modifier::any,
                                                         }));
    REQUIRE(event_definition.to_event() == krbn::event_queue::queued_event::event(krbn::key_code::spacebar));
  }
  {
    nlohmann::json json({
        {"key_code", "right_option"},
        {"modifiers", {
                          {"mandatory", {
                                            "shift", "left_command",
                                            // duplicated
                                            "shift", "left_command",
                                        }},
                          {"optional", {
                                           "any",
                                           // duplicated
                                           "any",
                                       }},
                      }},
    });
    krbn::manipulator::details::from_event_definition event_definition(json);
    REQUIRE(event_definition.get_type() == krbn::manipulator::details::event_definition::type::key_code);
    REQUIRE(event_definition.get_key_code() == krbn::key_code::right_option);
    REQUIRE(event_definition.get_pointing_button() == boost::none);
    REQUIRE(event_definition.get_mandatory_modifiers() == std::unordered_set<krbn::manipulator::details::event_definition::modifier>({
                                                              krbn::manipulator::details::event_definition::modifier::shift,
                                                              krbn::manipulator::details::event_definition::modifier::left_command,
                                                          }));
    REQUIRE(event_definition.get_optional_modifiers() == std::unordered_set<krbn::manipulator::details::event_definition::modifier>({
                                                             krbn::manipulator::details::event_definition::modifier::any,
                                                         }));
  }
  {
    nlohmann::json json({
        {"key_code", nlohmann::json::array()},
        {"modifiers", {
                          {"mandatory", {
                                            "dummy",
                                        }},
                      }},
    });
    krbn::manipulator::details::from_event_definition event_definition(json);
    REQUIRE(event_definition.get_type() == krbn::manipulator::details::event_definition::type::none);
    REQUIRE(event_definition.get_key_code() == boost::none);
    REQUIRE(event_definition.get_pointing_button() == boost::none);
    REQUIRE(event_definition.get_mandatory_modifiers().size() == 0);
  }
}

TEST_CASE("manipulator.details.to_event_definition") {
  {
    nlohmann::json json;
    krbn::manipulator::details::to_event_definition event_definition(json);
    REQUIRE(event_definition.get_type() == krbn::manipulator::details::event_definition::type::none);
    REQUIRE(event_definition.get_modifiers().size() == 0);
    REQUIRE(!(event_definition.to_event()));
  }
  {
    nlohmann::json json({
        {"key_code", "spacebar"},
        {"modifiers", {
                          "shift", "left_command",
                      }},
    });
    krbn::manipulator::details::to_event_definition event_definition(json);
    REQUIRE(event_definition.get_type() == krbn::manipulator::details::event_definition::type::key_code);
    REQUIRE(event_definition.get_key_code() == krbn::key_code::spacebar);
    REQUIRE(event_definition.get_pointing_button() == boost::none);
    REQUIRE(event_definition.get_modifiers() == std::unordered_set<krbn::manipulator::details::event_definition::modifier>({
                                                    krbn::manipulator::details::event_definition::modifier::shift,
                                                    krbn::manipulator::details::event_definition::modifier::left_command,
                                                }));
    REQUIRE(event_definition.to_event() == krbn::event_queue::queued_event::event(krbn::key_code::spacebar));
  }
  {
    nlohmann::json json({
        {"key_code", "right_option"},
        {"modifiers", {
                          "shift", "left_command",
                          // duplicated
                          "shift", "left_command",
                      }},
    });
    krbn::manipulator::details::to_event_definition event_definition(json);
    REQUIRE(event_definition.get_type() == krbn::manipulator::details::event_definition::type::key_code);
    REQUIRE(event_definition.get_key_code() == krbn::key_code::right_option);
    REQUIRE(event_definition.get_pointing_button() == boost::none);
    REQUIRE(event_definition.get_modifiers() == std::unordered_set<krbn::manipulator::details::event_definition::modifier>({
                                                    krbn::manipulator::details::event_definition::modifier::shift,
                                                    krbn::manipulator::details::event_definition::modifier::left_command,
                                                }));
  }
  {
    nlohmann::json json({
        {"key_code", nlohmann::json::array()},
        {"modifiers", {
                          "dummy",
                      }},
    });
    krbn::manipulator::details::to_event_definition event_definition(json);
    REQUIRE(event_definition.get_type() == krbn::manipulator::details::event_definition::type::none);
    REQUIRE(event_definition.get_key_code() == boost::none);
    REQUIRE(event_definition.get_pointing_button() == boost::none);
    REQUIRE(event_definition.get_modifiers().size() == 0);
  }
}

int main(int argc, char* const argv[]) {
  krbn::thread_utility::register_main_thread();
  return Catch::Session().run(argc, argv);
}
