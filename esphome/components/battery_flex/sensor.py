import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_NAME
from esphome.components import sensor

battery_flex_ns = cg.esphome_ns.namespace('battery_flex')

BatteryFlexSensor = battery_flex_ns.class_('BatteryFlexSensor', cg.Component)

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(BatteryFlexSensor),
    cv.Required("grid_power_raw"): cv.use_id(sensor.Sensor),
    cv.Required("battery_power_raw"): cv.use_id(sensor.Sensor),
    cv.Required("battery_soc"): cv.use_id(sensor.Sensor),
    cv.Required("battery_soh"): cv.use_id(sensor.Sensor),
    cv.Required("battery_capacity"): cv.use_id(sensor.Sensor),
    cv.Required("battery_operation"): cv.use_id(sensor.Sensor),
})

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    
    grid = await cg.get_variable(config["grid_power_raw"])
    cg.add(var.set_grid_power_raw(grid))

    bat = await cg.get_variable(config["battery_power_raw"])
    cg.add(var.set_battery_power_raw(bat))

    soc = await cg.get_variable(config["battery_soc"])
    cg.add(var.set_battery_soc(soc))
    
    soh = await cg.get_variable(config["battery_soh"])
    cg.add(var.set_battery_soh(soh))
        
    cap = await cg.get_variable(config["battery_capacity"])
    cg.add(var.set_battery_capacity(cap))
            
    operation = await cg.get_variable(config["battery_operation"])
    cg.add(var.set_battery_operation(operation))