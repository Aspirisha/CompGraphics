#pragma once

#include <stdio.h>

namespace D3D11Framework 
{

  class Log 
  {
  public:
    static Log* Get() { return m_instance; }
    static void Create();
    static void Destroy();

    void Print(const char *message, ...);
    void Debug(const char *message, ...);
    void Err(const char *message, ...);
  private:
    Log();
    ~Log();

    void m_init();
    void m_close();
    void m_print(const char *levtext, const char *text);

    FILE *m_file;
    static Log *m_instance;
  };
}