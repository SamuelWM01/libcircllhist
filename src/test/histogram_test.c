#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <circllhist.h>

typedef histogram_t *(*halloc_func)();
halloc_func halloc = NULL;

static int tcount = 1;
static int failed = 0;
static char *test_desc = "??";
#define okf(fmt, ex...) do { \
  printf("ok %d - %s : " fmt "\n", tcount++, test_desc, ex); \
} while(0)
#define ok() do { \
  printf("ok %d - %s\n", tcount++, test_desc); \
} while(0)
#define notokf(fmt, ex...) do { \
  printf("not ok %d - %s : " fmt "\n", tcount++, test_desc, ex); \
  failed++; \
} while(0)
#define notok() do { \
  printf("not ok %d - %s\n", tcount++, test_desc); \
  failed++; \
} while(0)
static void is(int expr) { 
  if(expr) { ok(); } else { notok(); }
};
#define T(a) do { \
  test_desc = #a; \
  a; \
  test_desc = "??"; \
} while(0)

bool double_equals(double a, double b) {
  double r, diff, max = fabs(a);
  if(fabs(b) > max) max = fabs(b);
  if(max == 0) return true;
  diff = b-a;
  r = diff/max;
  if(fabs(r) < 0.0001) return true;
  return false;
}
void bucket_tests() {
  hist_bucket_t b, o;

  b = int_scale_to_hist_bucket(0,0);
  T(is(b.val == 0 && b.exp == 0));

  b = int_scale_to_hist_bucket(2,0);
  o = double_to_hist_bucket(2);
  T(is(b.val == o.val && b.exp == o.exp));

  b = int_scale_to_hist_bucket(1,-9);
  o = double_to_hist_bucket(1e-9);
  T(is(b.val == o.val && b.exp == o.exp));

  b = int_scale_to_hist_bucket(1300000000,-9);
  o = double_to_hist_bucket(1.3);
  T(is(b.val == o.val && b.exp == o.exp));

  b = int_scale_to_hist_bucket(-2700,-9);
  o = double_to_hist_bucket(-2.7e-6);
  T(is(b.val == o.val && b.exp == o.exp));

  b = int_scale_to_hist_bucket(7,-9);
  o = double_to_hist_bucket(7e-9);
  T(is(b.val == o.val && b.exp == o.exp));

  b = double_to_hist_bucket(0);
  T(is(b.val == 0 && b.exp == 0));

  b = double_to_hist_bucket(9.9999e-129);
  T(is(b.val == 0 && b.exp == 0));

  b = double_to_hist_bucket(1e-128);
  T(is(b.val == 10 && b.exp == -128));

  b = double_to_hist_bucket(1.00001e-128);
  T(is(b.val == 10 && b.exp == -128));

  b = double_to_hist_bucket(1.09999e-128);
  T(is(b.val == 10 && b.exp == -128));

  b = double_to_hist_bucket(1.1e-128);
  T(is(b.val == 11 && b.exp == -128));

  b = double_to_hist_bucket(1e127);
  T(is(b.val == 10 && b.exp == 127));

  b = double_to_hist_bucket(9.999e127);
  T(is(b.val == 99 && b.exp == 127));

  b = double_to_hist_bucket(1e128);
  T(is(b.val == -1 && b.exp == 0));

  // negative range

  b = double_to_hist_bucket(-9.9999e-129);
  T(is(b.val == 0 && b.exp == 0));

  b = double_to_hist_bucket(-1e-128);
  T(is(b.val == -10 && b.exp == -128));

  b = double_to_hist_bucket(-1.00001e-128);
  T(is(b.val == -10 && b.exp == -128));

  b = double_to_hist_bucket(-1.09999e-128);
  T(is(b.val == -10 && b.exp == -128));

  b = double_to_hist_bucket(-1.1e-128);
  T(is(b.val == -11 && b.exp == -128));

  b = double_to_hist_bucket(-1e127);
  T(is(b.val == -10 && b.exp == 127));

  b = double_to_hist_bucket(-9.999e127);
  T(is(b.val == -99 && b.exp == 127));

  b = double_to_hist_bucket(-1e128);
  T(is(b.val == -1 && b.exp == 0));

  b = double_to_hist_bucket(9.999e127);
  T(is(b.val == 99 && b.exp == 127));
}
void test1(double val, double b, double w) {
  double out, interval;
  hist_bucket_t in;
  in = double_to_hist_bucket(val);
  out = hist_bucket_to_double(in);
  interval = hist_bucket_to_double_bin_width(in);
  if(out < 0) interval *= -1.0;
  if(double_equals(b,out)) ok();
  else notokf("(%f bin %g != %g)\n", val, out, b);
  if(double_equals(w,interval)) ok();
  else notokf("(%f width %f != %f)\n", val, interval, w);
}

histogram_t *build(double *vals, int nvals) {
  int i;
  histogram_t *out = halloc();
  for(i=0;i<nvals;i++)
    hist_insert(out, vals[i], 1);
  return out;
}
void mean_test(double *vals, int nvals, double expected) {
  histogram_t *h = build(vals, nvals);
  double m = hist_approx_mean(h);
  if(double_equals(m,expected)) ok();
  else notokf("(mean() -> %g != %g)\n", m, expected);
  hist_free(h);
}
void q_test(double *vals, int nvals, double *in, int nin, double *expected) {
  double *out;
  histogram_t *h = build(vals, nvals);
  out = calloc(nin, sizeof(*out));
  int rv = hist_approx_quantile(h, in, nin, out);
  if(rv != 0) notokf("quantile -> %d", rv);
  else {
    int i;
    for(i=0;i<nin;i++) {
      if(!double_equals(out[i], expected[i])) {
        notokf("q(%f) -> %g != %g", in[i], out[i], expected[i]);
        return;
      }
    }
    ok();
  }
  hist_free(h);
}

int main() {
  bucket_tests();

  T(test1(43.3, 43, 1));
  T(test1(99.9, 99, 1));
  T(test1(10, 10, 1));
  T(test1(1, 1, 0.1));
  T(test1(0.0002, 0.0002, 0.00001));
  T(test1(0.003, 0.003, 0.0001));
  T(test1(0.3201, 0.32, 0.01));
  T(test1(0.0035, 0.0035, 0.0001));
  T(test1(-1, -1, -0.1));
  T(test1(-0.00123, -0.0012, -0.0001));
  T(test1(-987324, -980000, -10000));

  halloc = hist_alloc;
  for(int ai=0; ai<2; ai++) {
    double s1[] = { 0.123, 0, 0.43, 0.41, 0.415, 0.2201, 0.3201, 0.125, 0.13 };
    T(mean_test(s1, 9, 0.24444));
  
    double h[] = { 1 };
    double qin[] = { 0, 0.25, 0.5, 1 };
    double qout[] = { 1, 1.025, 1.05, 1.1 };
    T(q_test(h, 1, qin, 4, qout));
  
    double qin2[] = { 0, 0.95, 0.99, 1.0 };
    double qout2[] = { 0, 0.4355, 0.4391, 0.44 };
    T(q_test(s1, 9, qin2, 4, qout2));
  
    double s3[] = { 1.0, 2.0 };
    double qin3[] = { 0.5 };
    double qout3[] = { 1.1 };
    T(q_test(s3, 2, qin3, 1, qout3));
    halloc = hist_fast_alloc;
  }

  printf("%d..%d\n", 1, tcount-1);
  return failed ? -1 : 0;
}
