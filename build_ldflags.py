import sys
import sysconfig

pyver = sysconfig.get_config_var('VERSION')
getvar = sysconfig.get_config_var

abiflags = getattr(sys, 'abiflags', '')
libs = ['-lpython' + pyver + abiflags]
libs += getvar('LIBS').split()
libs += getvar('SYSLIBS').split()
# add the prefix/lib/pythonX.Y/config dir, but only if there is no
# shared library in prefix/lib/.
if not getvar('Py_ENABLE_SHARED'):
    libs.insert(0, '-L' + getvar('LIBPL'))
if not getvar('PYTHONFRAMEWORK'):
    libs.extend(getvar('LINKFORSHARED').split())
print(' '.join(libs))


