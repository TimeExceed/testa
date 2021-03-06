// This file is picked from project testa [https://github.com/TimeExceed/testa.git]
// Copyright (c) 2013, Taoda (tyf00@aliyun.com)
// All rights reserved.

// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:

// * Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.

// * Redistributions in binary form must reproduce the above copyright notice, this
//   list of conditions and the following disclaimer in the documentation and/or
//   other materials provided with the distribution.

// * Neither the name of the {organization} nor the names of its
//   contributors may be used to endorse or promote products derived from
//   this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
// ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
// ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#![feature(fnbox)]

use std::collections::BTreeMap;
use std::cmp::Eq;
use std::fmt::{Debug, format};
use std::boxed::FnBox;

#[macro_export]
macro_rules! new_case {
    ($cases: ident, $name: expr, $f: expr) => {
        assert!($cases.insert($name.to_string(), $f).is_none());
    };
}


pub type Case = Box<FnBox() -> Option<String>>;

pub fn is<T: Eq + Debug + 'static, F>(casefunc: F, expect: T) -> Case
    where F: Fn() -> T + 'static {
    Box::new(move || {
        let actual = casefunc();
        if expect == actual {
            None
        } else {
            Some(format(format_args!("expect: {:?}, actual: {:?}", expect, actual)))
        }
    })
}

fn eq_composite<In: Debug, Out: Eq + Debug>(
    casefunc: &Fn(&In)->Out, oracle: &Fn(&In)->Out, inp: &In) -> Option<String>
{
    let actual: Out = casefunc(inp);
    let expect: Out = oracle(inp);
    if actual == expect {
        None
    } else {
        Some(format(format_args!("expect: {:?}, actual: {:?}, by applying {:?}", expect, actual, inp)))
    }
 }

pub fn eq<In: Debug, Out: Eq + Debug, F, H, G>(
    casefunc: F, oracle: H, testbench: G) -> Case
    where F: Fn(&In) -> Out + 'static,
          H: Fn(&In) -> Out + 'static,
          G: Fn(&(Fn(&In) -> Option<String>)) -> Option<String> + 'static {
    let c = move || {
        let cf = casefunc;
        let ora = oracle;
        let tb = testbench;
        tb(&(|inp: &In| -> Option<String> {
            eq_composite(&cf, &ora, inp)
        }))
    };
    Box::new(c)
}

fn ver_composite<In: Debug, Out: Eq + Debug>(
    casefunc: &Fn(&In)->Out,
    verifier: &Fn(&Out, &In) -> Option<String>,
    inp: &In) -> Option<String>
{
    let actual: Out = casefunc(inp);
    verifier(&actual, inp)
}

pub fn verify<In: Debug, Out: Eq + Debug, F, G, H>(
    casefunc: F, verifier: G, testbench: H) -> Case
    where F: Fn(&In) -> Out + 'static,
          G: Fn(&Out, &In) -> Option<String> + 'static,
          H: Fn(&(Fn(&In) -> Option<String>)) -> Option<String> + 'static {
    let c = move || {
        let cf = casefunc;
        let ver = verifier;
        let tb = testbench;
        tb(&(|inp: &In| -> Option<String> {
            ver_composite(&cf, &ver, inp)
        }))
    };
    Box::new(c)
}


#[derive(Debug)]
enum TestaError {
    Help,
    CaseFail(String),
    ArgsError(String),
}

impl From<String> for TestaError {
    fn from(s: String) -> TestaError {
        TestaError::ArgsError(s)
    }
}


fn ego(args: &Vec<String>, cases: &mut BTreeMap<String, Case>) -> Result<i32, TestaError> {
    if args.iter().any(|x| {x == "-h" || x == "--help"}) {
        return Result::Err(TestaError::Help);
    };
    match args.len() {
        2 => {
            let a = &args[1];
            if a == "--show-cases" {
                for x in cases.keys() {
                    println!("{}", x);
                }
            } else {
                match cases.remove(a) {
                    None => {
                        return Result::Err(TestaError::ArgsError(
                            std::fmt::format(format_args!("unknown arg: {}", a))))
                    },
                    Some(c) => {
                        if let Some(msg) = c() {
                            return Result::Err(TestaError::CaseFail(msg));
                        }
                    }
                }
            }
        },
        _ => {
            return Result::Err(TestaError::ArgsError("want exactly one arg".to_owned()));
        }
    };
    Ok(0)
}

fn print_usage(exe: &String) {
    println!("{} [--help|-h] [--show-cases] [CASENAME]", exe);
    println!("CASENAME\ta case name that will be executed");
    println!("--show-cases\ta list of case names, one name per line");
    println!("--help,-h\tthis help message");
}

pub fn go(args: &Vec<String>, cases: &mut BTreeMap<String, Case>) -> ! {
    let res = ego(args, cases);
    let rc = match res {
        Ok(_) => 0,
        Err(TestaError::Help) => {print_usage(&args[0]); 0},
        Err(TestaError::CaseFail(err)) => {println!("{}", err); 1},
        Err(TestaError::ArgsError(err)) => {println!("Error: {}", err); 2},
    };
    std::process::exit(rc);
}

