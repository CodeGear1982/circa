import os, sys, ConfigParser, SCons

def fatal(msg):
    print "fatal:",msg
    exit(1)

# Load build.config
config = ConfigParser.ConfigParser()
config.read('build.config')

POSIX = os.name == 'posix'
WINDOWS = os.name == 'nt'

# Check for a Mac-like environment. On Macs, 'POSIX' is true, but there is
# a little bit of special behavior for using frameworks.
OSX = os.path.exists('/Library/Frameworks')

DEBUG = Environment(tools = ["default"], toolpath=".")
RELEASE = Environment(tools = ["default"], toolpath=".")
DEBUG['variant_name'] = 'debug'
RELEASE['variant_name'] = 'release'
ALL = [DEBUG, RELEASE]

SHARED_LIBRARY = config.get('circa', 'shared_library') == 'true'

# Build flags
if POSIX:
    def common_flags(env):
        env.Append(CPPFLAGS=['-ggdb', '-Wall','-fasm-blocks'])
        env.Append(LINKFLAGS=['-ldl'])

        if config.get('circa', 'gprof_support') == 'true':
            env.Append(CPPFLAGS=['-pg'])
            env.Append(LINKFLAGS=['-pg'])

        env.SetOption('num_jobs', 2)
    map(common_flags, ALL)
    DEBUG.Append(CPPDEFINES = ["DEBUG"])
    RELEASE.Append(CPPFLAGS=['-O1'])

if WINDOWS:
    def common_flags(env):
        env.Append(CPPDEFINES = ['WINDOWS'])
        env.Append(LINKFLAGS='/SUBSYSTEM:CONSOLE /MACHINE:X86 /DEBUG'.split())
    map(common_flags, ALL)
    DEBUG.Append(CPPFLAGS='/EHsc /W3 /MDd /Z7 /TP /Od'.split())
    DEBUG.Append(LINKFLAGS=['/NODEFAULTLIB:msvcrt.lib'])
    DEBUG.Append(CPPDEFINES = ["DEBUG", "_DEBUG"])
    RELEASE.Append(CPPFLAGS='/EHsc /W3 /MD /Z7 /TP /O2'.split())
    RELEASE.Append(CPPDEFINES = ["NDEBUG"])

def list_source_files(dir):
    for path in os.listdir(dir):
        full_path = os.path.join(dir,path)
        if not os.path.isfile(full_path): continue
        if not path.endswith('.cpp'): continue
        yield path

circa_libs = {}

# Define library builds, save the results in circa_libs.
def circa_library(env):
    env = env.Clone()
    variant_name = env['variant_name']

    env.BuildDir('build/'+variant_name+'/src', 'src')
    env.Append(CPPPATH = ['src'])

    source_files = (list(list_source_files('src')) + 
            ['generated/'+filename for filename in list_source_files('src/generated')])
    source_files = filter(lambda f: f != 'main.cpp', source_files)

    baseName = 'circa' + ('_d' if variant_name == 'debug' else '')
    fullPath = 'build/'+baseName
    sources = ['build/'+variant_name+'/src/'+filename for filename in source_files]

    buildStep = env.SharedLibrary if SHARED_LIBRARY else env.StaticLibrary

    result = buildStep(fullPath, sources)
    circa_libs[variant_name] = result

    if OSX and SHARED_LIBRARY:
        actualFile = 'lib'+baseName+'.dylib'
        env.AddPostAction(result, 'install_name_tool -id @executable_path/'+actualFile
                + ' build/'+actualFile)

map(circa_library, ALL)

# Define command-line app builds, save the results in circa_cl_builds
circa_cl_apps = {}

def circa_command_line_app(env):
    variant_name = env['variant_name']
    env.Append(CPPPATH = ['src'])
    path = 'build/circa'

    libs = [circa_libs[variant_name]]
    
    result = env.Program(path, 'build/'+variant_name+'/src/main.cpp',
            LIBS=libs)
    circa_cl_apps[variant_name] = result

    if OSX and SHARED_LIBRARY:
        env.AddPostAction(result, 'install_name_tool -change build/libcirca_d.dylib '
                + '@executable_path/libcirca_d.dylib '+path)
    return result
    
# Default build target is debug command-line binary.
circa_cl = circa_command_line_app(DEBUG)
Default(circa_cl)
Alias('circa', circa_cl)

########################### SDL-based targets ###############################

def sdl_env(env):
    if POSIX:
        # import path so that we will find the correct sdl-config
        env['ENV']['PATH'] = os.environ['PATH']
        try:
            env.ParseConfig('sdl-config --cflags')
            env.ParseConfig('sdl-config --libs')
        except OSError:
            pass
        env.Append(LIBS = ['SDL_gfx','SDL_image','SDL_ttf'])

        if OSX:
            env['FRAMEWORKS'] = ['OpenGL']
            env.Append(CPPDEFINES = ['PLASTIC_OSX'])
        else:
            env.Append(LIBS = ['libGL'])

    if WINDOWS:
        env.Append(LIBS=['opengl32.lib'])

        env.Append(CPPPATH=['build/deps/include'])
        env.Append(LIBS=['build/deps/lib/SDL.lib'])
        env.Append(LIBS=['build/deps/lib/SDLmain.lib'])
        env.Append(LIBS=['build/deps/lib/SDL_image.lib'])
        env.Append(LIBS=['build/deps/lib/SDL_mixer.lib'])
        env.Append(LIBS=['build/deps/lib/SDL_ttf.lib'])
        env.Append(LIBS=['build/deps/lib/glew32.lib'])

    env.Append(CPPPATH=['#src'])

    variant_name = env['variant_name']

    # Append appropriate circa lib
    env.Append(LIBS = [circa_libs[variant_name]])

def build_plastic(env):
    env = env.Clone()
    env.BuildDir('build/plastic/src', 'plastic/src')
    sdl_env(env)

    source_files = list_source_files('plastic/src')

    env.Append(CPPDEFINES=['PLASTIC_USE_SDL'])

    result = env.Program('#build/plas',
        source = ['build/plastic/src/'+f for f in source_files])

    # On Windows, embed manifest
    if WINDOWS:
        env.AddPostAction(result,
        'mt.exe -nologo -manifest plastic/windows/plastic.manifest -outputresource:$TARGET;1')
    return result
    
plastic_variant = config.get('plastic', 'variant')
if plastic_variant == "release":
    plastic_env = RELEASE
elif plastic_variant == "debug":
    plastic_env = DEBUG
else:
    fatal("Unrecognzied variant under [plastic]: " + plastic_variant)
plastic = build_plastic(plastic_env)

Alias('plastic', plastic)
