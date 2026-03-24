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
	 (const BatteryFlexSensor::JsonDocument&	p_jsonDoc,
	  const char*								p_name,
	  sensor::Sensor*							p_sensor)
	
	{
	  TypeValue actValue	= p_jsonDoc[p_name].as<TypeValue>();
	  
	  publishSensor<TypeValue>(actValue, p_name, p_sensor);
	}
	
	
	BatteryFlexSensor::BatteryFlexSensor
	 () :
	 
	 PollingComponent(PERIOD_MS),
	 
	 m_indexPack(0),
	 
	 m_disconnectTimeout(false),
	 
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
	  // check if there's any connection to Battery Flex pending
	  if ((false == m_clientStatus.connected()) &&
		  (false == m_clientPacks.connected()))
	  {	
	  	const std::string	actAddress(IP_BATTERY_FLEX);
		
		// ensure, that buffer holding Json document is cleared
		m_jsonDocument.clear();
		
		// mark flag for regular disconnect
		m_disconnectTimeout	= false;
		
		// check if ready to access battery packs
		if (0 == m_timerPacks)
		{
          // open connection to get information about batterie packs
	      m_clientPacks.connect(actAddress.c_str(), HTTP_PORT);
		}
		else
		{
		  // open connection to get status of Client
		  m_clientStatus.connect(actAddress.c_str(), HTTP_PORT);
		}

		// increase timer to access battery pack
	    m_timerPacks++;
	  
	    m_timerPacks		%= PERIOD_PACK;
	  }
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
							
	  // setup timeout
	  m_clientStatus.setRxTimeout(GET_TIMEOUT_MS);
							
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
			  ESP_LOGE("battery_flex", "m_clientStatus.onData: unable to close Client");
			}
		  }
		},
		nullptr);
				
	  m_clientStatus.onTimeout
	   ([this](void *p_arg, AsyncClient *p_client, uint32_t p_time)
		 
		{
		  ESP_LOGW("battery_flex", "m_clientStatus.onTimeout: Timeout");
		  
		  if (nullptr != p_client)
		  {
			// mark as disconnect due to timeout
			m_disconnectTimeout	= true;
			
			p_client -> close();
		  }
		  else
		  {
			ESP_LOGE("battery_flex", "m_clientStatus.onTimeout: invalid client");
		  }
		}
	   );

	  m_clientStatus.onDisconnect
	   ([this]
	     (void* p_arg, AsyncClient* p_client)
		 
		{
		  // skip processing in case of timeout
		  if (false == m_disconnectTimeout)
		  {
		    // get access to payload
		    const char*		actPayload					= m_bufferStatus.getPayload();
		  
		    // check if payload is available
		    if (nullptr != actPayload)
		    {
		      // read values from JSON Object
			  DeserializationError actError				= deserializeJson(m_jsonDocument, actPayload);
		
			  if (!actError)
			  {
			    publishSensor<int>(m_jsonDocument, "SoC", m_battery_soc_sensor);
			    publishSensor<float>(m_jsonDocument, "PGrid", m_grid_power_raw_sensor);
			    publishSensor<float>(m_jsonDocument, "PBat", m_battery_power_raw_sensor);
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
		  }
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
							
	  // setup timeout
	  m_clientPacks.setRxTimeout(GET_TIMEOUT_MS);
	  	  							
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
			  ESP_LOGE("battery_flex", "m_clientPacks.onData: unable to close Client");
			}
		  }
		},
		nullptr);
				
	  m_clientPacks.onTimeout
	   ([this](void *p_arg, AsyncClient *p_client, uint32_t p_time)
		 
		{
		  ESP_LOGW("battery_flex", "m_clientPacks.onTimeout: Timeout");
		  
		  if (nullptr != p_client)
		  {
			// mark as disconnect due to timeout
			m_disconnectTimeout	= true;
			
			p_client -> close();
		  }
		  else
		  {
			ESP_LOGE("battery_flex", "m_clientPacks.onTimeout: invalid client");
		  }
		} 
	   );

	  m_clientPacks.onDisconnect
	   ([this]
	     (void* p_arg, AsyncClient* p_client)
		 
		{
		  // skip processing in case of timeout
		  if (false == m_disconnectTimeout)
		  {
		    // get access to payload
		    const char*		actPayload					= m_bufferPack.getPayload();
		  
		    // check if payload is available
		    if (nullptr != actPayload)
		    {
		      // read values from JSON Object
			  DeserializationError actError				= deserializeJson(m_jsonDocument, actPayload);
		
			  if (!actError)
			  {
			    m_dataPacks[m_indexPack].m_serialNumber	= m_jsonDocument["PK"]["SN"].as<std::string>();
			    m_dataPacks[m_indexPack].m_soh			= m_jsonDocument["PK"]["SOH"].as<float>();
			  
			    // calculate overall SoH of battery
			    m_soh									= std::min(m_soh, m_dataPacks[m_indexPack].m_soh);
			  
			    // publish SoH
			    publishSensor<float>(m_soh, "SoH", m_battery_soh_sensor);
			  
			  
			    // calculate remaining capacity of battery
			    float	actCapacity						= NUM_PACKS * PACK_CAPACITY * m_soh / 100;
			  
			    // publish remaining capacity
			    publishSensor<float>(actCapacity,"Capacity", m_battery_capacity_sensor);
			  
			  
			    // calculate opererating hours of battery
			    float actOperatingHours					= m_jsonDocument["PK"]["OP"].as<float>() / HOURS_IN_SEC;
			  
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
	      }
		},
		nullptr);	
	}
  }  // namespace battery_flex
}  // namespace esphome