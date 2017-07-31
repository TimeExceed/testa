# -*- python -*-
import os
import os.path as path
import shutil
import hashlib
import subprocess
import re

env = Environment()

env['BUILD_DIR'] = env.Dir('build')
if path.exists('/dev/shm'):
    env['WORK_DIR'] = env.Dir('/dev/shm/build')
else:
    env['WORK_DIR'] = env['BUILD_DIR']
env['CLOJURE'] = env['BUILD_DIR'].File('clojure-1.5.1.jar')
env['MANIFEST'] = {
    'Manifest-Version': '1.0',
    'Created-By': 'scons 2.3.0',
}

def walkDir(env, dir):
    d = dir.abspath
    for rt, dirs, files in os.walk(d):
        if '.git' in dirs:
            dirs.remove('.git')
        for f in files:
            f = path.join(rt, f)
            yield dir.File(path.relpath(f, d))

env.AddMethod(walkDir)

def cleanLinks():
    for rt, dirs, files in os.walk(env['BUILD_DIR'].abspath):
        for f in files:
            f = path.join(rt, f)
            if path.islink(f):
                os.remove(f)

def cleanTmpDirs():
    pat = re.compile('^[0-9a-f]{32}$')
    for rt, dirs, _ in os.walk(env['BUILD_DIR'].abspath):
        for d in dirs:
            if pat.match(d):
                dirs.remove(d)
                shutil.rmtree(path.join(rt, d))

def copyTree():
    dstDir = env['BUILD_DIR'].abspath
    def walk(srcDir):
        dstBase = path.basename(dstDir)
        dstRt = path.abspath(path.dirname(dstDir))
        for rt, dirs, files in os.walk(srcDir):
            if '.git' in dirs:
                dirs.remove('.git')
            if path.abspath(rt) == dstRt and dstBase in dirs:
                dirs.remove(dstBase)
            for f in files:
                yield path.join(rt, f)

    srcs = [x for x in walk(os.getcwd())]
    dsts = [path.join(dstDir, path.relpath(x)) for x in srcs]

    dirs = set(path.dirname(x) for x in dsts)
    for x in dirs:
        if path.exists(x) and not path.isdir(x):
            shutil.rmtree(x)
    for x in dirs:
        if not path.exists(x):
            os.makedirs(x)
    for src, dst in zip(srcs, dsts):
        os.symlink(src, dst)

def downloadClojure():
    r = subprocess.call(['wget', '-nc',
        '-O', env['BUILD_DIR'].File('clojure-1.5.1.jar').abspath,
        'http://search.maven.org/remotecontent?filepath=org/clojure/clojure/1.5.1/clojure-1.5.1.jar'])
    assert r in [0, 1]

def downloadFromClojars(prj, app, ver):
    jar = '%s-%s.jar' % (app, ver)
    r = subprocess.call(['wget', '-nc',
        '-O', env['BUILD_DIR'].File(jar).abspath,
        'https://clojars.org/repo/%s/%s/%s/%s' % (prj, app, ver, jar)])
    assert r in [0, 1]

def prepareBuildDir(env):
    if not path.isdir(env['BUILD_DIR'].abspath):
        os.makedirs(env['BUILD_DIR'].abspath)
    cleanLinks()
    cleanTmpDirs()
    copyTree()
    downloadClojure()
    downloadFromClojars('myguidingstar', 'clansi', '1.3.0')

prepareBuildDir(env)

# global builders and helpers

def parseClojurePackage(src):
    pkg = src.split(os.sep)
    pkg[-1] = path.splitext(pkg[-1])[0]
    return '.'.join(pkg).replace('_', '-')

def compileClojure(env, workdir, root, cljs, kwargs):
    if cljs:
        print 'compile clojures in %s into %s' % (root.path, workdir.path)
        warnOnReflection = 'true' if kwargs.get('warnOnReflection', False) else 'false'
        subprocess.check_call(['java',
            '-cp', '%s' %
                (':'.join(
                    [root.abspath, workdir.abspath, env['CLOJURE'].abspath] +
                    [x.abspath for x in kwargs['libs']]
                )),
            '-Dclojure.compile.path=%s' % workdir.abspath,
            '-Dclojure.compile.warn-on-reflection=%s' % warnOnReflection,
            'clojure.lang.Compile',
            ] + [
                parseClojurePackage(root.rel_path(x)) for x in cljs
            ])

def unjar(env, workdir, kwargs):
    for x in kwargs['libs'] + [env['CLOJURE']]:
        print 'unjar %s into %s' % (x.path, workdir.path)
        subprocess.check_call(['jar', 'xf', x.abspath], cwd=workdir.abspath)

