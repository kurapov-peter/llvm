#include <CL/sycl/multi_queue.hpp>
#include <detail/multi_queue_impl.hpp>


__SYCL_INLINE_NAMESPACE(cl) {
namespace sycl {

multi_queue::multi_queue(const async_handler &AsyncHandler, const property_list &PropList) {
  impl = std::make_shared<detail::multi_queue_impl>(AsyncHandler, PropList);
}

event multi_queue::submit_impl(function_class<void(handler &)> CGH,
                               const detail::code_location &CodeLoc) {
  return impl->submit(CGH, impl, CodeLoc);
}


} // namespace sycl
} // __SYCL_INLINE_NAMESPACE(cl)
