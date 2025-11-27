#pragma once

#include <array>
#include <string>

#include <AsyncTCP.h>

#include "esphome/components/sensor/sensor.h"
#include "esphome/core/component.h"

#include "Buffer.hpp"


namespace esphome
{
  namespace battery_flex
  {
	class BatteryFlexSensor : public PollingComponent //, public sensor::Sensor
	{
	public:

		inline static const	std::string_view		IP_BATTERY_FLEX			= "solar-batterie.fritz.box";
		
		inline static const	std::string_view		HTTP_COMMAND_GET		= "GET";
		inline static const std::string_view	    HTTP_STATUS_TARGET		= "/stat";
		inline static const std::string_view	    HTTP_PACK_TARGET		= "/pack?p=";
		inline static const std::string_view	    HTTP_PROTOCOL			= "HTTP/1.1";
		inline static const std::string_view	    HTTP_EOL				= "\r\n";
		inline static const	std::string_view		HTTP_HEADER_TERMINATOR	= Buffer::HTTP_HEADER_TERMINATOR;
		inline static const	std::string_view		HTTP_HOST				= "Host:";
		inline static const	std::string_view		HTTP_CONNECTION			= "Connection: close";
		inline static const	std::string_view		HTTP_ACCEPT_ANY			= "Accept: */*";
		
		inline static constexpr int 				HTTP_PORT				= 80;
		inline static constexpr int					HTTP_OK       			= 200;
		
		inline static constexpr int					PERIOD_MS				= 1000;   	// polling interval
		inline static constexpr int					PERIOD_PACK				= 5;
		
		inline static constexpr	size_t				NUM_PACKS				= 6;
		inline static constexpr float				PACK_CAPACITY			= 2400;		// capacity per pack in Wh
		
		inline static constexpr float				HOURS_IN_SEC			= 3600;

	
		BatteryFlexSensor();

		void setup() override;
		void update() override;
		
		void set_grid_power_raw(sensor::Sensor* p_sensor);
		void set_battery_power_raw(sensor::Sensor* p_sensor);
		void set_battery_soc(sensor::Sensor* p_sensor);
		void set_battery_soh(sensor::Sensor* p_sensor);
		void set_battery_capacity(sensor::Sensor* p_sensor);
		void set_battery_operation(sensor::Sensor* p_sensor);
	
  
	private:
	
		struct PackData
		{
		  std::string								m_serialNumber;
		  float										m_soh;
		};
	
		void setupClientStatus();
		void setupClientBatteryPacks();
		
		void scheduleClientStatus();
		void scheduleClientBatteryPacks();
	
	    // members to deal with status information
		std::string									m_httpCommandStatus;
		
	    Buffer										m_bufferStatus;
	
	    AsyncClient									m_clientStatus;
		
		// members to deal with detailed information
		int											m_timerPacks;
		size_t										m_indexPack;
		std::array<std::string, NUM_PACKS>			m_httpCommandPacks;
		std::array<PackData, NUM_PACKS>				m_dataPacks;

		Buffer										m_bufferPack;
			
	    AsyncClient									m_clientPacks;
		
		// pointers to YAML-sensors
		sensor::Sensor*								m_grid_power_raw_sensor;
		sensor::Sensor* 							m_battery_power_raw_sensor;
		sensor::Sensor*								m_battery_soc_sensor;
		sensor::Sensor*								m_battery_soh_sensor;
		sensor::Sensor*								m_battery_capacity_sensor;
		sensor::Sensor*								m_battery_operation_sensor;
		
		// calculated values
		float										m_soh;
	};	
  }  // namespace battery_flex
}  // namespace esphome