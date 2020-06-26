#include <CL/sycl/multi_queue.hpp>
#include <CL/sycl/queue.hpp>
#include <CL/sycl/platform.hpp>

#include <string>
#include <iostream>
#include <algorithm>

__SYCL_INLINE_NAMESPACE(cl) {
namespace sycl {

// FIXME
queue *multi_queue::choose_cpu() {
  std::cerr << "Running with cpu:" << std::endl;
  return &qs[0];
}

// FIXME
queue *multi_queue::choose_gpu() {
  std::cerr << "Running with gpu:" << std::endl;
  return &qs[1];
}

void multi_queue::init_all_platforms(const property_list& PropList) {
  for (const auto &_platform : platform::get_platforms()) {
    for (const auto &_device : _platform.get_devices()) {
      auto _queue = queue(_device);
      qs.push_back(_queue);
    }
  }
}

} // namespace sycl
} // __SYCL_INLINE_NAMESPACE(cl)
