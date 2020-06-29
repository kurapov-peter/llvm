#pragma once

#include <CL/sycl/queue.hpp>
#include <detail/queue_impl.hpp>

#include <iostream>
#include <string>

__SYCL_INLINE_NAMESPACE(cl) {
namespace sycl {
namespace detail {

class multi_queue_impl {
  //vector_class<shared_ptr_class<queue[_impl]>> qs;
  // FIXME
  vector_class<queue> qs;

  queue *choose_cpu() {return &qs[0];}
  queue *choose_gpu() {return &qs[1];}

  void init_all_platforms(const property_list &PropList = {});

  queue *choose_best_queue(const function_class<void(handler &)> &CGF) {
    // Choose logic goes here.
    std::cout << "Total devices: " << qs.size() << std::endl;
    std::string m_device;
    std::cout << "Device: ";
    std::cin >> m_device;
    queue *d = nullptr;
    // bool MHostQueue = false;
    if (m_device == "cpu") {
      d = choose_cpu();
      // MHostQueue = true;
    } else {
      d = choose_gpu();
    }
    return d;
  }

  event submit_impl(const function_class<void(handler &)> &CGF,
                    const detail::code_location &Loc) {
    auto d = choose_best_queue(CGF);
    return d->submit(CGF);
  }

public:
  multi_queue_impl(const async_handler &AsyncHandler, const property_list &PropList);

  event submit(const function_class<void(handler &)> &CGF,
               shared_ptr_class<multi_queue_impl> Self /*remove?*/,
               const detail::code_location &Loc) {
    return submit_impl(CGF, Loc);
  }
};

} // namespace detail
} // namespace sycl
} // __SYCL_INLINE_NAMESPACE(cl)