def compile(env, workdir, srcDir, kwargs):
    root = env.Dir(path.dirname(srcDir.path))
    fs = [x for x in env.walkDir(srcDir)]
    cljs = set(x for x in fs if x.path.endswith('.clj'))
    compileClojure(env, workdir, root, cljs, kwargs)

def writeManifest(env, workdir, kwargs):
    manifest = env['MANIFEST']
    if 'main' in kwargs:
        manifest['Main-Class'] = kwargs['main']
    manifestFile = workdir.File('manifest').abspath
    with open(manifestFile, 'w') as f:
        for k, v in manifest.items():
            f.write('%s: %s\n' % (k, v))
    return manifestFile

def jar(env, dstJar, workdir, kwargs):
    print 'jar %s into %s' % (workdir.path, dstJar.path)
    manifestFile = writeManifest(env, workdir, kwargs)
    tmpJar = env['WORK_DIR'].File('%s.jar' % path.basename(workdir.abspath))
    tmpJar = tmpJar.abspath
    subprocess.check_call(['jar', 'cfm', tmpJar, manifestFile,
        '-C', workdir.abspath, '.'])
    subprocess.check_call(['jar', 'i', tmpJar])
    shutil.copy(tmpJar, dstJar.abspath)
    os.remove(tmpJar)

def _compileAndJar(target, source, env):
    kwargs = env['kwargs']
    workdir = kwargs['workdir']
    os.makedirs(workdir.abspath)
    dstJar = target[0]
    for srcDir in source:
        compile(env, workdir, srcDir, kwargs)
        if kwargs.get('standalone', False):
            unjar(env, workdir, kwargs)
        jar(env, dstJar, workdir, kwargs)
        shutil.rmtree(workdir.abspath)

def expandLibs(libs):
    res = list(set(env.File(x) for x in libs))
    res.sort(key=lambda x:x.abspath)
    return res

def compileAndJar(env, dstJar, srcDir, **kwargs):
    srcDir = [env.Dir(x) for x in env.Flatten([srcDir])]
    for x in srcDir:
        assert x.exists()
    dstJar = env.File(dstJar)

    workdir = env['WORK_DIR'].Dir(hashlib.md5(dstJar.abspath).hexdigest())
    if path.exists(workdir.abspath):
        shutil.rmtree(workdir.abspath)
    kwargs['workdir'] = workdir
    kwargs['libs'] = expandLibs(env.Flatten([kwargs.get('libs', [])]))

    env = env.Clone()
    env['kwargs'] = kwargs
    env.Append(BUILDERS={'_compileAndJar': Builder(
        action=_compileAndJar,
        suffix='.jar')})
    env._compileAndJar(target=dstJar, source=srcDir)
    env.Depends(dstJar, kwargs['libs'])
    for x in srcDir:
        for f in env.walkDir(x):
            env.Depends(dstJar, f)

    return dstJar

env.AddMethod(compileAndJar)

flags = {
    'CFLAGS': [], 
    'CXXFLAGS': [], #'--std=c++11'
    'CCFLAGS': ['-Wall', '-Wfloat-equal',
                '-O0', '--coverage', '-fsanitize=address', '-fvar-tracking-assignments',
                '-g', '-gdwarf-4'],
    'LINKFLAGS': ['--coverage', '-fsanitize=address']}
env.MergeFlags(flags)

def _Glob(self, *args, **kwargs):
    files = self.Glob_(*args, **kwargs)
    files.sort(key=str)
    return files
env.__class__.Glob_ = env.__class__.Glob
env.__class__.Glob = _Glob

def subdirs(self, *dirs):
    return self.SConscript(dirs=Flatten(dirs), exports={'env':self})
env.AddMethod(subdirs)

def erlc(target, source, env):
    for src in source:
        subprocess.check_call(['/usr/bin/erlc', src.abspath],
                              cwd=path.dirname(src.abspath))

env.Append(BUILDERS={'erlc': Builder(action=erlc, suffix='.beamer')})

def dialyzer(target, source, env):
    for src,tgt in zip(source, target):
        with open(tgt.abspath, 'w') as fp:
            subprocess.check_call(
                ['/usr/bin/dialyzer', src.abspath],
                stderr=subprocess.STDOUT,
                stdout=fp)

env.Append(BUILDERS={'dialyzer': Builder(action=dialyzer, suffix='.dialyzer')})

env.SConscriptChdir(1)
env.SConscript('$BUILD_DIR/SConscript', exports='env')
