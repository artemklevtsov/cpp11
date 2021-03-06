#pragma once

#include "cpp11/integers.hpp"
#include "cpp11/vector.hpp"

namespace cpp11 {
template <typename V, typename T>
class matrix {
 private:
  V vector_;
  int nrow_;

  class row {
   private:
    matrix& parent_;
    int row_;

   public:
    row(matrix& parent, R_xlen_t row) : parent_(parent), row_(row) {}
    T operator[](const int pos) { return parent_.vector_[row_ + (pos * parent_.nrow_)]; }
  };
  friend row;

 public:
  matrix(SEXP data) : vector_(data), nrow_(INTEGER_ELT(vector_.attr("dim"), 0)) {}

  template <typename V2, typename T2>
  matrix(const cpp11::matrix<V2, T2>& rhs) : vector_(rhs), nrow_(rhs.nrow()) {}

  matrix(int nrow, int ncol) : vector_(R_xlen_t(nrow * ncol)), nrow_(nrow) {
    vector_.attr("dim") = {nrow, ncol};
  }

  int nrow() const { return nrow_; }

  int ncol() const { return size() / nrow_; }

  SEXP data() const { return vector_.data(); }

  R_xlen_t size() const { return vector_.size(); }

  operator SEXP() const { return SEXP(vector_); }

  // operator sexp() { return sexp(vector_); }

  sexp attr(const char* name) const { return SEXP(vector_.attr(name)); }

  sexp attr(const std::string& name) const { return SEXP(vector_.attr(name)); }

  sexp attr(SEXP name) const { return SEXP(vector_.attr(name)); }

  vector<string> names() const { return SEXP(vector_.names()); }

  row operator[](const int pos) { return {*this, pos}; }

  T operator()(int row, int col) { return vector_[row + (col * nrow_)]; }

  // operator cpp11::matrix<V, T>() { return SEXP(); }
};

using doubles_matrix = matrix<vector<double>, double>;
using integers_matrix = matrix<vector<int>, int>;
using logicals_matrix = matrix<vector<Rboolean>, Rboolean>;
using strings_matrix = matrix<vector<string>, string>;

namespace writable {
using doubles_matrix = matrix<vector<double>, vector<double>::proxy>;
using integers_matrix = matrix<vector<int>, vector<int>::proxy>;
using logicals_matrix = matrix<vector<Rboolean>, vector<Rboolean>::proxy>;
using strings_matrix = matrix<vector<string>, vector<string>::proxy>;
}  // namespace writable

// TODO: Add tests for Matrix class

};  // namespace cpp11
