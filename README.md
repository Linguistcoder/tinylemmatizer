# tinylemmatizer
Simple lemmatizer that uses the same lemmatization rules as CSTlemma. 

This project consists of a Python3 wrapper around a small C-program that lemmatizes full forms using a rule set ('flexrules') that is in the same binary format as the rule sets used by cstlemma (https://github.com/kuhumcst/cstlemma).
Rule sets can be trained using the affixtrain proram (https://github.com/kuhumcst/affixtrain). Alternatively, rule sets can be downloaded from https://github.com/kuhumcst/texton-linguistic-resources. Look for files in folders such as

    https://github.com/kuhumcst/texton-linguistic-resources/blob/master/da/lemmatiser/notags/c21/0/flexrules

or

    https://github.com/kuhumcst/texton-linguistic-resources/blob/master/en/lemmatiser/notags/2/flexrules

In these paths, the names of the subdirectories named '0' and '2' indicate the pruning level. '0' means unpruned.

To test, edit the file 'launch.py' and make sure that 'filename' is the path to a flexrule file.
