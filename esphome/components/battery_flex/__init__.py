# __init__.py
# BatteryFlex custom component package

from . import sensor

CONFIG_SCHEMA = sensor.CONFIG_SCHEMA
to_code = sensor.to_code