<!---
This readme is writted with the markdown syntax.
To generate a pdf from it, use pandoc (need texlive) like: pandoc README.md -o readme.pdf
-->
# DCM-MD
***

DCM-MD (Discriminant chronicles mining - Multi-dimensional) is a C++ implementation of a modified 
implementation of the DCM algorithm proposed by Yann Dauxais et al., working with multidimensional data.

## License

The code of the multidimensional extension is written by **Radek Buša** and belongs to the **Czech Technical University in Prague**.

The original code, except files present in **_Ripper/ripper/code_**, is written by **Yann Dauxais** and belongs to the **University of Rennes 1**.

This code is licensed under the **BSD** License - see the [**LICENSE.md**](LICENSE.md) file for details.

The files present in **_Ripper/ripper/code_** belong to **AT&T** and were written by **William W. Cohen**.

## Building

To compile DCM-MD, just use the **_CMakeLists.txt_** in the root directory.

```
mkdir build
cd build
cmake ..
make
```

The compiled executable `DCM` will reside in the `build` directory.

## Running unit tests

All the tests are located in separate CMake project in the `tests/` directory, running the test
executable after the compilation completes will perform all tests.

```
cd tests
mkdir build
cd build
cmake ..
make
./DCM_tests
```

## Compilation Requirements

Compilation tested on compiler `Apple clang version 11.0.0`.

The implementation requires the `<boost/program_options.hpp>`, `libomp` and `libtool` libraries that must be installed.

The unit tests require the `catch2` unit testing library to be installed on your system.

## Examples

The directories **_datasets_old_** and **_datasets_new_** contain the datasets supplied with the original version of DCM
by Yann Dauxais et al. and a dataset with crystal growth data.

The sequences in those datasets are represented using the *one-sequence-per-line* format.

The uses of the command
`DCM datasets_new/proportionality_2D_legacy/pos.dat -d datasets_new/proportionality_2D_legacy/neg.dat --vecsize 2 --fmin 0.25 --gmin 2.0 --mincs 2 --maxcs 5`
will return 8 discriminant chronicles:

```
C: {["A", "B", "C"]}
0, 1: (<-inf,-inf>, <6,6>)
0, 2: (<-inf,-inf>, <9,9>)
1, 2: (<0,0>, <inf,inf>)
f: 25/6

C: {["A", "B", "C", "D"]}
0, 1: (<-inf,-inf>, <4,4>)
0, 2: (<-inf,-inf>, <inf,inf>)
0, 3: (<-inf,-inf>, <inf,inf>)
1, 2: (<-inf,-inf>, <inf,inf>)
1, 3: (<-inf,-inf>, <inf,inf>)
2, 3: (<6,6>, <inf,inf>)
f: 26/0

C: {["A", "B", "C", "D"]}
0, 1: (<-inf,-inf>, <inf,inf>)
0, 2: (<-inf,-inf>, <inf,inf>)
0, 3: (<-inf,-inf>, <inf,inf>)
1, 2: (<-inf,-inf>, <inf,inf>)
1, 3: (<-inf,-inf>, <inf,inf>)
2, 3: (<10,10>, <inf,inf>)
f: 50/0

C: {["A", "C"]}
0, 1: (<4,4>, <9,9>)
f: 27/12

C: {["A", "C", "D"]}
0, 1: (<-inf,-inf>, <inf,inf>)
0, 2: (<-inf,-inf>, <inf,inf>)
1, 2: (<10,10>, <inf,inf>)
f: 50/0

C: {["B", "C"]}
0, 1: (<0,0>, <6,6>)
f: 33/15

C: {["B", "C", "D"]}
0, 1: (<-inf,-inf>, <inf,inf>)
0, 2: (<-inf,-inf>, <inf,inf>)
1, 2: (<10,10>, <inf,inf>)
f: 50/0

C: {["C", "D"]}
0, 1: (<10,10>, <inf,inf>)
f: 50/0
```

Those chronicles are the discriminant multidimensional chronicles of the positive dataset **_pos.dat_**
in comparison to the negative dataset **_neg.dat_** which occurs in at least 25% of the
sequences of the positive dataset and 2 times in the positive than in the negative.

For example, for the first chronicle, `{["A", "B", "C"]}`
corresponds to the multiset of the chronicle.
The events are separated by commas.
The line `0, 1: (<-inf,-inf>, <6,6>)` corresponds to a hyperrectangle constraint of the chronicle.
`0` and `1` correspond to indices in the multiset, it is so a temporal constraint between
event type `A` and event type `B`. The temporal interval is defined by
`(<-inf,-inf>, <6,6>)` what means that the temporal constraint is `A [<-inf,-inf>, <6,6>] B`.
Finally, `50/0` corresponds to a positive support of 50 and a negative support of 0.

