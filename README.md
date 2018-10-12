# mutator
Print passwords derived from a given text in order of likelihood.

## Compilation
```
cd src/
make clean && make mutator
```

After compilation invoke with:
```
./mutator \<seed text\> \[\<optional custom frequency data\>\]
```

## Generating custom frequency data
Frequency data is generated from a list of passwords and a list of words that
some of the passwords are derived from.

Compilation:
```
cd freqdata-generator/
make clean && make
```

After the generator is compiled supply the passwords and wordlist file with:
```
cat passwords | ./generator wordlist > your_output_file
```
