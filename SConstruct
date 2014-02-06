#

env = Environment()

env.Append( CPPPATH = ['#./include','#./include/editor','/usr/include/libxml2'])
env.Append( CPPPATH = ['/usr/include'])

env.Append(CPPFLAGS = ['-Wall'])

env.Append(LIBPATH = ['/usr/local/lib','#./lib'])
env.Append(RPATH = ['lib/'])
env.Append( LIBS = ['GL','SDL_image','SDL','xml2'])

gameObjs = []
Export('env','gameObjs')

env.SConscript('src/SConscript')
src = ['src/Scroller.cpp']
gameLib = env.SharedLibrary ('Game',gameObjs)
env.Install ('#./lib', gameLib)
env.Program('./scroller', src,LIBS=['Game','GL','SDL_image','SDL','xml2'])

# tileset maker
env.Program ('./tset-maker', 'src/editor/TilesetMaker.cpp',LIBS=['Game','GL','SDL_image','SDL','xml2'])

AddOption ('--editor', dest='doEditor')
#===========================================
#Tile Editor
if GetOption('doEditor') or GetOption('clean'):
	edEnv = env.Clone ()
	edEnv.Append( CPPPATH = ['/usr/lib/gtkglext-1.0/include'])
	edEnv.Append( CPPPATH = ['/usr/include/gtkglext-1.0'])
	edEnv.ParseConfig('pkg-config --cflags --libs gtk+-2.0')
	edEnv.ParseConfig('pkg-config --cflags --libs gtkglext-1.0')
	editorObjs = []
	Export('edEnv','editorObjs')
	edEnv.SConscript('src/editor/SConscript')



	edEnv.Append( LIBS = ['Game', 'glut','GL','SDL_image','SDL','xml2'])
	edEnv.Program('./editor', editorObjs)

