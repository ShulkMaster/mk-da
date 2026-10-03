#include <new>

namespace std {

// Linker-stripped in retail; its throw alone emits the retained weak
// std::exception destructor, what(), vtable, RTTI and type-name strings.
static void default_new_handler() throw(bad_alloc)
{
  throw(bad_alloc());
}

} // namespace std
