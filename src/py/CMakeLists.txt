add_subdirectory(amose)
# ########### install files ###############
install(PROGRAMS
    NindFile.py
    NindPadFile.py
    NindRetrolexicon.py
    NindIndex.py
    NindLexiconindex.py
    NindTermindex.py
    NindLocalindex.py
    Nind_search.py
    Nind_dumpDocument.py
    Nind_trouveNombresPremiers.py
  DESTINATION bin)

