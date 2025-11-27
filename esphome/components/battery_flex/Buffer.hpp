#pragma once

#include <string>

namespace esphome
{
  namespace battery_flex
  {
	class Buffer
	{
	public:
	
		inline static const	std::string_view		HTTP_HEADER_TERMINATOR	= "\r\n\r\n";
	
		inline static constexpr size_t				CAPACITY_BUFFER		= 2048;
		
		enum class Status : int						{ SUCCESS = 0, OVERFLOW = -1 };

	
		Buffer();
		
		size_t		size() const;		
		
		void 		clear();
		
		const char*	getData() const;
		
		Status		append(void* p_data, size_t p_length); 
		
		const char*	getPayload() const;
	
  
	private:
	
		// disabled operations
		Buffer(const Buffer&)											= delete;
		Buffer(Buffer&&)												= delete;
		
		Buffer&		operator=(const Buffer&)							= delete;
		Buffer&		operator=(Buffer&&)									= delete;
		
		const std::string							m_http_header_terminator;
	
		size_t										m_lengthBuffer;
		
		char										m_buffer[CAPACITY_BUFFER];
	};	
  }  // namespace battery_flex
}  // namespace esphome