# UoYWeek C
UoYWeek C gives the current date string formatted as used by the [University of York](https://www.york.ac.uk/). For example, `spr/3/wednesday` would refer to Wednesday on the 3rd week of Spring term.

_This is a reimplementation of [Luke Moll's UoYWeek](https://github.com/LukeMoll/uoyweek) in C rather than C++, full credit goes to him for idea and initial reference code used for this version._

## Installing
### Linux/Mac
```bash
# Clone the repository
git clone https://github.com/JMAlego/uoyweek_c.git
cd uoyweek_c
# To compile
make
# To install
make install
```
You now have an executable file, `uoyweek`. To run it while you're in `uoyweek_c/`, just type `./uoyweek`. 

If you installed it you should now be able to run `uoyweek` from any directory.

You must specify a terms file (defaults to `terms.txt` in the current working directory), an example of which can be found in this reposity as `terms.txt`.

You can also pass it the `--fancy` argument to get proper capitalisation, for example `Spr/3/Wednesday`.

And you can pass it the `--short` argument to get a shorter day name, for example `spr/3/wed`.

<!-- TODO Windows instructions -->
