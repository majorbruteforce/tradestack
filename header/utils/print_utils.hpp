#pragma once
#include <iomanip>
#include <iostream>
#include <string>

namespace utils
{

  template <typename T>
  void printField(const std::string &label, const T &value, std::size_t labelWidth = 15, std::ostream &os = std::cout)
  {
    os << std::left << std::setw(labelWidth) << label << ": " << value << '\n';
  }
}
