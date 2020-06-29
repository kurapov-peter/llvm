#include <detail/multi_queue_impl.hpp>

__SYCL_INLINE_NAMESPACE(cl) {
namespace sycl {
namespace detail {

multi_queue_impl::multi_queue_impl(const async_handler &AsyncHandler,
                                   const property_list &PropList) {
  init_all_platforms(PropList);
}

void multi_queue_impl::init_all_platforms(const property_list &PropList) {
  for (const auto &_platform : platform::get_platforms()) {
    for (const auto &_device : _platform.get_devices()) {
      auto _queue = queue(_device);
      qs.push_back(_queue);
    }
  }
}

} // namespace detail
} // namespace sycl
} // __SYCL_INLINE_NAMESPACE(cl)
