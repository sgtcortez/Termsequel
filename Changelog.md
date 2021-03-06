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
    
- Release 1.0.2

    - Bug Fixes

        | Feature | Issue | Pull Request | Description |
        | :-:   | :-: | :-: | :-: |    
        | Where conditions does not work well | [ISSUE](https://github.com/sgtcortez/Termsequel/issues/76) | [PR](https://github.com/sgtcortez/Termsequel/pull/81) | When using three or more conditions, termsequel returns wrong results |
        

- Release 1.1

    - Information

        Released at: 2021-09-07.   

    + Developers

        Developers that helped with commits, tests, ideas.

        * [sgtcortez](https://github.com/sgtcortez)

    - Features

        There is the list of features in this release.

        | Feature | Issue | Pull Request | Description |
        | :-:   | :-: | :-: | :-: |
        | Add LAST_MODIFICATION column | [ISSUE](https://github.com/sgtcortez/Termsequel/issues/32) | [PR](https://github.com/sgtcortez/Termsequel/pull/47) | Add the last modification date of a file |
        | Support for negate expressions | [ISSUE](https://github.com/sgtcortez/Termsequel/issues/40) | [PR]() | Add three new operators, to be used as the negate form |
        | Add support for start pseudo column | [ISSUE](https://github.com/sgtcortez/Termsequel/issues/51) | [PR](https://github.com/sgtcortez/Termsequel/pull/57) | Add support for the **\***(STAR) pseudo column. |
        | Add RELATIVE_PATH column | [ISSUE](https://github.com/sgtcortez/Termsequel/issues/53) | [PR](https://github.com/sgtcortez/Termsequel/pull/60) | Added the relative path column |
        | Add ABSOLUTE_PATH column | [ISSUE](https://github.com/sgtcortez/Termsequel/issues/52) | [PR](https://github.com/sgtcortez/Termsequel/pull/64) | Added the absolute path column |  
        | Add support for variables | [ISSUE]() | [PR](https://github.com/sgtcortez/Termsequel/pull/69) | Now, it is possible to use environment variables, and terminal variables(if supported by your terminal) with termsequel **FROM** source.  
        | Add CREATION_DATE column | [ISSUE](https://github.com/sgtcortez/Termsequel/issues/31) | [PR](https://github.com/sgtcortez/Termsequel/pull/77) | Now, the file creation date is available |
        | Add NOT EQUAL comparasion | [ISSUE](https://github.com/sgtcortez/Termsequel/issues/74) | [PR](https://github.com/sgtcortez/Termsequel/pull/78) | Now, it is possible to filter by not equal values |

- Release 1.1.1

    - Bug Fixes

        | Bug | Issue | Pull Request | Description |
        | :-:   | :-: | :-: | :-: |    
        | Cant use on systems with glibc older than 2.28 | [ISSUE](https://github.com/sgtcortez/Termsequel/issues/82) | [PR](https://github.com/sgtcortez/Termsequel/pull/83) | Now, we call **stax** system call directly |         
        | Creation Date overwrite Last Modification | [ISSUE](https://github.com/sgtcortez/Termsequel/issues/84) | [PR](https://github.com/sgtcortez/Termsequel/pull/85) | Since, the add of statx syscall, we overwrite the last modification date|
        | Star do not include CREATION_DATE  | [ISSUE](https://github.com/sgtcortez/Termsequel/issues/86) | [PR](https://github.com/sgtcortez/Termsequel/issues/87) | When select with star, it do not include the new creation date column | 

- Release 1.1.2

    - Bug Fixes

        | Bug | Issue | Pull Request |
        | :-:   | :-: | :-: | :-: |    
        | Double declaration of statx structure | [ISSUE](https://github.com/sgtcortez/Termsequel/issues/93) | [PR](https://github.com/sgtcortez/Termsequel/pull/94) |

- Release 1.1.3

    - Bug Fixes

        | Bug | Issue | Pull Request |
        | :-:   | :-: | :-: |
        | Whitespace between date & time information | [ISSUE](https://github.com/sgtcortez/Termsequel/issues/91) | [PR](https://github.com/sgtcortez/Termsequel/pull/99) |    
        
- Release 1.2

    - Information

        Release date not defined.   

    + Developers

        Developers that helped with commits, tests, ideas.

        * [sgtcortez](https://github.com/sgtcortez)

    - Features

        There is the list of features in this release.

        | Feature | Issue | Pull Request | Description |
        | :-:   | :-: | :-: | :-: |

    - Fixes

        There is a list of fix that were fixed in this release.   
        | Fix | Issue | Pull Request |
        | :-:   | :-: | :-: |
        | Windows, does not have the implemention of [getopt](https://www.man7.org/linux/man-pages/man3/getopt.3.html), so, we could not use for example: `$ termsequel -h` to see the help ... | [ISSUE](https://github.com/sgtcortez/Termsequel/issues/88) | [PR](https://github.com/sgtcortez/Termsequel/pull/90)  |
        | Max level is now a command line argument, instead of a filter condition | [ISSUE](https://github.com/sgtcortez/Termsequel/issues/80) | [PR](https://github.com/sgtcortez/Termsequel/pull/92) |

    - Features

        There is a list of features from this release.

        | Feature | Issue | Pull Request |
        | :-:   | :-: | :-: | :-: |
        | Add GROUP column for linux systems | [ISSUE](https://github.com/sgtcortez/Termsequel/issues/75) | [PR](https://github.com/sgtcortez/Termsequel/pull/97) |
