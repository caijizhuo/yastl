#ifndef _INCLUDE_EXCEPTDEF_H_
#define _INCLUDE_EXCEPTDEF_H_

#include <stdexcept>

#include <cassert>

namespace yastl {

#define yastl_DEBUG(expr) \
  assert(expr)  

#define THROW_LENGTH_ERROR_IF(expr, what) \
  if ((expr)) throw std::length_error(what)

#define THROW_OUT_OF_RANGE_IF(expr, what) \
  if ((expr)) throw std::out_of_range(what)

#define THROW_RUNTIME_ERROR_IF(expr, what) \
  if ((expr)) throw std::runtime_error(what)

} // namepsace yastl
#endif // _INCLUDE_EXCEPTDEF_H_

