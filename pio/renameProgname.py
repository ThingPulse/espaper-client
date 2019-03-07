Import("env")
print env

my_flags = env.ParseFlags(env['BUILD_FLAGS'])
build_flags = env.ParseFlags(my_flags['CPPDEFINES'])
#print build_flags['CLIENT_VERSION']
#print my_flags.get("CPPDEFINES").get("CLIENT_VERSION")

print defines
for key in my_flags['CPPDEFINES']:
    if isinstance(key, list) and key[0] == 'CLIENT_VERSION':
        print(key[1])
        env.Replace(PROGNAME="firmware_%s" % key[1])
