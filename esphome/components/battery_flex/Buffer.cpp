#include <algorithm>
#include <cstring>

#include "Buffer.hpp"


namespace esphome
{
  namespace battery_flex
  {
	Buffer::Buffer
	 () :
	 
	 m_http_header_terminator(HTTP_HEADER_TERMINATOR),
	 
	 m_lengthBuffer(0)
	 
	{
	  // ensure, that buffer is cleared
	  memset(m_buffer, 0, sizeof(m_buffer));
	}	 


	size_t
	Buffer::size
	 () const
	 
	{
	  return m_lengthBuffer;
	}
	

	void
	Buffer::clear
	 ()
	 
	{
	  m_lengthBuffer	= 0;
	  
	  m_buffer[0]		= '\0';
	}


	const char*
	Buffer::getData
	 () const
	 
	{
	  return (m_buffer);
	}
	
	
	Buffer::Status
	Buffer::append
	 (void*		p_data,
	  size_t	p_length)
	
	{
	  Status	result				= Status::SUCCESS;
	  
	  size_t	actLength			= std::min(sizeof(m_buffer) - 1 - m_lengthBuffer, p_length);
	  
	  // check if there's still space available
	  if (0 < actLength)
	  {
		memcpy(m_buffer + m_lengthBuffer, p_data, actLength);
		
		m_lengthBuffer				+= actLength;
		
		// ensure, that buffer is terminated with '\0'
		m_buffer[m_lengthBuffer]	= '\0';
	  }
	  
	  // check for buffer overflow
	  if (p_length > actLength)
	  {
	    result						= Status::OVERFLOW;
	  }
	  
	  return result;
	}
	
	
	const char*
	Buffer::getPayload
	 () const
	 
	{
	  // setup default result
	  const char*	result	= nullptr;
	  
	  
	  // check if buffer holds any data
	  if (0 < m_lengthBuffer)
	  {
		result				= strstr(m_buffer, m_http_header_terminator.c_str());
		
		if (nullptr != result)
		{
		  // skip terminator
		  result			= &result[m_http_header_terminator.length()];
		}
	  }
	  
	  return result;
	}
  }  // namespace battery_flex
}  // namespace esphome