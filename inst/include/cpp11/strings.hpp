#pragma once

#include <initializer_list>     // for initializer_list
#include <stdexcept>            // for out_of_range
#include <string>               // for basic_string
#include "cpp11/R.hpp"          // for SET_STRING_ELT, SEXP, SEXPREC, R_Pr...
#include "cpp11/as.hpp"         // for as_sexp
#include "cpp11/named_arg.hpp"  // for named_arg
#include "cpp11/protect.hpp"    // for unwind_protect, protect, protect::f...
#include "cpp11/string.hpp"     // for string
#include "cpp11/vector.hpp"     // for vector, vector<>::proxy, vector<>::...

// Specializations for strings

namespace cpp11 {

template <>
inline SEXP vector<string>::valid_type(SEXP data) {
  if (TYPEOF(data) != STRSXP) {
    throw type_error(STRSXP, TYPEOF(data));
  }
  return data;
}

template <>
inline string vector<string>::operator[](const R_xlen_t pos) const {
  // NOPROTECT: likely too costly to unwind protect every elt
  return STRING_ELT(data_, pos);
}

template <>
inline string vector<string>::at(const R_xlen_t pos) const {
  if (pos < 0 || pos >= length_) {
    throw std::out_of_range("strings");
  }
  // NOPROTECT: likely too costly to unwind protect every elt
  return STRING_ELT(data_, pos);
}

template <>
inline string* vector<string>::get_p(bool, SEXP) {
  return nullptr;
}

template <>
inline void vector<string>::const_iterator::fill_buf(R_xlen_t) {
  return;
}

template <>
inline string vector<string>::const_iterator::operator*() {
  return STRING_ELT(data_->data(), pos_);
}

typedef vector<string> strings;

namespace writable {

template <>
inline typename vector<string>::proxy& vector<string>::proxy::operator=(string rhs) {
  unwind_protect([&] { SET_STRING_ELT(data_, index_, rhs); });
  return *this;
}

template <>
inline vector<string>::proxy::operator string() const {
  // NOPROTECT: likely too costly to unwind protect every elt
  return STRING_ELT(data_, index_);
}

inline bool operator==(const vector<string>::proxy& lhs, string rhs) {
  return static_cast<string>(lhs).operator==(static_cast<std::string>(rhs).c_str());
}

inline SEXP alloc_or_copy(const SEXP& data) {
  switch (TYPEOF(data)) {
    case CHARSXP:
      return cpp11::vector<string>(safe[Rf_allocVector](STRSXP, 1));
    case STRSXP:
      return safe[Rf_shallow_duplicate](data);
    default:
      throw type_error(STRSXP, TYPEOF(data));
  }
}

inline SEXP alloc_if_charsxp(const SEXP& data) {
  switch (TYPEOF(data)) {
    case CHARSXP:
      return cpp11::vector<string>(safe[Rf_allocVector](STRSXP, 1));
    case STRSXP:
      return data;
    default:
      throw type_error(STRSXP, TYPEOF(data));
  }
}

template <>
inline vector<string>::vector(const SEXP& data)
    : cpp11::vector<string>(alloc_or_copy(data)),
      protect_(protect_sexp(data_)),
      capacity_(length_) {
  if (TYPEOF(data) == CHARSXP) {
    SET_STRING_ELT(data_, 0, data);
  }
}

template <>
inline vector<string>::vector(SEXP&& data)
    : cpp11::vector<string>(alloc_if_charsxp(data)),
      protect_(protect_sexp(data_)),
      capacity_(length_) {
  if (TYPEOF(data) == CHARSXP) {
    SET_STRING_ELT(data_, 0, data);
  }
}

template <>
inline vector<string>::vector(std::initializer_list<string> il)
    : cpp11::vector<string>(as_sexp(il)), capacity_(il.size()) {}

template <>
inline vector<string>::vector(std::initializer_list<const char*> il)
    : cpp11::vector<string>(as_sexp(il)), capacity_(il.size()) {}

template <>
inline vector<string>::vector(std::initializer_list<named_arg> il)
    : cpp11::vector<string>(safe[Rf_allocVector](STRSXP, il.size())),
      capacity_(il.size()) {
  try {
    unwind_protect([&] {
      protect_ = protect_sexp(data_);
      attr("names") = Rf_allocVector(STRSXP, capacity_);
      SEXP names = attr("names");
      auto it = il.begin();
      for (R_xlen_t i = 0; i < capacity_; ++i, ++it) {
        SET_STRING_ELT(data_, i, strings(it->value())[0]);
        SET_STRING_ELT(names, i, Rf_mkCharCE(it->name(), CE_UTF8));
      }
    });
  } catch (const unwind_exception& e) {
    release_protect(protect_);
    throw e;
  }
}

template <>
inline void vector<string>::reserve(R_xlen_t new_capacity) {
  data_ = data_ == R_NilValue ? safe[Rf_allocVector](STRSXP, new_capacity)
                              : safe[Rf_xlengthgets](data_, new_capacity);

  SEXP old_protect = protect_;
  protect_ = protect_sexp(data_);
  release_protect(old_protect);

  capacity_ = new_capacity;
}

template <>
inline void vector<string>::push_back(string value) {
  while (length_ >= capacity_) {
    reserve(capacity_ == 0 ? 1 : capacity_ *= 2);
  }
  unwind_protect([&] { SET_STRING_ELT(data_, length_, value); });
  ++length_;
}

typedef vector<string> strings;

template <typename T>
inline void vector<T>::push_back(const named_arg& value) {
  push_back(value.value());
  cpp11::writable::strings nms(names());
  nms.push_back(value.name());
}

}  // namespace writable

}  // namespace cpp11
