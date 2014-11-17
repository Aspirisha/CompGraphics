#include "stdafx.h"
#include "Log.h"

#define LOGNAME "log.txt"

namespace D3D11Framework 
{
  Log *Log::m_instance = nullptr;

  /*user interface*/
  void Log::Create()
  {
    if (!m_instance)
      m_instance = new Log();
    else
			m_instance->Err("Log was already created.");
  }

  void Log::Destroy() 
  {
    if (m_instance) 
      delete m_instance;
  }

  void Log::Print(const char *message, ...)
  {
    va_list args;
    va_start(args, message);
    int len = _vscprintf(message, args) + 1;
    char *buffer = new char[len];
    vsprintf_s(buffer, len, message, args);
    m_print("", buffer);
    va_end(args);
    delete[] buffer;
  }

  void Log::Debug(const char *message, ...)
  {
#ifdef _DEBUG
    va_list args;
    va_start(args, message);
    int len = _vscprintf(message, args) + 1;
    char *buffer = new char[len];
    vsprintf_s(buffer, len, message, args);
    m_print("*DEBUG: ", buffer);
    va_end(args);
    delete[] buffer;
#endif
  }

  void Log::Err(const char *message, ...)
  {
    va_list args;
    va_start(args, message);
    int len = _vscprintf(message, args) + 1;
    char *buffer = new char[len];
    vsprintf_s(buffer, len, message, args);
    m_print("*ERROR: ", buffer);
    va_end(args);
    delete[] buffer;
  }

  /*Inner implementation*/
  Log::Log() 
  {
    m_file = nullptr;
    m_init();
  }

  Log::~Log() 
  {
    m_close();
    m_instance = nullptr;
  }

  void Log::m_init()
	{	
		if (fopen_s(&m_file, LOGNAME, "w") == 0)
		{
			char timer[9];
			_strtime_s(timer,9);
			char date[9];
			_strdate_s(date,9);
			fprintf(m_file, "Log created: %s %s.\n", date, timer);
			fprintf(m_file, "---------------------------------------\n\n");
		}		
		else
		{
			printf("Error creating log file...\n");
			m_file = nullptr;
		}
	}

  void Log::m_close()
	{
		if (!m_file)
			return;

		char timer[9];
		_strtime_s(timer,9);
		char date[9];
		_strdate_s(date,9);
		fprintf(m_file, "\n---------------------------------------\n");
		fprintf(m_file, "Log finished: %s %s.", date, timer);
		fclose(m_file);
	}

  void Log::m_print(const char *levtext, const char *text)
	{
		char timer[9];
		_strtime_s(timer,9);
		clock_t cl = clock();

		printf("%s::%d: %s%s\n", timer, cl, levtext, text); 
		if(m_file)
		{
			fprintf(m_file, "%s::%d: %s%s\n", timer, cl, levtext, text);
			fflush(m_file);
		}
	}
}
