#include "ISerState.hpp"
#include "Exception.hpp"

namespace huse {

ISerState::~ISerState() = default;

void ISerState::throwException(std::string msg) const {
    throw SerException(msg, {});
}

} // namespace huse
