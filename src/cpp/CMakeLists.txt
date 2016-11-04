add_subdirectory(NindLexicon)
add_subdirectory(NindBasics)
add_subdirectory(NindIndex)
add_subdirectory(NindAmose)

########### install files ###############
install(FILES NindCommonExport.h NindExceptions.h DESTINATION include)

