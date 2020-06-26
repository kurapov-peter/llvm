#pragma once
#include <CL/sycl/queue.hpp>

#include <iostream>
#include <string>
#include <vector>

__SYCL_INLINE_NAMESPACE(cl) {
namespace sycl {

class __SYCL_EXPORT multi_queue : public queue {
  // wrap with a mutli_queue_impl ?
  std::vector<queue> qs;

  void init_all_platforms(const property_list &PropList = {});

  queue *choose_cpu();
  queue *choose_gpu();

  template <typename T> queue *choose_best_queue(const T &CGF) {
    // Choose logic goes here.
    std::cout << "Total devices: " << qs.size() << std::endl;
    std::string m_device;
    std::cout << "Device: ";
    std::cin >> m_device;
    queue *d = nullptr;
    bool MHostQueue = false;
    if (m_device == "cpu") {
      d = choose_cpu();
      MHostQueue = true;
    } else {
      d = choose_gpu();
    }
    return d;
  }

public:
  explicit multi_queue(const property_list &PropList = {}) {
    init_all_platforms(PropList);
  }

  // just a proxy call to appropriate queue->submit
  template <typename T> event submit(T CGF) {
    auto d = choose_best_queue(CGF);
    return d->submit(CGF);
  }
};

} // namespace sycl
} // __SYCL_INLINE_NAMESPACE(cl)

