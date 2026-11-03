#include "MazeCode.hpp"

#include <utility>

MazeCode::MazeCode(WordProvider provider) : provider_(std::move(provider)) {}

void MazeCode::reset() { code_ = provider_(); }

bool MazeCode::verify(const std::string& code) const { return !code_.empty() && code == code_; }

std::string MazeCode::value() const { return code_; }
