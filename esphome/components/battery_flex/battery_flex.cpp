#include <string>

#include "esphome/core/hal.h"

#include "battery_flex.hpp"


namespace esphome
{
  namespace battery_flex
  {
	template<typename TypeValue>
	static void
	publishSensor
	 (const TypeValue		p_value,
	  const char*			p_name,
	  sensor::Sensor*		p_sensor)
	
	{
	  if (nullptr != p_name)
	  {
		if (nullptr != p_sensor)
		{
		  p_sensor->publish_state(p_value);
		}
		else
		{
		  ESP_LOGW("battery_flex", "readValue: sensor '%s' not bound", p_name);
		}
	  }
	  else
	  {
		ESP_LOGW("battery_flex", "readValue: got invalid name");
	  }
	}
	

	template<typename TypeValue>
	static void
	publishSensor
	 (const JsonDocument&	p_doc,
	  const char*			p_name,
	  sensor::Sensor*		p_sensor)
	
	{
	  TypeValue actValue	= p_doc[p_name].as<TypeValue>();
	  
	  publishSensor<TypeValue>(actValue, p_name, p_sensor);
	}
	
	
	BatteryFlexSensor::BatteryFlexSensor
	 () :
	 
	 PollingComponent(PERIOD_MS),
	 
	 m_indexPack(0),
	 
	 m_grid_power_raw_sensor(nullptr),
     m_battery_power_raw_sensor(nullptr),
     m_battery_soc_sensor(nullptr),
	 m_battery_soh_sensor(nullptr),
	 m_battery_capacity_sensor(nullptr),
	 m_battery_operation_sensor(nullptr),
	 
	 m_soh(100.0)
	 
	{
	  // nothing to be done here
	}	 


	void
	BatteryFlexSensor::setup
	 ()
	 
	{
	  setupClientStatus();
	  setupClientBatteryPacks();
	}


	void
	BatteryFlexSensor::update
	 ()
	
	{
	  scheduleClientStatus();
	  scheduleClientBatteryPacks(); 
    }
	
	
	void
	BatteryFlexSensor::set_grid_power_raw
	 (sensor::Sensor* p_sensor)
	 
	{
	  m_grid_power_raw_sensor	= p_sensor;
	}
	
	
	void
	BatteryFlexSensor::set_battery_power_raw
	 (sensor::Sensor* p_sensor)

    {
	  m_battery_power_raw_sensor = p_sensor;
	}
	
	
	void
	BatteryFlexSensor::set_battery_soc
	 (sensor::Sensor* p_sensor)
	 
	{
	  m_battery_soc_sensor = p_sensor;
	}
		
	
	void
	BatteryFlexSensor::set_battery_soh
	 (sensor::Sensor* p_sensor)
	 
	{
	  m_battery_soh_sensor = p_sensor;
	}
		
	void
	BatteryFlexSensor::set_battery_capacity
	 (sensor::Sensor* p_sensor)
	 
	{
	  m_battery_capacity_sensor = p_sensor;
	}
	
	
	void
	BatteryFlexSensor::set_battery_operation
	 (sensor::Sensor* p_sensor)
	 
	{
	  m_battery_operation_sensor = p_sensor;
	}
	
	
	void
	BatteryFlexSensor::setupClientStatus
	 ()
	 
	{
	  // setup command to access status of battery
	  m_httpCommandStatus	= std::string(HTTP_COMMAND_GET) + " " + std::string(HTTP_STATUS_TARGET) + " " + std::string(HTTP_PROTOCOL) + std::string(HTTP_EOL)
							+ std::string(HTTP_HOST) + " " + std::string(IP_BATTERY_FLEX) + std::string(HTTP_EOL)
 							+ std::string(HTTP_CONNECTION) + std::string(HTTP_HEADER_TERMINATOR);
							
	  // setup callback for connect
	  m_clientStatus.onConnect
	   ([this]
	     (void* p_arg, AsyncClient* p_client)
		 
		{
		  p_client->write(m_httpCommandStatus.c_str());
		},
		nullptr);

      // setup callback for receiving data
	  m_clientStatus.onData
	   ([this]
	     (void* p_arg, AsyncClient* p_client, void* p_data, size_t p_length)
		 
		{
		  // add received data to buffer
		  Buffer::Status	actStatus	= m_bufferStatus.append(p_data, p_length);
		  
		  // check for overflow
		  if (Buffer::Status::SUCCESS != actStatus)
		  {
			ESP_LOGW("battery_flex", "m_clientStatus.onData: Buffer overflow, closing Client");
			
			// ensure, that client is available
			if (nullptr != p_client)
			{
			  p_client -> close();
			}
			else
			{
			  ESP_LOGW("battery_flex", "m_clientStatus.onData: unable to close Client");
			}
		  }
		},
		nullptr);

	  m_clientStatus.onDisconnect
	   ([this]
	     (void* p_arg, AsyncClient* p_client)
		 
		{
		  // get access to payload
		  const char*		actPayload					= m_bufferStatus.getPayload();
		  
		  // check if payload is available
		  if (nullptr != actPayload)
		  {
		    // read values from JSON Object
 		    JsonDocument	actDoc;
	
			DeserializationError actError				= deserializeJson(actDoc, actPayload);
		
			if (!actError)
			{
			  publishSensor<int>(actDoc, "SoC", m_battery_soc_sensor);
			  publishSensor<float>(actDoc, "PGrid", m_grid_power_raw_sensor);
			  publishSensor<float>(actDoc, "PBat", m_battery_power_raw_sensor);
			}
			else
			{
			  ESP_LOGW("battery_flex", "m_clientStatus.onDisconnect: unable to deserialize Json: %s", actError.c_str());
			}
		  }
		  else
		  {
			ESP_LOGW("battery_flex", "m_clientStatus.onDisconnect: unable to detect payload");
		  }
		  
		  // finally clear buffer
		  m_bufferStatus.clear();
	    },
		nullptr);	
	}
	
	
	void
	BatteryFlexSensor::setupClientBatteryPacks
	 ()
	 