## <a name="args"></a>Argument options

The available parameters are listed in the help of the executable.
To print this help, use the parameter `--help` or simply run the executable without
parameters.

```
Usage:  DCM input_file fmin [options]
Positional Options (required):
  -i [ --input_file ] arg input file containing dataset to mine (string)
                          - positive dataset if --disc is used
                          positional : input_file
  -f [ --fmin ] arg       minimal frequency threshold (number)
                          Number of sequences if >= 1 (support)
                          Percent of positive sequences number else
                          positional fmin

General Options:
  --help                   Display this help message
  -d [ --disc ] arg        Extract discriminant chronicles using this file as
                           negative dataset
  --mincs arg              Minimum size of extracted chronicles
  --maxcs arg              Maximum size of extracted chronicles
  --tid                    Output the tid list in addition of extracted
                           chronicles (first line is 0) /!\ has to be proven to
                           really correspond to the right line numbers
  -c [ --close ]           Extract frequent closed chronicles or discriminant
                           chronicles from closed multisets if --disc is used
  -j [ --json ]            Output format is json instead of plain text
  -v [ --verbose ]         The program will speak
  -s [ --vecsize ] arg     Number of vector components in the input file
  -g [ --gmin ] arg        Minimal growth threshold
                           default : 2
```

## <a name="citations"></a>Citations

#### <a name="dcmmd"></a>Multi-dimensional discriminant chronicle mining

- Buša, Radek. Implementation of a generalized version of a system for discriminant chronicles mining. Master’s thesis. Czech Technical University in Prague, Faculty of Information Technology, 2020.

#### <a name="mddataset"></a>Multi-dimensional dataset materials

- Natasha Dropka, Martin Holena, et al. Fast forecasting of VGF crystal growth process by dynamic neural networks. In Journal of Crystal Growth 521. pp 9-14. Elsevier B.V., 2020. Available from https://doi.org/10.1016/j.jcrysgro.2019.05.022.

#### <a name="disc"></a>Discriminant chronicle mining

- Dauxais, Y., Guyet, T., Gross-Amblard, D., & Happe, A. (2017, June).
Discriminant Chronicles Mining. In Conference on Artificial Intelligence in Medicine
in Europe (pp. 234-244). Springer, Cham.
- Dauxais, Y., Gross-Amblard, D., Guyet, T., & Happe, A. (2017, January). 
Extraction de chroniques discriminantes. In Extraction et Gestion des Connaissances (EGC).

#### <a name="freq"></a>Frequent chronicle mining in sequence sets

- Dauxais, Y., Gross-Amblard, D., Guyet, T., & Happe, A. (2015, September).
Chronicles mining in a database of drugs exposures. In ECML Doctoral consortium.
- Huang, Z., Lu, X., & Duan, H. (2012).
On mining clinical pathway patterns from medical behaviors.
Artificial intelligence in medicine, 56(1), 35-50.
- Álvarez, M. R., Félix, P., & CariñEna, P. (2013).
Discovering metric temporal constraint networks on temporal databases.
Artificial intelligence in medicine, 58(3), 139-154.

#### <a name="bide-d"></a>BIDE-D materials

- Mörchen, F., Fradkin, D.: Robust mining of time intervals with semi-interval partial
order patterns, In Proceedings SIAM Conference on Data Mining, (2010), pp. 315-326
(http://www.mybytes.de/papers/moerchen10robust.pdf)
- Fradkin, D., & Mörchen, F. (2015). Mining sequential patterns for classification.
Knowledge and Information Systems, 45(3), 731-749.

#### <a name="mit-bih"></a>MIT-BIH materials
- Moody GB, Mark RG. The impact of the MIT-BIH Arrhythmia Database.
IEEE Eng in Med and Biol 20(3):45-50 (May-June 2001). (PMID: 11446209)
- Goldberger AL, Amaral LAN, Glass L, Hausdorff JM, Ivanov PCh, Mark RG, Mietus JE,
 Moody GB, Peng C-K, Stanley HE. PhysioBank, PhysioToolkit, and PhysioNet:
 Components of a New Research Resource for Complex Physiologic Signals.
 Circulation 101(23):e215-e220
 [Circulation Electronic Pages; http://circ.ahajournals.org/content/101/23/e215.full];
 2000 (June 13).
