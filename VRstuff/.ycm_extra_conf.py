def FlagsForFile(filename, **kwargs ):
  return {
    'flags': [ '-x','c++' ,'-Wall', '-Wextra', '-Werror' ,'-I./include/','-I../../PakkiUtils/','-I./shared/'
        ,'-I./game/include']
  }
    
# ,'-IC:/Users/Pate/Desktop/PakkiUtils']
