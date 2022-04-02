# NIND, as "nouvelle indexation"

New Indexation method in the ``Amose`` search engine since the former ``S2`` and ``Lucene`` are deprecated.

In 2014, **Atejcon** developed and documented nind, a simple indexing module based on flat files. It was developed to meet the need for indexing an application such **ANT'inno**'s ``ANT'box`` but can undoubtedly be used for other more ambitious applications in terms of volume and speed of indexing and reading.

**CEA LVIC** used the Lucene indexer for its ``Amose`` engine and wished to find an alternative. nind was a candidate and had to prove its relevance with regard to interfacing and volume performance.
nind has been modified to adapt to the functional of ``Amose``, richer than the functional of ``S2``, in particular by the types of terms and the various measures on the corpus for the calculations of relevance.

nind uses a flat file system for indexing and searching. These files are accessible outside real-time to make different analyzes of the indexed corpus. The nind system can be analyzed at different levels using only the flat files themselves.
And outside real time, without anything from C ++ , Python classes allow you to consider all kinds of development to analyze the indexed corpus.

To control the coding of binary files, nind uses a grammar. **EBNF** (Extented Backus-Naur Form) was chosen. All binary files are thus specified in EBNF.
An immediate benefit of this grammar is that the files used by C++ programs
can be checked by Python programs using this grammar. The tests are therefore simpler to make and above all more complete.
<br>
And the hellish questioning of ``S2`` "Does it malfunction in indexing or research ?" is definitely outdated.
