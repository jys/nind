add_subdirectory(NindLexicon)
add_subdirectory(NindBasics)
add_subdirectory(NindIndex)
add_subdirectory(NindAmose)
add_subdirectory(NindRetrolexicon)

########### install files ###############
install(FILES NindCommonExport.h NindExceptions.h DESTINATION include)
