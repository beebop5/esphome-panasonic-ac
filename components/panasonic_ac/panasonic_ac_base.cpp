#include "panasonic_ac_base.h"

#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/core/log.h"

namespace esphome {
namespace panasonic_ac {

static const char *const TAG = "panasonic_ac";

climate::ClimateTraits PanasonicACBase::traits() {
  auto traits = climate::ClimateTraits();

  // Use new feature flags API instead of deprecated methods
  traits.add_feature_flags(climate::CLIMATE_SUPPORTS_CURRENT_TEMPERATURE);
  // Note: CLIMATE_SUPPORTS_ACTION and CLIMATE_REQUIRES_TWO_POINT_TARGET_TEMPERATURE are not added (false)
  traits.set_visual_min_temperature(MIN_TEMPERATURE);
  traits.set_visual_max_temperature(MAX_TEMPERATURE);
  traits.set_visual_temperature_step(TEMPERATURE_STEP);

  traits.set_supported_modes({climate::CLIMATE_MODE_OFF, climate::CLIMATE_MODE_HEAT_COOL, climate::CLIMATE_MODE_COOL,
                              climate::CLIMATE_MODE_HEAT, climate::CLIMATE_MODE_FAN_ONLY, climate::CLIMATE_MODE_DRY});

  traits.set_supported_custom_fan_modes({"Automatic", "1", "2", "3", "4", "5"});

  traits.set_supported_swing_modes({climate::CLIMATE_SWING_OFF, climate::CLIMATE_SWING_BOTH,
                                    climate::CLIMATE_SWING_VERTICAL, climate::CLIMATE_SWING_HORIZONTAL});

  traits.set_supported_custom_presets({"Normal", "Powerful", "Quiet"});

  return traits;
}

void PanasonicACBase::setup() {
  // Initialize times
  this->init_time_ = millis();
  this->last_packet_sent_ = millis();

  ESP_LOGI(TAG, "Panasonic AC component v%s starting...", VERSION);
}

void PanasonicACBase::loop() {
  read_data();  // Read any available data from UART
}

void PanasonicACBase::read_data() {
  // Limit reads to prevent blocking - read max 10 bytes per call
  uint8_t bytes_read = 0;
  const uint8_t MAX_BYTES_PER_CALL = 10;
  
  while (available() && bytes_read < MAX_BYTES_PER_CALL) {
    uint8_t c;
    this->read_byte(&c);
    this->rx_buffer_.push_back(c);
    this->last_read_ = millis();
    bytes_read++;
  }
}

bool PanasonicACBase::is_valid_temperature(int8_t temperature) {
  // Check if temperature value is valid (not an error code and within acceptable range)
  return temperature != TEMP_SENSOR_NOT_AVAILABLE && 
         temperature != TEMP_SENSOR_ERROR && 
         temperature != TEMP_SENSOR_INVALID && 
         temperature <= TEMPERATURE_THRESHOLD;
}

void PanasonicACBase::update_outside_temperature(int8_t temperature) {
  if (!is_valid_temperature(temperature)) {
    return;
  }

  if (this->outside_temperature_sensor_ != nullptr && 
      this->outside_temperature_sensor_->state != (float)temperature) {
    this->outside_temperature_sensor_->publish_state((float)temperature);
  }
}

void PanasonicACBase::update_current_temperature(int8_t temperature) {
  if (!is_valid_temperature(temperature)) {
    ESP_LOGD(TAG, "Inside temperature sensor not available (value: %d)", temperature);
    return;
  }

  this->current_temperature = temperature;
}

void PanasonicACBase::update_target_temperature(uint8_t raw_value) {
  if (!is_valid_temperature(raw_value)) {
    ESP_LOGD(TAG, "Target temperature sensor not available (value: %d)", raw_value);
    return;
  }
  
  float temperature = raw_value * TEMPERATURE_STEP;
  this->target_temperature = temperature;
}

void PanasonicACBase::update_swing_horizontal(const std::string &swing) {
  this->horizontal_swing_state_ = swing;

  if (this->horizontal_swing_select_ != nullptr &&
      std::string(this->horizontal_swing_select_->current_option()) != this->horizontal_swing_state_) {
    this->horizontal_swing_select_->publish_state(this->horizontal_swing_state_);
  }
}

void PanasonicACBase::update_swing_vertical(const std::string &swing) {
  this->vertical_swing_state_ = swing;

  if (this->vertical_swing_select_ != nullptr && 
      std::string(this->vertical_swing_select_->current_option()) != this->vertical_swing_state_) {
    this->vertical_swing_select_->publish_state(this->vertical_swing_state_);
  }
}

void PanasonicACBase::update_nanoex(bool nanoex) {
  if (this->nanoex_switch_ != nullptr) {
    this->nanoex_state_ = nanoex;
    this->nanoex_switch_->publish_state(this->nanoex_state_);
  }
}

void PanasonicACBase::update_powerful(bool powerful) {
  if (this->powerful_switch_ != nullptr) {
    this->powerful_state_ = powerful;
    this->powerful_switch_->publish_state(this->powerful_state_);
  }
}

void PanasonicACBase::update_quiet(bool quiet) {
  if (this->quiet_switch_ != nullptr) {
    this->quiet_state_ = quiet;
    this->quiet_switch_->publish_state(this->quiet_state_);
  }
}

void PanasonicACBase::update_fan_mode_status(const std::string &fan_mode) {
  if (this->fan_mode_status_text_sensor_ != nullptr &&
      this->fan_mode_status_text_sensor_->state != fan_mode) {
    this->fan_mode_status_text_sensor_->publish_state(fan_mode);
  }
}

void PanasonicACBase::update_preset_status(const std::string &preset) {
  if (this->preset_status_text_sensor_ != nullptr &&
      this->preset_status_text_sensor_->state != preset) {
    this->preset_status_text_sensor_->publish_state(preset);
  }
}

climate::ClimateAction PanasonicACBase::determine_action() {
  // Determine the current climate action based on mode and temperature conditions
  if (this->mode == climate::CLIMATE_MODE_OFF) {
    return climate::CLIMATE_ACTION_OFF;
  } else if (this->mode == climate::CLIMATE_MODE_FAN_ONLY) {
    return climate::CLIMATE_ACTION_FAN;
  } else if (this->mode == climate::CLIMATE_MODE_DRY) {
    return climate::CLIMATE_ACTION_DRYING;
  } else if ((this->mode == climate::CLIMATE_MODE_COOL || this->mode == climate::CLIMATE_MODE_HEAT_COOL) &&
             this->current_temperature + TEMPERATURE_TOLERANCE >= this->target_temperature) {
    return climate::CLIMATE_ACTION_COOLING;
  } else if ((this->mode == climate::CLIMATE_MODE_HEAT || this->mode == climate::CLIMATE_MODE_HEAT_COOL) &&
             this->current_temperature - TEMPERATURE_TOLERANCE <= this->target_temperature) {
    return climate::CLIMATE_ACTION_HEATING;
  } else {
    return climate::CLIMATE_ACTION_IDLE;
  }
}

// Sensor and component setup methods
void PanasonicACBase::set_outside_temperature_sensor(sensor::Sensor *outside_temperature_sensor) {
  this->outside_temperature_sensor_ = outside_temperature_sensor;
}

void PanasonicACBase::set_vertical_swing_select(select::Select *vertical_swing_select) {
  this->vertical_swing_select_ = vertical_swing_select;
  this->vertical_swing_select_->add_on_state_callback([this](const std::string &value, size_t index) {
    if (value == this->vertical_swing_state_)
      return;
    this->on_vertical_swing_change(value);
  });
}

void PanasonicACBase::set_horizontal_swing_select(select::Select *horizontal_swing_select) {
  this->horizontal_swing_select_ = horizontal_swing_select;
  this->horizontal_swing_select_->add_on_state_callback([this](const std::string &value, size_t index) {
    if (value == this->horizontal_swing_state_)
      return;
    this->on_horizontal_swing_change(value);
  });
}

void PanasonicACBase::set_nanoex_switch(switch_::Switch *nanoex_switch) {
  this->nanoex_switch_ = nanoex_switch;
  this->nanoex_switch_->add_on_state_callback([this](bool state) {
    if (state == this->nanoex_state_)
      return;
    this->on_nanoex_change(state);
  });
}

void PanasonicACBase::set_powerful_switch(switch_::Switch *powerful_switch) {
  this->powerful_switch_ = powerful_switch;
  this->powerful_switch_->add_on_state_callback([this](bool state) {
    if (state == this->powerful_state_)
      return;
    this->on_powerful_change(state);
  });
}

void PanasonicACBase::set_quiet_switch(switch_::Switch *quiet_switch) {
  this->quiet_switch_ = quiet_switch;
  this->quiet_switch_->add_on_state_callback([this](bool state) {
    if (state == this->quiet_state_)
      return;
    this->on_quiet_change(state);
  });
}

void PanasonicACBase::set_fan_mode_status_text_sensor(text_sensor::TextSensor *fan_mode_status) {
  this->fan_mode_status_text_sensor_ = fan_mode_status;
}

void PanasonicACBase::set_preset_status_text_sensor(text_sensor::TextSensor *preset_status) {
  this->preset_status_text_sensor_ = preset_status;
}

// Debugging utilities
void PanasonicACBase::log_packet(std::vector<uint8_t> data, bool outgoing) {
  if (outgoing) {
    ESP_LOGV(TAG, "TX: %s", format_hex_pretty(data).c_str());
  } else {
    ESP_LOGV(TAG, "RX: %s", format_hex_pretty(data).c_str());
  }
}

}  // namespace panasonic_ac
}  // namespace esphome
