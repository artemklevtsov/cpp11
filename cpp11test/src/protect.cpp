#include <cpp11/sexp.hpp>
#include <vector>

#include "Rcpp.h"

[[cpp11::export]] void protect_one_(SEXP x, int n) {
  for (R_xlen_t i = 0; i < n; ++i) {
    PROTECT(x);
    UNPROTECT(1);
  }
}

[[cpp11::export]] void protect_one_sexp_(SEXP x, int n) {
  for (R_xlen_t i = 0; i < n; ++i) {
    cpp11::sexp y(x);
  }
}

[[cpp11::export]] void protect_one_cpp11_(SEXP x, int n) {
  for (R_xlen_t i = 0; i < n; ++i) {
    SEXP p = cpp11::protect_sexp(x);
    cpp11::release_protect(p);
  }
}

[[cpp11::export]] void protect_one_preserve_(SEXP x, int n) {
  for (R_xlen_t i = 0; i < n; ++i) {
    R_PreserveObject(x);
    R_ReleaseObject(x);
  }
}

// The internal protections here are actually uneeded, but it is a useful way to benchmark
// them

[[cpp11::export]] void protect_many_(int n) {
  std::vector<SEXP> res;
  for (R_xlen_t i = 0; i < n; ++i) {
    res.push_back(PROTECT(Rf_ScalarInteger(n)));
  }

  for (R_xlen_t i = n - 1; i >= 0; --i) {
    SEXP x = res[i];
    UNPROTECT(1);
    res.pop_back();
  }
}

[[cpp11::export]] void protect_many_cpp11_(int n) {
  std::vector<SEXP> res;
  for (R_xlen_t i = 0; i < n; ++i) {
    res.push_back(cpp11::protect_sexp(Rf_ScalarInteger(n)));
  }

  for (R_xlen_t i = n - 1; i >= 0; --i) {
    SEXP x = res[i];
    cpp11::release_protect(x);
    res.pop_back();
  }
}

[[cpp11::export]] void protect_many_sexp_(int n) {
  std::vector<cpp11::sexp> res;
  for (R_xlen_t i = 0; i < n; ++i) {
    res.push_back(Rf_ScalarInteger(n));
  }

  for (R_xlen_t i = n - 1; i >= 0; --i) {
    SEXP x = res[i];
    res.pop_back();
  }
}

[[cpp11::export]] void protect_many_preserve_(int n) {
  std::vector<cpp11::sexp> res;
  for (R_xlen_t i = 0; i < n; ++i) {
    SEXP x = Rf_ScalarInteger(n);
    R_PreserveObject(x);
    res.push_back(x);
  }

  for (R_xlen_t i = n - 1; i >= 0; --i) {
    SEXP x = res[i];
    R_ReleaseObject(x);
    res.pop_back();
  }
}

[[cpp11::export]] void protect_many_rcpp_(int n) {
  std::vector<Rcpp::RObject> res;
  for (R_xlen_t i = 0; i < n; ++i) {
    SEXP x = Rf_ScalarInteger(n);
    res.push_back(x);
  }

  for (R_xlen_t i = n - 1; i >= 0; --i) {
    SEXP x = res[i];
    res.pop_back();
  }
}
