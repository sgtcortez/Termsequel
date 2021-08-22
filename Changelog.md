# Changelog
Contains information about the versions of Termsequel

- Release 1.0

    - Information

        Released at: 2021-08-21.  
        We use Cmake as the build tool.

    + Developers

        Developers that helped with commits, tests, ideas.

        * [sgtcortez](https://github.com/sgtcortez)

    - Features

        There is the list of features in this release.

        | Feature | Issue | Pull Request | Description |
        | :-:   | :-: | :-: | :-: |
        | Lexical Analysis | [ISSUE](https://github.com/sgtcortez/Termsequel/issues/2) | [PR](https://github.com/sgtcortez/Termsequel/pull/3) | Lexical analysisi of the user input. And converts to lexemes | 
        | Syntax Analysis | [ISSUE](https://github.com/sgtcortez/Termsequel/issues/4) | [PR](https://github.com/sgtcortez/Termsequel/pull/5) | Syntax analysis of lexemes and reporting errors |
        | Rewrite of Lexical and Syntax Analysis | NE | [PR](https://github.com/sgtcortez/Termsequel/pull/7) | Rewrite the analysis to make it more programmer friendly. Report more useful errors |
        | Owner colum | [ISSUE](https://github.com/sgtcortez/Termsequel/issues/10) | [PR](https://github.com/sgtcortez/Termsequel/pull/20) | Add the file owner column |
        | Where support | [ISSUE](https://github.com/sgtcortez/Termsequel/issues/14) | [PR](https://github.com/sgtcortez/Termsequel/pull/26) | Add support for where filters |  
        | Level column | [ISSUE](https://github.com/sgtcortez/Termsequel/issues/13) | [PR](https://github.com/sgtcortez/Termsequel/pull/28) | Add the level column, now, users can set the level(directory depth) of a file |
        | File type column | [ISSUE](https://github.com/sgtcortez/Termsequel/issues/9) | [PR](https://github.com/sgtcortez/Termsequel/pull/30) | Add the file type column |
        | Filename column display | [ISSUE](https://github.com/sgtcortez/Termsequel/issues/34) | [PR](https://github.com/sgtcortez/Termsequel/pull/38) | The file name column, is displayed with the relative path |
        | Fix filter conditions too slow | [ISSUE](https://github.com/sgtcortez/Termsequel/issues/36) | [PR](https://github.com/sgtcortez/Termsequel/pull/39) | Filters to slow, because, they were applied at the result set | 
        | Implement support for Windows | [ISSUE](https://github.com/sgtcortez/Termsequel/issues/11) | [PR](https://github.com/sgtcortez/Termsequel/pull/44) | Now, termsequel is available on Windows systems |
        | Set CMake as the build tool | [ISSUE](https://github.com/sgtcortez/Termsequel/issues/1) | [PR](https://github.com/sgtcortez/Termsequel/pull/45) | Using CMake, makes the development and the cross compile easy |
        | File permissions columns | [ISSUE](https://github.com/sgtcortez/Termsequel/issues/12) | [PR](https://github.com/sgtcortez/Termsequel/pull/29) | Add support for file permissions columns |

- Release 1.0.1

    - Bug Fixes

        | Feature | Issue | Pull Request | Description |
        | :-:   | :-: | :-: | :-: |    
        | Equal operators consider only the user input string | [ISSUE](https://github.com/sgtcortez/Termsequel/issues/56) | [PR](https://github.com/sgtcortez/Termsequel/pull/58) | The equal operation was not working as expected to be |

- Release 1.1

    - Information

        Release date is undefined for now.

    + Developers

        Developers that helped with commits, tests, ideas.

        * [sgtcortez](https://github.com/sgtcortez)

    - Features

        There is the list of features in this release.

        | Feature | Issue | Pull Request | Description |
        | :-:   | :-: | :-: | :-: |
        | Add LAST_MODIFICATION column | [ISSUE](https://github.com/sgtcortez/Termsequel/issues/32) | [PR](https://github.com/sgtcortez/Termsequel/pull/47) | Add the last modification date of a file |
        | Support for negate expressions | [ISSUE](https://github.com/sgtcortez/Termsequel/issues/40) | [PR]() | Add three new operators, to be used as the negate form |
