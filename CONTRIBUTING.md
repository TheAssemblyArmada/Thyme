# Contributing to Thyme

First of all, thanks for considering contributing to the Thyme project.

The following is a quick overview of the different ways to contribute to the project. For more detailed guides, please check out the [wiki](https://github.com/TheAssemblyArmada/Thyme/).

### Ways To Contribute

* [Report Bugs](#report-bugs)
* [Produce Documentation](#produce-documentation)
* [Write Code](#write-code)

#### Report Bugs

So you've stumbled on a bug? First thing to do is to check that the bug was not already reported by searching on GitHub under [Issues](https://github.com/TheAssemblyArmada/Thyme/issues). If you find one already exists, you can add comments to it rather than opening a new one.

If an issue doesn't already exist, before you create one make sure you have the following information (basically follow the issue template):
 * If it happens in the vanilla game, only in Thyme or both.
 * Steps to reproduce the bug.
 * If the bug only happens with custom content, include a copy of the data or a link to it.
 * Any debug logs or crash logs produced.
 
#### Produce Documentation

Not into C++ or even programming in general? If you like writing, then you could write some documentation for the project. The project wiki has been made publically editable on github so anyone can contribute to it.

There are many kinds of documentation that the project needs, you can write how tos for Thyme itself, technical documentation to help in making tools to work with Thyme file formats or modding guides for adding custom content to Thyme.

#### Write Code

If you know a bit of C++ then you can write code for the project. At the time of writing the focus is on reimplementing code in a way that is [ABI](https://en.wikipedia.org/wiki/Application_binary_interface) compatible with the original binary so Thyme can use it to provide functions that Thyme itself hasn't implemented yet. This means that new code has restrictions on class layout and which compilers can produce working code (essentially only the Microsoft compiler).

There are two main types of contribution we are looking for code wise. First is reimplemented code that replaces functions Thyme currently needs the original binary to provide. Second is code that improves the existing Thyme code with new features or bugfixes.

