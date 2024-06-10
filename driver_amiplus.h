/*
  Based on: https://github.com/wmbusmeters/wmbusmeters/blob/master/src/driver_amiplus.cc
  Copyright (C) 2019-2022 Fredrik Öhrström (gpl-3.0-or-later)
*/

#pragma once

#include "driver.h"

#include <vector>
#include <string>

struct Amiplus: Driver
{
  Amiplus(std::string key = "") : Driver(std::string("amiplus"), key) {};
  virtual esphome::optional<std::map<std::string, double>> get_values(std::vector<unsigned char> &telegram) override {
    std::map<std::string, double> ret_val{};

    add_to_map(ret_val, "total_energy_consumption_kwh", this->get_total_energy_consumption_kwh(telegram));
    add_to_map(ret_val, "total_energy_consumption_tarrif_1_kwh", this->get_total_energy_consumption_by_tarrif_kwh(1, telegram));
    add_to_map(ret_val, "total_energy_consumption_tarrif_2_kwh", this->get_total_energy_consumption_by_tarrif_kwh(2, telegram));
    add_to_map(ret_val, "total_energy_consumption_tarrif_3_kwh", this->get_total_energy_consumption_by_tarrif_kwh(3, telegram));
    add_to_map(ret_val, "current_power_consumption_kw", this->get_current_power_consumption_kw(telegram));
    add_to_map(ret_val, "total_energy_production_kwh", this->get_total_energy_production_kwh(telegram));
    add_to_map(ret_val, "current_power_production_kw", this->get_current_power_production_kw(telegram));
    add_to_map(ret_val, "voltage_at_phase_1_v", this->get_voltage_at_phase_v(1, telegram));
    add_to_map(ret_val, "voltage_at_phase_2_v", this->get_voltage_at_phase_v(2, telegram));
    add_to_map(ret_val, "voltage_at_phase_3_v", this->get_voltage_at_phase_v(3, telegram));

    if (ret_val.size() > 0) {
      return ret_val;
    }
    else {
      return {};
    }
  };

private:
  esphome::optional<double> get_voltage_at_phase_v(uint8_t phase, std::vector<unsigned char> &telegram) {
    esphome::optional<double> ret_val{};
    uint32_t usage = 0;
    size_t i = 11;
    uint32_t total_register = 0x0AFDC9FC;
    while (i < telegram.size()) {
      uint32_t c = (((uint32_t)telegram[i+0] << 24) | ((uint32_t)telegram[i+1] << 16) |
                    ((uint32_t)telegram[i+2] << 8) | ((uint32_t)telegram[i+3]));
      if (c == total_register) {
        i += 5;
        if ((uint8_t)telegram[i-1] == phase) {
          usage = bcd_2_int(telegram, i, 2);
          ret_val = usage / 1.0;
          ESP_LOGVV(TAG, "Found register '0AFDC9FC%d' with '%d'->'%f'", phase, usage, ret_val.value());
          break;
        }
      }
      i++;
    }
    return ret_val;
  };

  esphome::optional<double> get_total_energy_consumption_kwh(std::vector<unsigned char> &telegram) {
    esphome::optional<double> ret_val{};
    uint32_t usage = 0;
    size_t i = 11;
    uint32_t total_register = 0x0E03;
    while (i < telegram.size()) {
      uint32_t c = (((uint32_t)telegram[i+0] << 8) | ((uint32_t)telegram[i+1]));
      if (c == total_register) {
        i += 2;
        usage = bcd_2_int(telegram, i, 6);
        ret_val = usage / 1000.0;
        ESP_LOGVV(TAG, "Found register '0E03' with '%d'->'%f'", usage, ret_val.value());
        break;
      }
      i++;
    }
    return ret_val;
  };

  esphome::optional<double> get_total_energy_consumption_by_tarrif_kwh(uint32_t tarrif, std::vector<unsigned char> &telegram) {
    esphome::optional<double> ret_val{};
    uint32_t usage = 0;
    size_t i = 11;
    uint32_t total_tarrif_register = 0x8E0000 | (tarrif << 12) | 0x000003;
    while (i < telegram.size()) {
      uint32_t c = (((uint32_t)telegram[i+0] << 16) | ((uint32_t)telegram[i+1] << 8) | ((uint32_t)telegram[i+2]));
      if (c == total_tarrif_register) {
        i += 2;
        usage = bcd_2_int(telegram, i, 6);
        ret_val = usage / 100000.0;
        ESP_LOGVV(TAG, "Found register '%06x' with '%d'->'%f'", total_tarrif_register, usage, ret_val.value());
        break;
      }
      i++;
    }
    return ret_val;
  };

  esphome::optional<double> get_current_power_consumption_kw(std::vector<unsigned char> &telegram) {
    esphome::optional<double> ret_val{};
    uint32_t usage = 0;
    size_t i = 11;
    uint32_t total_register = 0x0B2B;
    while (i < telegram.size()) {
      uint32_t c = (((uint32_t)telegram[i+0] << 8) | ((uint32_t)telegram[i+1]));
      if (c == total_register) {
        i += 2;
        usage = bcd_2_int(telegram, i, 3);
        ret_val = usage / 1000.0;
        ESP_LOGVV(TAG, "Found register '0B2B' with '%d'->'%f'", usage, ret_val.value());
        break;
      }
      i++;
    }
    return ret_val;
  };

  esphome::optional<double> get_total_energy_production_kwh(std::vector<unsigned char> &telegram) {
    esphome::optional<double> ret_val{};
    uint32_t usage = 0;
    size_t i = 11;
    uint32_t total_register = 0x0E833C;
    while (i < telegram.size()) {
      uint32_t c = (((uint32_t)telegram[i+0] << 16) | ((uint32_t)telegram[i+1] << 8) | ((uint32_t)telegram[i+2]));
      if (c == total_register) {
        i += 3;
        usage = bcd_2_int(telegram, i, 6);
        ret_val = usage / 1000.0;
        ESP_LOGVV(TAG, "Found register '0E833C' with '%d'->'%f'", usage, ret_val.value());
        break;
      }
      i++;
    }
    return ret_val;
  };

  esphome::optional<double> get_current_power_production_kw(std::vector<unsigned char> &telegram) {
    esphome::optional<double> ret_val{};
    uint32_t usage = 0;
    size_t i = 11;
    uint32_t total_register = 0x0BAB3C;
    while (i < telegram.size()) {
      uint32_t c = (((uint32_t)telegram[i+0] << 16) | ((uint32_t)telegram[i+1] << 8) | ((uint32_t)telegram[i+2]));
      if (c == total_register) {
        i += 3;
        usage = bcd_2_int(telegram, i, 3);
        ret_val = usage / 1000.0;
        ESP_LOGVV(TAG, "Found register '0BAB3C' with '%d'->'%f'", usage, ret_val.value());
        break;
      }
      i++;
    }
    return ret_val;
  };
};