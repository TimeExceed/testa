Import('env')

testa = env.compileAndJar('testa.jar', 'testa')
env.compileAndJar('example.jar', 'example', libs=[testa], standalone=True,
  main='example.main')
env.compileAndJar('runtests.jar', 'runtests',
  libs=[env['BUILD_DIR'].File('clansi-1.3.0.jar')], standalone=True,
  main='runtests.main')
env.InstallAs('puretest.jar', 'example.jar')
