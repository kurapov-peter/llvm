#pragma once
#include <CL/sycl/queue.hpp>

__SYCL_INLINE_NAMESPACE(cl) {
namespace sycl {

namespace detail {
  class multi_queue_impl;
}

class __SYCL_EXPORT multi_queue {

public:
  explicit multi_queue(const property_list &PropList = {})
      : multi_queue(async_handler{}, PropList) {}

  multi_queue(const async_handler &AsyncHandler, const property_list &PropList = {});

  // just a proxy call to appropriate queue->submit
  template <typename T> event submit(T CGF) {
    const detail::code_location &CodeLoc = {};
    return submit_impl(CGF, CodeLoc);
  }

private:
  shared_ptr_class<detail::multi_queue_impl> impl;
  multi_queue(shared_ptr_class<detail::multi_queue_impl> impl) : impl(impl) {}

  event submit_impl(function_class<void(handler &)> CGH,
                    const detail::code_location &CodeLoc);
};

} // namespace sycl
} // __SYCL_INLINE_NAMESPACE(cl)

