#!/usr/bin/python3
import subprocess as sp
from pathlib import Path
import shutil as sh
import argparse
import hashlib
from termcolor import colored

BUILD_DIR = Path('build')

def parse_arg():
    parser = argparse.ArgumentParser(description='build cpp libs & tests for testa.')
    parser.add_argument('--mode',
        dest='mode',
        action='store',
        choices=['debug', 'release'],
        default='debug',
        help='Set compile mode: debug, release.',
    )
    parser.add_argument('--prune',
        dest='prune',
        action='store_true',
        default=False,
        help='Prune everything in the build directory.',
    )
    parser.add_argument('--fmt',
        dest='fmtlib',
        action='store',
        choices=['std', 'fmtlib'],
        default='std',
        help='Pick a format lib to support. Can be `std`(for standard library) or `fmtlib`(for `https://fmt.dev/`)',
    )
    args = parser.parse_args()
    return args

def plan(args):
    global BUILD_DIR
    cmd = ['cmake', '-G', 'Ninja']
    if args.mode == 'debug':
        build_dir = BUILD_DIR / 'debug'
        cmd += ['-DCMAKE_BUILD_TYPE=Debug', '-DCMAKE_EXPORT_COMPILE_COMMANDS=ON']
    elif args.mode == 'release':
        build_dir = BUILD_DIR / 'release'
        cmd += ['-DCMAKE_BUILD_TYPE=Release']
    if args.fmtlib == 'std':
        cmd += ['-DFMTLIB=std']
    elif args.fmtlib == 'fmtlib':
        cmd += ['-DFMTLIB=fmtlib']
    else:
        raise Exception(f'unsupported fmtlib: {args.fmtlib}')
    cmd += ['../..']

    if args.prune and build_dir.exists():
        sh.rmtree(build_dir)
    build_dir.mkdir(parents=True, exist_ok=True)

    sp.check_call(cmd, cwd=build_dir)
    return build_dir

def build(args, build_dir):
    cmd = ['ninja', '--verbose']
    sp.check_call(cmd, cwd=build_dir)

def link_last(build_dir):
    last = BUILD_DIR / 'last'
    if last.exists():
        last.unlink()
    last.symlink_to(build_dir.relative_to(BUILD_DIR))

if __name__ == '__main__':
    args = parse_arg()
    build_dir = plan(args)
    build(args, build_dir)
    link_last(build_dir)
