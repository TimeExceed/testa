#[macro_use(new_case)]
extern crate testa;

use std::collections::BTreeMap;
use std::env;
use std::fmt::format;
use testa::{is, eq, verify, go};


fn gcd(a: i32, b: i32) -> i32 {
    assert!(a >= 0);
    assert!(b >= 0);
    assert!(a > 0 || b > 0);
    if a == 0 {
        b
    } else {
        gcd(b % a, a)
    }
}

fn verify_gcd(res: &i32, inp: &(i32, i32)) -> Option<String> {
    let a = inp.0;
    let b = inp.1;
    if a % res == 0 && b % res == 0 {
        None
    } else {
        Some(format(format_args!("wrong answer: {:?} by applying {:?}", res, inp)))
    }
}

fn gcdtb(case: &Fn(&(i32,i32))->Option<String>) -> Option<String> {
    for i in 0..13 {
        for j in 0..13 {
            if i > 0 || j > 0 {
                let r = case(&(i, j));
                if r.is_some() {
                    return r;
                }
            }
        }
    };
    None
}

fn multiply_tb(case: &Fn(&(i32, i32))->Option<String>) -> Option<String> {
    for i in 1..6 {
        for j in 1..6 {
            let r = case(&(i, j));
            if r.is_some() {
                return r;
            }
        }
    };
    None
}

fn main() {
    let mut cases = BTreeMap::new();
    new_case!(cases, "testTesta.correctAddition",
              is(
                  || 1 + 2,
                  3));
    new_case!(cases, "testTesta.wrongAddition",
              is(
                  || 2 + 2,
                  3));
    new_case!(cases, "testTesta.correctMultiple",
              eq(
                  |a: &(i32, i32)| -> i32 {a.0 * a.1},
                  |a: &(i32, i32)| -> i32 {
                      let x = a.0;
                      let y = a.1;
                      let mut r: i32 = 0;
                      for _ in 0..y {
                          r = r + x;
                      };
                      r
                  },
                  |case: &Fn(&(i32,i32))->Option<String>| {
                      for i in 1..6 {
                          for j in 1..6 {
                              let r = case(&(i, j));
                              if r.is_some() {
                                  return r;
                              }
                          }
                      };
                      None
                  }));
    new_case!(cases, "testTesta.wrongMultiple",
              eq(
                  |a: &(i32, i32)| -> i32 {a.0 + a.1},
                  |a: &(i32, i32)| -> i32 {
                      let x = a.0;
                      let y = a.1;
                      let mut r: i32 = 0;
                      for _ in 0..y {
                          r = r + x;
                      };
                      r
                  },
                  multiply_tb));
    new_case!(cases, "testTesta.correctGcd",
              verify(
                  |a: &(i32, i32)| -> i32 {gcd(a.0, a.1)},
                  verify_gcd,
                  gcdtb));
    new_case!(cases, "testTesta.wrongGcd",
              verify(
                  |a: &(i32, i32)| -> i32 {
                      if a.0 == 1 {
                          10
                      } else {
                          gcd(a.0, a.1)
                      }
                  },
                  verify_gcd,
                  gcdtb));
    
    go(&env::args().collect(), &mut cases);
}

