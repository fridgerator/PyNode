import sysconfig

flags = ['-I' + sysconfig.get_path('include'),
         '-I' + sysconfig.get_path('platinclude')]
print(' '.join(flags))

