# Solarwatt BatteryFlex Integration for ESPHome & Home Assistant

A custom ESPHome component to integrate **Solarwatt BatteryFlex** into Home Assistant.  
This project provides transparent access to key battery and power metrics such as **SoC, SoH, Remaining Capacity, Grid Import/Export, Battery Charge/Discharge, and Operating Hours** – without relying on opaque vendor dashboards.

---

## ✨ Features

- **State of Charge (SoC)** – current battery fill level (%)
- **State of Health (SoH)** – battery condition (%)
- **Remaining Capacity** – available energy in Wh/kWh and Ah
- **Grid Import (Netz Bezug)** – power drawn from the grid (W)
- **Grid Export (Netz Einspeisung)** – power fed into the grid (W)
- **Battery Discharge (Batterie Bezug)** – power delivered from the battery (W)
- **Battery Charge (Batterie Einspeisung)** – power stored into the battery (W)
- **Operating Hours** – total runtime, derived from `PK/OP` counter (seconds → hours)

---

## 📐 How It Works

- Raw values are parsed from the BatteryFlex JSON API.  
- Calculations:
  - Remaining Capacity = `Nominal Capacity × SoH`
  - Total Capacity = `6 × 2.4 kWh × SoH` (for 6 packs in series)
  - Operating Hours = `PK/OP ÷ 3600`
- All values are published as ESPHome sensors and appear natively in Home Assistant.

---

## 🛠 Example ESPHome Snippet

```yaml
sensor:
  - platform: template
    name: "Grid Import"
    icon: mdi:transmission-tower-import
    unit_of_measurement: "W"
    lambda: "return id(grid_import_raw);"

  - platform: template
    name: "Grid Export"
    icon: mdi:transmission-tower-export
    unit_of_measurement: "W"
    lambda: "return id(grid_export_raw);"

  - platform: template
    name: "Battery Discharge"
    icon: mdi:flash
    unit_of_measurement: "W"
    lambda: "return id(battery_discharge_raw);"

  - platform: template
    name: "Battery Charge"
    icon: mdi:battery-charging
    unit_of_measurement: "W"
    lambda: "return id(battery_charge_raw);"

---

## 🤝 Contributing

Feel free to fork, improve, or adapt this project to your own setup. Pull requests are welcome!

---

## 📜 License

This project is licensed under the MIT License. See `LICENSE` for details.

---

## 💬 Contact

Created by **Albrecht Schroth**  
For questions or feedback, feel free to open an issue or reach out via G