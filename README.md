# Summary
- [Termsequel](#Termsequel)    
    - [What is](#What-is)    
    - [Dependencies](#Dependencies)
    - [Goals](#Goals)
        + [Personal](#Personal)
        + [Project](#Project)
    - [Installation](#Installation)
- [Users](#Users)
    + [Features](#Features)
        + [Columns](#Columns)
        + [SELECT](#SELECT)
        + [WHERE](#WHERE)
        + [Variables](#Variables)
- [Developers](#Developers)

# Termsequel
Bringing SQL to our filesystem.      
![Termsequel](./.assets/Termsequel-logo.png)

Open Source Project hosted on [Github](https://github.com/sgtcortez/Termsequel) [Site](https://sgtcortez.github.io/Termsequel/)

- ## What is

    Termsequel, is an ideia that I have been thought a time ago(since, 2018 I guess), but, I had no ideia in how to implement it.  
    


- ## Dependencies

    To execute the binary, you will need just one dependency! A implementation of the [C++ standard library](https://en.wikipedia.org/wiki/C%2B%2B_Standard_Library).   
    **Note:** The implementation might be installed by default in your operating system. If you are on an embedded system, you may install it by yourself.

    If you want to compile, then, you will need a C++ compiler. I suggest [**clang**](https://clang.llvm.org/)!

    **If you are running in Linux, you will need at the Kernel 4.11 to run Termsequel(because of the stax syscall).**

- ## Goals

    What are my goals and **Termsequel** goals?

    + ### Personal

        For me, I always wanted to learn C++ and Compilers. Well, at least the concepts of both.  
        And, I always wanted to have some project that really helps me on my daily routines, and **Termsequel** helps me. Because, it really helps me, I decided to make it open source, and try to help someone else(This is the objective of an Open Source Project, right? :grimacing: ).  

        I have never learned the [find](https://www.man7.org/linux/man-pages/man1/find.1.html) command very well, and I believe that it has a lot of information and does a lot of things(but is very useful).  

        I am not building **Termsequel** to be a replacement of **find**, not this, but, an alternative tool that is more user friendly for easy and basic tasks, like, search for a *.exe* file in a directory.

    + ### Project

        **Termsequel** aims to help programmers, system administrators, hackers and even, non technical people who likes to learn new things.     
        **Termsequel** aims to be the most easy(this is why, SQL is chosen) to people. Plataform independent(work in progress), no external dependencies(yes, I could use the [Boost](https://www.boost.org/)) because, first of all, I want to learn, and, there is no need for an external library. This project is simple(yet).

- ## Installation

    You will need [CMake](https://cmake.org/) and, one C++ compiler(clang, gcc, MSVC ...) to build the project!    
    Just, create a directory called `build` and, then execute:   
    ```shell
    $ cd build
    $ cmake ../
    $ cmake --build .
    ```
    Since, this project is easy for CMake, it can detect the operating system and the available compiler.

# Users
Some advices to run **Termsequel**.   
The system is not thread safe, because, there is no need to this. Well, at least for now.    

You will need to provide the command in quotes or double quotes, otherwise, it will be different command line arguments.    
**Example:** Running Termsequel   
```bash
$ termsequel 'SELECT NAME FROM /HOME'
```

- ## Features

    Some features that are ready to be used. For now, just the SELECT command is available, but, I want to implement some DML commands.    

    For now, you can not make joins, select from multiple tables, and, I am not sure if it will be implemented.     
    
    - ### Columns

        The columns, is what you can obtain(just obtain, because where is not supported yet) of information in a command.
    
        * NAME

            The name of the file.

        + SIZE

            The size in bytes of the file.

        + OWNER 

            The owner of the file.

        + GROUP

            The group owner of the file.  
            **Obs:** Only available for Linux.

        + LEVEL

            The depth level. Considering the from value as the relative path(if a directory).
            
        + FILE_TYPE

            The type of the file.

        + OWNER_PERMISSIONS

            Returns the permissions of the owner of the file.     
            The value, must be in octal mode!

        + GROUP_PERMISSIONS

            **Just for Linux**.    
            Returns the permissions of the group of the file.     
            The value, must be in octal mode!

        + OTHERS_PERMISSIONS

            **Just for Linux**.    
            Returns the permissions of other users.    
            The value, must be in octal mode!

        + LAST_MODIFICATION

            The datetime when the last modification ocurred.

        + STAR pseudo column

            Use this, when you want to select all the available columns ...
            For example:
            ```sql
            SELECT * FROM .
            ```
            Means, select "all" the columns from .   
            **Note:** You can not use the **star** in the **WHERE** statement!

        + RELATIVE_PATH

            The relative path of the file.

        + ABSOLUTE_PATH

            The absolute path of the file.

        + CREATION_DATE

            The creation date of the file.

    - ### SELECT

        The same syntax as a SQL database.    
        **Termsequel SELECT syntax:** `SELECT COLUMN_LIST FROM FILE` 

        **Example:** *"SELECT"* syntax 
        ```sql
        SELECT NAME FROM <FILE>    
        -- <FILE> is a placeholder. File can be a regular file, and a directory. Can even be pseudo directories
        SELECT NAME FROM . -- Current directory
        SELECT NAME FROM ../ -- Parent directory
        ```

        **Example:** Selecting names from user documents directory.    
        ```sql
        SELECT NAME FROM /home/user/documents
        ```

    - ### WHERE

        Now, **Termsequel** supports **WHERE** filters!   
        You can write filters like:   
        ```sql
        SELECT NAME FROM . WHERE OWNER = myuser
        ```
        Even, you can group logical operators(for now, they are just left precedence!)  
        ```sql
        SELECT NAME FROM . WHERE OWNER = myuser AND SIZE > 10
        ```
        The logical operators are: **AND** and **OR**.    

        **Example:** Check if owner has execute permission in a file:    
        ```sql
        SELECT NAME, OWNER_PERMISSIONS FROM . WHERE OWNER_PERMISSIONS CONTAINS X
        -- Returns the files that the owner has the execute flag set.
        SELECT NAME, OWNER_PERMISSIONS FROM . WHERE OWNER_PERMISSIONS CONTAINS R AND OWNER_PERMISSIONS CONTAINS X   
        -- Returns the files that the owner has the read and execute flag set.
        ```

        **Note:** If you write something like:  
        ```sql
        SELECT NAME FROM . WHERE NAME ENDS_WITH .cpp AND SIZE > 1000  
        ```
        This means:  
        > Return the name of the files whose ends with .cpp and has more than 1000 bytes

        Now, if you write something like:  
        ```sql
        SELECT NAME FROM . WHERE NAME ENDS_WITH .cpp AND SIZE > 1000 OR NAME ENDS_WITH .txt
        ```
        This means:   
        > Return the name of the files whose ends with .cpp and has the size greater than 1000 bytes or ends with .txt

        It is possible to negate operators too!    
        Actually, we are not negating operators, **Termsequel 1.1** introduced new operators(NOT_CONTAINS, NOT_STARTS_WITH and NOT_ENDS_WITH).   
        For examples, you want to select all the files that do not end with `.txt`.   
        ```sql
        SELECT NAME FROM . WHERE NAME NOT_ENDS_WITH .txt
        ```

    - ### Variables

        Now, it is possible to use your environment variables with **Termsequel**!   

        For example, in bash, you can write:    
        ```shell
        # This is the same to write: /home/my_user 
        $ termsequel 'SELECT NAME FROM $HOME WHERE NAME ENDS_WITH .c OR NAME ENDS_WITH .cpp'
        ```

        If you terminal support, you even use non environment variables:   
        **Example:** Exporting a user defined shell variable(will live only in this shel session)   
        ```shell
        $ export BOOKS=/home/sgtcortez/Documentos/Books/
        $ termsequel 'SELECT NAME, SIZE FROM $BOOKS'
        ```

        **Note:** You can only use this feature at the first part!   
        For example, `$HOME/$OTHER` will be expanded to: `/home/my_user/$OTHER`.  
        There are no plans to support more than one ...     
        
        **Note:** The **$** character must be the first source character!
        For example, `Documents$HOME` will not be expanded! 


# Developers
There are sections for developers to be able to join this project.

<details>
<summary>
Git Guidelines
</summary>
Feature branch <strong>must</strong> be created from the most stable branch(usually main), and, when done, should make a Pull Request to the <strong>development</strong> branch.  
The commits <strong>must</strong> be <strong><italic>"squashed"</italic></strong>. The person who accepts the pull request, must, do a rebase to pick the single commit and put it on the target branch. 
</details>