	{
	  // setup commands to get information of individual battery pack
	  for (int actIndex = 0; actIndex < NUM_PACKS; actIndex++)
	  {
		// setup command to GET data related to pack
		char	actBuffer[16];
		
		itoa(actIndex, actBuffer, 10);
		
		m_httpCommandPacks[actIndex]	= std::string(HTTP_COMMAND_GET) + " " + std::string(HTTP_PACK_TARGET) + actBuffer + " " + std::string(HTTP_PROTOCOL) + std::string(HTTP_EOL)
										+ std::string(HTTP_HOST) + " " + std::string(IP_BATTERY_FLEX) + std::string(HTTP_EOL)
										+ std::string(HTTP_CONNECTION) + std::string(HTTP_EOL)
										+ std::string(HTTP_ACCEPT_ANY) + std::string(HTTP_HEADER_TERMINATOR);
										
		// setup State of Health related to pack to invalid
		m_dataPacks[actIndex].m_soh		= -1.0;
	  }
	  	  							
	  // setup callback for connect
	  m_clientPacks.onConnect
	   ([this]
	     (void* p_arg, AsyncClient* p_client)
		 
		{
		  p_client->write(m_httpCommandPacks[m_indexPack].c_str());
		},
		nullptr);

      // setup callback for receiving data
	  m_clientPacks.onData
	   ([this]
	     (void* p_arg, AsyncClient* p_client, void* p_data, size_t p_length)
		 
		{
		  // add received data to buffer
		  Buffer::Status	actStatus	= m_bufferPack.append(p_data, p_length);
		  
		  // check for overflow
		  if (Buffer::Status::SUCCESS != actStatus)
		  {
			ESP_LOGW("battery_flex", "m_clientPacks.onData: Buffer overflow, closing Client");
			
			// ensure, that client is available
			if (nullptr != p_client)
			{
			  p_client -> close();
			}
			else
			{
			  ESP_LOGW("battery_flex", "m_clientPacks.onData: unable to close Client");
			}
		  }
		},
		nullptr);

	  m_clientPacks.onDisconnect
	   ([this]
	     (void* p_arg, AsyncClient* p_client)
		 
		{
		  // get access to payload
		  const char*		actPayload					= m_bufferPack.getPayload();
		  
		  // check if payload is available
		  if (nullptr != actPayload)
		  {
		    // read values from JSON Object
 		    JsonDocument	actDoc;
	
			DeserializationError actError				= deserializeJson(actDoc, actPayload);
		
			if (!actError)
			{
			  m_dataPacks[m_indexPack].m_serialNumber	= actDoc["PK"]["SN"].as<std::string>();
			  m_dataPacks[m_indexPack].m_soh			= actDoc["PK"]["SOH"].as<float>();
			  
			  // calculate overall SoH of battery
			  m_soh										= std::min(m_soh, m_dataPacks[m_indexPack].m_soh);
			  
			  // publish SoH
			  publishSensor<float>(m_soh, "SoH", m_battery_soh_sensor);
			  
			  
			  // calculate remaining capacity of battery
			  float	actCapacity							= NUM_PACKS * PACK_CAPACITY * m_soh / 100;
			  
			  // publish remaining capacity
			  publishSensor<float>(actCapacity,"Capacity", m_battery_capacity_sensor);
			  
			  
			  // calculate opererating hours of battery
			  float actOperatingHours					= actDoc["PK"]["OP"].as<float>() / HOURS_IN_SEC;
			  
			  // publish operation hours
			  publishSensor<float>(actOperatingHours, "Operation", m_battery_operation_sensor);
			}
			else
			{
			  ESP_LOGW("battery_flex", "m_clientPacks.onDisconnect: unable to deserialize Json: %s", actError.c_str());
			}
		  }
		  else
		  {
			ESP_LOGW("battery_flex", "m_clientPacks.onDisconnect: unable to detect payload");
		  }
		  
		  // finally clear buffer
		  m_bufferPack.clear();
		  
		  // move to next battery pack
		  m_indexPack++;
		  
		  m_indexPack						%= NUM_PACKS;
	    },
		nullptr);	
	}
	
	
	void
	BatteryFlexSensor::scheduleClientStatus
	 ()
	 
	{
	  // check if connection is closed
	  if (false == m_clientStatus.connected())
	  {
		const std::string	actAddress(IP_BATTERY_FLEX);
		
		// re-open connection
		m_clientStatus.connect(actAddress.c_str(), HTTP_PORT);
	  }
	  else
	  {
		ESP_LOGW("battery_flex", "Client Status still connected – skipping new request");
	  }
	}	
		
	
	void
	BatteryFlexSensor::scheduleClientBatteryPacks
	 ()
	 
	{
	  // check if ready to access battery packs
	  if (0 == m_timerPacks)
	  {
	    // check if connection is closed
		if (false == m_clientPacks.connected())
	    {
		  const std::string	actAddress(IP_BATTERY_FLEX);
		
		  // re-open connection
		  m_clientPacks.connect(actAddress.c_str(), HTTP_PORT);
	    }
	    else
	    {
		  ESP_LOGW("battery_flex", "Client Packs still connected – skipping new request");
	    }
	  }
	  
	  m_timerPacks++;
	  
	  m_timerPacks	%= PERIOD_PACK;
	}
  }  // namespace battery_flex
}  // namespace esphome