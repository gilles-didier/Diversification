# Diversification
Diversification and fossilization rates estimation from the fossil record

Directory "bin" contains a standalone osX application and a Windows application (to use the Windows application, unzip the folder and click on the file "Diversification.exe").

Directory "data" contains the supplementary material of "Estimating diversification rates from the fossil record", in particular an example of tree with fossil information ("SOM 1 Eupelycosauria tree.txt"). It also contains a file storing the optimization settings which were used for the examples in the paper and can be loaded via the "option dialog" of the software ("Optimisation_parameters.nlopt").

Directory "src" contains the C (application core and console version) and C++ (GUI version) sources of all the softwares. The Makefile yields to compile all the console applications (see the README file in src) while "DiversificationV1.0.pro" file may be open by QtCreator to build the GUI software.
